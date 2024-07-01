// Copyright (C) 2022-2024  ilobilo

#pragma once

#include <llvm/IR/IRBuilder.h>

#include <yapl/lexer.hpp>

#include <string_view>
#include <string>

#include <variant>
#include <vector>
#include <memory>

#include <cstdint>

namespace yapl
{
    struct unit;
} // namespace yapl

namespace yapl::ast
{
    namespace detail
    {
        template<typename ...Types>
        struct overloads : Types... { using Types::operator()...; };

        enum class num_size : std::uint8_t
        {
            i8, i16, i32, i64, f32, f64
        };
    } // namespace detail

    namespace types
    {
        struct type
        {
            virtual ~type() = default;
            virtual llvm::Type *codegen(llvm::IRBuilder<> &builder) const = 0;
        };

        struct string : type
        {
            llvm::Type *codegen(llvm::IRBuilder<> &builder) const override
            {
                // TODO
                return nullptr;
            }
        };

        struct number : type
        {
            detail::num_size size;
            bool is_signed;

            number(detail::num_size size, bool is_signed) :
                size { size }, is_signed { is_signed } { }

            llvm::Type *codegen(llvm::IRBuilder<> &builder) const override
            {
                switch (this->size)
                {
                    case detail::num_size::i8:
                        return builder.getInt8Ty();
                    case detail::num_size::i16:
                        return builder.getInt16Ty();
                    case detail::num_size::i32:
                        return builder.getInt32Ty();
                    case detail::num_size::i64:
                        return builder.getInt64Ty();
                    case detail::num_size::f32:
                        return builder.getFloatTy();
                    case detail::num_size::f64:
                        return builder.getDoubleTy();
                    default:
                        return nullptr;
                }
            }
        };

        struct boolean : type
        {
            llvm::Type *codegen(llvm::IRBuilder<> &builder) const override
            {
                return builder.getInt1Ty();
            }
        };

        struct void_type : type
        {
            llvm::Type *codegen(llvm::IRBuilder<> &builder) const override
            {
                return builder.getVoidTy();
            }
        };

        struct pointer : type
        {
            const type *tp;

            explicit pointer(const type *tp) : tp { tp } { }

            llvm::Type *codegen(llvm::IRBuilder<> &builder) const override
            {
                return llvm::PointerType::get(this->tp->codegen(builder), 0);
            }
        };

        struct array : type
        {
            const type *tp;
            std::size_t size;

            array(const type *tp, std::size_t size) :
                tp { tp }, size { size } { }

            llvm::Type *codegen(llvm::IRBuilder<> &builder) const override
            {
                return llvm::ArrayType::get(this->tp->codegen(builder), this->size);
            }
        };
    } // namespace types

    namespace expressions
    {
        struct expression
        {
            virtual ~expression() = default;
            virtual llvm::Value *codegen(llvm::IRBuilder<> &builder) = 0;
        };

        struct boolean : expression
        {
            private:
            bool value;

            public:
            explicit boolean(bool value) : value(value) { }

            llvm::Value *codegen(llvm::IRBuilder<> &builder) override
            {
                return builder.getInt1(this->value);
            }
        };

        struct number : expression
        {
            private:
            std::variant<std::uint64_t, double> value;

            public:
            explicit number(std::uint64_t value) : value { value } { }
            explicit number(double value) : value { value } { }

            llvm::Value *codegen(llvm::IRBuilder<> &builder) override
            {
                llvm::Value *ret = nullptr;
                std::visit(detail::overloads {
                    [&](std::uint64_t val) {
                        ret = builder.getInt64(val);
                    },
                    [&](double val) {
                        ret = llvm::ConstantFP::get(builder.getDoubleTy(), val);
                    }
                }, this->value);
                return ret;
            }
        };

        struct string : expression
        {
            private:
            std::string value;

            public:
            explicit string(std::string_view value) : value { value } { }

            llvm::Value *codegen(llvm::IRBuilder<> &builder) override
            {
                return llvm::ConstantDataArray::getString(builder.getContext(), this->value);
            }
        };

        struct binaryop : expression
        {
            private:
            lexer::token_type op;
            std::shared_ptr<expression> left;
            std::shared_ptr<expression> right;

            public:
            binaryop(lexer::token_type op, std::shared_ptr<expression> left, std::shared_ptr<expression> right) :
                op { op }, left { left }, right { right } { }

