// Copyright (C) 2022-2024  ilobilo

#pragma once

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>

#include <yapl/lexer.hpp>
#include <yapl/parser.hpp>

namespace yapl
{
    struct module
    {
        std::string target;
        std::string filename;

        lexer::tokeniser tokeniser;
        parser::parse parser;

        llvm::LLVMContext context;
        llvm::IRBuilder<> builder;
        llvm::Module llmod;

        module(std::string_view target, std::string_view filename, lexer::tokeniser::stream_type &stream) :
            target { target }, filename { filename },
            tokeniser { filename, stream }, parser { tokeniser },
            context { }, builder { context }, llmod { filename, context }
        {
            this->llmod.setTargetTriple(this->target);
        }
    };
} // namespace yapl