// Copyright (C) 2022-2024  ilobilo

#pragma once

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>

#include <yapl/lexer.hpp>
#include <yapl/parser.hpp>

#include <unordered_map>
#include <map>
#include <vector>
#include <memory>

namespace yapl
{
    namespace registries
    {
        using funcs = std::vector<std::unique_ptr<ast::func::function>>;

        struct types
        {
            std::unordered_map<std::string_view, std::unique_ptr<ast::types::type>> normal;
            std::unordered_map<std::string_view, std::unique_ptr<ast::types::pointer>> pointers;
            std::map<std::pair<std::string_view, std::size_t>, std::unique_ptr<ast::types::array>> arrays;
        };
    } // namespace registries

    struct module
    {
        std::string target;
        std::string filename;

        lexer::tokeniser tokeniser;
        ast::parser parser;

        registries::types type_registry;
        registries::funcs func_registry;

        llvm::LLVMContext context;
        llvm::IRBuilder<> builder;
        llvm::Module llmod;

        module(std::string_view target, std::string_view filename);

        bool parse();
    };
} // namespace yapl