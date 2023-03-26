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
#include <stdexcept>

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
    auto module = std::make_unique<llvm::Module>("Module", context);

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

    // llvm::Function::Create(llvm::FunctionType::get(builder.getInt32Ty(), { builder.getInt8PtrTy() }, true), llvm::Function::ExternalLinkage, "printf", module.get());
    // auto int8ptr = builder.getInt8PtrTy();
    // auto func = llvm::Function::Create(llvm::FunctionType::get(builder.getInt32Ty(), { builder.getInt32Ty(), llvm::PointerType::get(int8ptr, int8ptr->getAddressSpace()) }, false), llvm::Function::ExternalLinkage, "main", module.get());

    // builder.SetInsertPoint(llvm::BasicBlock::Create(context, "entry", func));
    // {
    //     auto args_count = func->getArg(0);
    //     auto args = func->getArg(1);

    //     auto i = builder.CreateAlloca(builder.getInt32Ty());
    //     builder.CreateStore(builder.getInt32(1), i);

    //     auto rac = builder.CreateAlloca(builder.getInt32Ty());
    //     builder.CreateStore(builder.CreateSub(args_count, builder.getInt32(1)), rac);

    //     builder.CreateCall(module->getFunction("printf"), { builder.CreateGlobalStringPtr("arguments: %d\n"), builder.CreateLoad(rac->getType(), rac) });

    //     auto loop_start = llvm::BasicBlock::Create(context, "loop_start", func);
    //     auto loop = llvm::BasicBlock::Create(context, "loop", func);
    //     auto loop_exit = llvm::BasicBlock::Create(context, "loop_exit", func);
    //     auto endln = llvm::BasicBlock::Create(context, "endln", func);
    //     auto endln_exit = llvm::BasicBlock::Create(context, "endln_exit", func);

    //     builder.CreateBr(loop_start);

    //     builder.SetInsertPoint(loop_start);
    //     builder.CreateCondBr(builder.CreateICmp(llvm::CmpInst::ICMP_SLT, builder.CreateLoad(builder.getInt32Ty(), i), args_count), loop, loop_exit);

    //     builder.SetInsertPoint(loop);
    //     {
    //         auto elem = builder.CreateInBoundsGEP(builder.getInt8PtrTy(), args, builder.CreateLoad(builder.getInt32Ty(), i));
    //         builder.CreateCall(module->getFunction("printf"), { builder.CreateGlobalStringPtr("%s "), builder.CreateLoad(elem->getType(), elem) });
    //         builder.CreateStore(builder.CreateAdd(builder.CreateLoad(builder.getInt32Ty(), i), builder.getInt32(1)), i);
    //         builder.CreateBr(loop_start);
    //     }

    //     builder.SetInsertPoint(loop_exit);
    //     builder.CreateCondBr(builder.CreateICmp(llvm::CmpInst::ICMP_EQ, builder.CreateLoad(rac->getType(), rac), builder.getInt32(0)), endln_exit, endln);

    //     builder.SetInsertPoint(endln);
    //     builder.CreateCall(module->getFunction("printf"), { builder.CreateGlobalStringPtr("\n") });
    //     builder.CreateBr(endln_exit);

    //     builder.SetInsertPoint(endln_exit);
    // }

    // builder.CreateRet(builder.getInt32(0));
    // module->print(llvm::outs(), nullptr);

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
                auto [str, type, tok, line, column] = tokeniser();
                if (type == yapl::lexer::token_type::eof)
                    break;

                if (tok.has_value())
                    fmt::print(" - {:02}:{:02}: {}: {}: {} \n", line, column, str, magic_enum::enum_name(type), magic_enum::enum_name(tok.value()));
                else
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