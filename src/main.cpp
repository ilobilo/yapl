// Copyright (C) 2022  ilobilo

#include <conflict/conflict.hpp>
#include <magic_enum.hpp>

#include <fmt/color.h>
#include <fmt/core.h>

#include <parser/parser.hpp>
#include <lexer/lexer.hpp>
#include <log.hpp>

#include <filesystem>
#include <fstream>

#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>

#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/Host.h>

#include <llvm/MC/TargetRegistry.h>

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Module.h>

std::vector<std::string_view> files;
std::string_view target_v;
uint64_t flags = 0;

const auto parser = conflict::parser
{
    conflict::option { { 'h', "help", "Show help" }, flags, (1 << 0) },
    conflict::option { { 'v', "version", "Show version" }, flags, (1 << 1) },
    conflict::string_option { { 't', "target", "Target triple: <arch><sub>-<vendor>-<sys>-<abi>" }, "triple", target_v }
};

bool parse_flags()
{
    if (flags & (1 << 0))
    {
        fmt::print("Options:\n");
        parser.print_help();
        return true;
    }
    else if (flags & (1 << 1))
    {
        fmt::print("YAPL v0.1\n");
        return true;
    }

    return false;
}

auto main(int argc, char **argv) -> int
{
    parser.apply_defaults();
    conflict::default_report(parser.parse(argc - 1, argv + 1, files));

    if (parse_flags())
        return EXIT_SUCCESS;

    llvm::LLVMContext context;
    context.setOpaquePointers(true);

    llvm::IRBuilder builder(context);
    auto module = std::make_shared<llvm::Module>("Module", context);

    std::string target;
    if (target_v.empty())
        target = llvm::sys::getDefaultTargetTriple();
    else
        target = target_v;

    module->setTargetTriple(target);

    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();

    if (std::string err; llvm::TargetRegistry::lookupTarget(target, err) == nullptr)
    {
        std::cerr << err << std::endl;
        return EXIT_FAILURE;
    }

    bool first = true;
    for (const auto &filename : files)
    {
        if (first == true)
            first = false;
        else
            fmt::print("\n");

        if (std::filesystem::exists(filename))
            fmt::print("File: \"{}\"\n", filename);
        else
        {
            fmt::print(fmt::fg(fmt::color::red), "File: \"{}\" does not exist!\n", filename);
            continue;
        }

        std::ifstream file(filename.data());
        yapl::lexer::tokeniser tokeniser(filename, file);

        while (true)
        {
            try
            {
                auto [str, type, line, column] = tokeniser();
                if (type == yapl::lexer::token_type::eof)
                    break;

                fmt::print(" - {:02}:{:02}: {}: {} \n", line, column, str, magic_enum::enum_name(type));
            }
            catch (const std::exception &e)
            {
                std::cerr << e.what() << '\n';
                break;
            }
        }
    }

    return EXIT_SUCCESS;
}