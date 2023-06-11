// Copyright (C) 2022  ilobilo

#pragma once

#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>

#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/Host.h>

#include <llvm/MC/TargetRegistry.h>

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Module.h>

#include <lexer/lexer.hpp>
#include <log.hpp>

namespace yapl::parser
{
    enum class node_type
    {
        number,
        binary
    };

    struct node
    {
        virtual ~node() { }
        virtual llvm::Value *codegen(llvm::IRBuilder<> &builder, std::shared_ptr<llvm::Module> module) = 0;
    };

    struct number_node : node
    {
        number_node(ssize_t value) : value(value) { }

        llvm::Value *codegen(llvm::IRBuilder<> &builder, std::shared_ptr<llvm::Module> module) override
        {
            return llvm::ConstantInt::get(llvm::Type::getInt64Ty(module->getContext()), value);
        }

        private:
        ssize_t value;
    };

    struct binaryop_node : node
    {
        private:
        char op;
        std::shared_ptr<node> left;
        std::shared_ptr<node> right;

        public:
        binaryop_node(char op, std::shared_ptr<node> left, std::shared_ptr<node> right) : op(op), left(left), right(right) { }

        llvm::Value *codegen(llvm::IRBuilder<> &builder, std::shared_ptr<llvm::Module> module) override
        {
            llvm::Value *leftValue = left->codegen(builder, module);
            llvm::Value *rightValue = right->codegen(builder, module);

            if (!leftValue || !rightValue)
                return nullptr;

            switch (op)
            {
                case '+':
                    return builder.CreateAdd(leftValue, rightValue, "addtmp");
                case '-':
                    return builder.CreateSub(leftValue, rightValue, "subtmp");
                case '*':
                    return builder.CreateMul(leftValue, rightValue, "multmp");
                case '/':
                    return builder.CreateSDiv(leftValue, rightValue, "divtmp");
                default:
                    return nullptr;
            }
        }
    };

    template<typename Stream>
    struct pratt
    {
        pratt(lexer::tokeniser<Stream> &tokeniser) : tokeniser(tokeniser), currentToken() { }
        std::shared_ptr<node> parse()
        {
            currentToken = tokeniser();
            return expr();
        }

        private:
        lexer::tokeniser<Stream> &tokeniser;
        lexer::token currentToken;

        std::shared_ptr<node> factor()
        {
            lexer::token token = currentToken;
            if (token.type == lexer::token_type::number)
            {
                currentToken = tokeniser();
                return std::make_shared<number_node>(std::stoi(token.name));
            }
            else if (token.type == lexer::token_type::open_round)
            {
                currentToken = tokeniser();
                std::shared_ptr<node> node = expr();
                if (currentToken.type != lexer::token_type::close_round)
                    throw std::runtime_error("Expected )");

                currentToken = tokeniser();
                return node;
            }
            else throw log::error(tokeniser.file(), token.line, token.column, "Expected integer or (");
        }

        std::shared_ptr<node> term()
        {
            std::shared_ptr<node> node = factor();
            while (currentToken.type == lexer::token_type::mul || currentToken.type == lexer::token_type::div)
            {
                lexer::token token = currentToken;
                currentToken = tokeniser();
                node = std::make_shared<binaryop_node>(token.name[0], node, factor());
            }
            return node;
        }

        std::shared_ptr<node> expr()
        {
            std::shared_ptr<node> node = term();
            while (currentToken.type == lexer::token_type::add || currentToken.type == lexer::token_type::sub)
            {
                lexer::token token = currentToken;
                currentToken = tokeniser();
                node = std::make_shared<binaryop_node>(token.name[0], node, term());
            }
            return node;
        }
    };

    template<typename Stream>
    pratt(lexer::tokeniser<Stream>) -> pratt<Stream>;
} // namespace yapl::parser