// Copyright (C) 2022-2024  ilobilo

#pragma once

// #include <llvm/Target/TargetMachine.h>
// #include <llvm/Target/TargetOptions.h>

// #include <llvm/Support/TargetSelect.h>
// #include <llvm/TargetParser/Host.h>

// #include <llvm/MC/TargetRegistry.h>

// #include <llvm/IR/LLVMContext.h>
// #include <llvm/IR/IRBuilder.h>
// #include <llvm/IR/Function.h>
// #include <llvm/IR/Module.h>

#include <yapl/lexer.hpp>

// #include <string_view>
// #include <string>
// #include <vector>
// #include <utility>

namespace yapl::parser
{
    struct parse
    {
        lexer::tokeniser &tokeniser;

        parse(lexer::tokeniser &tokeniser) :
            tokeniser(tokeniser) { }
    };

    // enum class node_type
    // {
    //     number,
    //     binary
    // };

    // struct node
    // {
    //     virtual ~node() { }
    //     virtual llvm::Value *codegen(llvm::IRBuilder<> &builder, std::shared_ptr<llvm::Module> module) = 0;
    // };

    // struct prototype : node
    // {
    //     private:
    //     std::string name;
    //     std::vector<std::string> args;

    //     public:
    //     prototype(std::string_view name, std::vector<std::string> args) : name(name), args(std::move(args)) { }
    // };

    // struct function
    // {
    //     private:
    //     std::unique_ptr<prototype> proto;
    //     std::unique_ptr<node> body;

    //     public:
    //     function(std::unique_ptr<prototype> proto, std::unique_ptr<node> body)
    //         : proto(std::move(proto)), body(std::move(body)) { }
    // };

    // struct variable : node
    // {
    //     private:
    //     std::string name;

    //     public:
    //     variable(std::string_view name) : name(name) { }
    // };

    // struct number_node : node
    // {
    //     private:
    //     double value;

    //     number_node(double value) : value(value) { }

    //     llvm::Value *codegen(llvm::IRBuilder<> &builder, std::shared_ptr<llvm::Module> module) override
    //     {
    //         return llvm::ConstantInt::get(llvm::Type::getInt64Ty(module->getContext()), value);
    //     }
    // };

    // struct binaryop_node : node
    // {
    //     private:
    //     lexer::token_type op;
    //     std::shared_ptr<node> left;
    //     std::shared_ptr<node> right;

    //     public:
    //     binaryop_node(lexer::token_type op, std::shared_ptr<node> left, std::shared_ptr<node> right) : op(op), left(left), right(right) { }

    //     llvm::Value *codegen(llvm::IRBuilder<> &builder, std::shared_ptr<llvm::Module> module) override
    //     {
    //         llvm::Value *leftValue = left->codegen(builder, module);
    //         llvm::Value *rightValue = right->codegen(builder, module);

    //         if (!leftValue || !rightValue)
    //             return nullptr;

    //         switch (op)
    //         {
    //             case lexer::token_type::add:
    //                 return builder.CreateAdd(leftValue, rightValue);
    //             case lexer::token_type::add_assign:
    //                 return builder.CreateStore(builder.CreateAdd(leftValue, rightValue), leftValue);

    //             case lexer::token_type::sub:
    //                 return builder.CreateSub(leftValue, rightValue);
    //             case lexer::token_type::sub_assign:
    //                 return builder.CreateStore(builder.CreateSub(leftValue, rightValue), leftValue);

    //             case lexer::token_type::mul:
    //                 return builder.CreateMul(leftValue, rightValue);
    //             case lexer::token_type::mul_assign:
    //                 return builder.CreateStore(builder.CreateMul(leftValue, rightValue), leftValue);

    //             case lexer::token_type::div:
    //                 return builder.CreateSDiv(leftValue, rightValue);
    //             case lexer::token_type::div_assign:
    //                 return builder.CreateStore(builder.CreateSDiv(leftValue, rightValue), leftValue);

    //             case lexer::token_type::mod:
    //                 return builder.CreateSRem(leftValue, rightValue);
    //             case lexer::token_type::mod_assign:
    //                 return builder.CreateStore(builder.CreateSRem(leftValue, rightValue), leftValue);


    //             case lexer::token_type::bw_and:
    //                 return builder.CreateAnd(leftValue, rightValue);
    //             case lexer::token_type::bw_and_assign:
    //                 return builder.CreateStore(builder.CreateAnd(leftValue, rightValue), leftValue);

    //             case lexer::token_type::bw_or:
    //                 return builder.CreateOr(leftValue, rightValue);
    //             case lexer::token_type::bw_or_assign:
    //                 return builder.CreateStore(builder.CreateOr(leftValue, rightValue), leftValue);

    //             case lexer::token_type::bw_xor:
    //                 return builder.CreateXor(leftValue, rightValue);
    //             case lexer::token_type::bw_xor_assign:
    //                 return builder.CreateStore(builder.CreateXor(leftValue, rightValue), leftValue);


    //             case lexer::token_type::shiftl:
    //                 return builder.CreateShl(leftValue, rightValue);
    //             case lexer::token_type::shiftl_assign:
    //                 return builder.CreateStore(builder.CreateShl(leftValue, rightValue), leftValue);

    //             case lexer::token_type::shiftr:
    //                 return builder.CreateAShr(leftValue, rightValue);
    //             case lexer::token_type::shiftr_assign:
    //                 return builder.CreateStore(builder.CreateAShr(leftValue, rightValue), leftValue);


    //             case lexer::token_type::eq:
    //                 return builder.CreateICmpEQ(leftValue, rightValue);
    //             case lexer::token_type::ne:
    //                 return builder.CreateICmpNE(leftValue, rightValue);
    //             case lexer::token_type::lt:
    //                 return builder.CreateICmpSLT(leftValue, rightValue);
    //             case lexer::token_type::gt:
    //                 return builder.CreateICmpSGT(leftValue, rightValue);
    //             case lexer::token_type::le:
    //                 return builder.CreateICmpSLE(leftValue, rightValue);
    //             case lexer::token_type::ge:
    //                 return builder.CreateICmpSGE(leftValue, rightValue);
    //             default:
    //                 return nullptr;
    //         }
    //     }
    // };
} // namespace yapl::parser