            llvm::Value *codegen(llvm::IRBuilder<> &builder) override
            {
                auto leftValue = left->codegen(builder);
                auto rightValue = right->codegen(builder);

                if (!leftValue || !rightValue)
                    return nullptr;

                switch (op)
                {
                    case lexer::token_type::inc:
                        return builder.CreateAdd(leftValue, llvm::ConstantInt::get(leftValue->getType(), 1));
                    case lexer::token_type::dec:
                        return builder.CreateSub(leftValue, llvm::ConstantInt::get(leftValue->getType(), 1));

                    case lexer::token_type::add:
                        return builder.CreateAdd(leftValue, rightValue);
                    case lexer::token_type::add_assign:
                        return builder.CreateStore(builder.CreateAdd(leftValue, rightValue), leftValue);

                    case lexer::token_type::sub:
                        return builder.CreateSub(leftValue, rightValue);
                    case lexer::token_type::sub_assign:
                        return builder.CreateStore(builder.CreateSub(leftValue, rightValue), leftValue);

                    case lexer::token_type::mul:
                        return builder.CreateMul(leftValue, rightValue);
                    case lexer::token_type::mul_assign:
                        return builder.CreateStore(builder.CreateMul(leftValue, rightValue), leftValue);

                    case lexer::token_type::div:
                        return builder.CreateSDiv(leftValue, rightValue);
                    case lexer::token_type::div_assign:
                        return builder.CreateStore(builder.CreateSDiv(leftValue, rightValue), leftValue);

                    case lexer::token_type::mod:
                        return builder.CreateSRem(leftValue, rightValue);
                    case lexer::token_type::mod_assign:
                        return builder.CreateStore(builder.CreateSRem(leftValue, rightValue), leftValue);

                    case lexer::token_type::bw_and:
                        return builder.CreateAnd(leftValue, rightValue);
                    case lexer::token_type::bw_and_assign:
                        return builder.CreateStore(builder.CreateAnd(leftValue, rightValue), leftValue);

                    case lexer::token_type::bw_or:
                        return builder.CreateOr(leftValue, rightValue);
                    case lexer::token_type::bw_or_assign:
                        return builder.CreateStore(builder.CreateOr(leftValue, rightValue), leftValue);

                    case lexer::token_type::bw_xor:
                        return builder.CreateXor(leftValue, rightValue);
                    case lexer::token_type::bw_xor_assign:
                        return builder.CreateStore(builder.CreateXor(leftValue, rightValue), leftValue);

                    case lexer::token_type::shiftl:
                        return builder.CreateShl(leftValue, rightValue);
                    case lexer::token_type::shiftl_assign:
                        return builder.CreateStore(builder.CreateShl(leftValue, rightValue), leftValue);

                    case lexer::token_type::shiftr:
                        return builder.CreateAShr(leftValue, rightValue);
                    case lexer::token_type::shiftr_assign:
                        return builder.CreateStore(builder.CreateAShr(leftValue, rightValue), leftValue);

                    case lexer::token_type::assign:
                        return builder.CreateStore(rightValue, leftValue);

                    case lexer::token_type::log_not:
                        return builder.CreateNot(leftValue);

                    case lexer::token_type::log_and:
                        return builder.CreateAnd(leftValue, rightValue);
                    case lexer::token_type::log_and_assign:
                        return builder.CreateStore(builder.CreateAnd(leftValue, rightValue), leftValue);

                    case lexer::token_type::log_or:
                        return builder.CreateOr(leftValue, rightValue);
                    case lexer::token_type::log_or_assign:
                        return builder.CreateStore(builder.CreateOr(leftValue, rightValue), leftValue);

                    case lexer::token_type::log_xor:
                        return builder.CreateXor(leftValue, rightValue);
                    case lexer::token_type::log_xor_assign:
                        return builder.CreateStore(builder.CreateXor(leftValue, rightValue), leftValue);

                    case lexer::token_type::eq:
                        return builder.CreateICmpEQ(leftValue, rightValue);
                    case lexer::token_type::ne:
                        return builder.CreateICmpNE(leftValue, rightValue);
                    case lexer::token_type::lt:
                        return builder.CreateICmpSLT(leftValue, rightValue);
                    case lexer::token_type::gt:
                        return builder.CreateICmpSGT(leftValue, rightValue);
                    case lexer::token_type::le:
                        return builder.CreateICmpSLE(leftValue, rightValue);
                    case lexer::token_type::ge:
                        return builder.CreateICmpSGE(leftValue, rightValue);

                    default:
                        return nullptr;
                }
            }
        };
    } // namespace expressions

    namespace statements
    {
        struct statement
        {
            virtual ~statement() = default;
            // virtual llvm::Value *codegen(llvm::IRBuilder<> &builder) = 0;
        };

        struct variable : statement
        {
            const types::type *type;
            std::string name;

            variable(const types::type *type, std::string_view name) :
                type { type }, name { name } { }
        };

        struct return_statement : statement
        {
            std::unique_ptr<expressions::expression> expr;

            return_statement(std::unique_ptr<expressions::expression> expr) :
                expr { std::move(expr) } { }
        };
    } // namespace statements

    namespace func
    {
        struct function
        {
            std::string name;
            std::vector<std::unique_ptr<statements::variable>> params;
            const types::type *ret_type;

            std::vector<statements::statement *> body;

            llvm::FunctionType *typegen(llvm::IRBuilder<> &builder)
            {
                std::vector<llvm::Type *> types;
                for (auto &param : this->params)
                    types.push_back(param->type->codegen(builder));

                auto ret = this->ret_type->codegen(builder);
                return llvm::FunctionType::get(ret, types, false);
            }
        };
    } // namespace func

    struct parser
    {
        private:
        const types::type *get_type(std::string_view name, std::size_t array_size = 0) const;

        std::tuple<std::string, std::size_t> parse_type(lexer::tokeniser &parent_toker, lexer::token tok, bool should_throw = true);
        std::tuple<std::string, std::string, std::size_t> parse_variable(lexer::tokeniser &parent_toker, lexer::token tok, bool should_throw = true);

        std::unique_ptr<expressions::expression> parse_expression(lexer::tokeniser &parent_toker, lexer::token tok, bool should_throw = true);
        std::unique_ptr<func::function> parse_function(lexer::tokeniser &parent_toker, lexer::token tok, bool should_throw = true);

        public:
        lexer::tokeniser &tokeniser;
        unit &parent;

        parser(lexer::tokeniser &tokeniser, unit &parent) :
            tokeniser { tokeniser }, parent { parent } { }

        void parse();
    };
} // namespace yapl::ast