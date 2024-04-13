// Copyright (C) 2022-2024  ilobilo

#pragma once

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>

#include <yapl/lexer.hpp>
#include <yapl/parser.hpp>

namespace yapl
{
    namespace registries
    {
        using variables = std::unordered_map<std::string_view, std::unique_ptr<ast::nodes::variable>>;
        using types = std::unordered_map<std::string_view, std::unique_ptr<ast::types::type>>;
    } // namespace registries

    struct module
    {
        std::string target;
        std::string filename;

        lexer::tokeniser tokeniser;
        ast::parser parser;

        registries::types type_registry;
        std::vector<std::unique_ptr<ast::func::function>> func_registry;

        llvm::LLVMContext context;
        llvm::IRBuilder<> builder;
        llvm::Module llmod;

        module(std::string_view target, std::string_view filename);

        bool parse();
    };
} // namespace yapl