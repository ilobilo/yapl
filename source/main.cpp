// Copyright (C) 2022-2024  ilobilo

#include <conflict/conflict.hpp>
#include <magic_enum.hpp>

#include <yapl/yapl.hpp>
#include <yapl/log.hpp>

#include <filesystem>
#include <optional>

#include <llvm/Support/TargetSelect.h>
#include <llvm/TargetParser/Host.h>
#include <llvm/MC/TargetRegistry.h>

namespace arguments
{
    constexpr std::string_view version_string { YAPL_VERSION };

    std::string_view target;
    std::string_view input;
    std::string_view output;

    uint64_t flags = 0;

    enum args : std::uint64_t
    {
        help = (1 << 0),
        version = (1 << 1)
    };

    const conflict::parser parser
    {
        conflict::option {
            { 'h', "help", "Show help" },
            flags, args::help
        },
        conflict::option {
            { 'v', "version", "Show version" },
            flags, args::version
        },
        conflict::string_option {
            { 'i', "input", "Input file. Required" },
            "file.yapl", input
        },
        conflict::string_option {
            { 'o', "output", "Output file. Required" },
            "file.o", output
        },
        conflict::string_option {
            { 't', "target", "Target triple: <arch><sub>-<vendor>-<sys>-<abi>. Optional" },
            "triple", target
        }
    };

    void print_version()
    {
        fmt::println("YAPL {}", version_string);
    }

    void print_help()
    {
        fmt::println("Parameters:");
        parser.print_help();
        print_version();
    }

    std::optional<int> parse(int argc, char **argv)
    {
        parser.apply_defaults();
        conflict::default_report(parser.parse(argc - 1, argv + 1));

        bool help_requested = (flags & args::help);
        bool stupid_user = (input.empty() || output.empty()) && !help_requested;

        if (flags & args::version)
        {
            print_version();
            return EXIT_SUCCESS;
        }
        else if (help_requested || stupid_user)
        {
            print_help();
            return (flags & args::help) ? EXIT_SUCCESS : EXIT_FAILURE;
        }

        namespace fs = std::filesystem;
        namespace log = yapl::log;
        using level = log::level;

        if (!fs::exists(arguments::input))
        {
            log::println<level::error>("File '{}' does not exist", arguments::input);
            return EXIT_FAILURE;
        }

        if (fs::exists(arguments::output))
        {
            log::println<level::error>("File '{}' already exists", arguments::output);
            return EXIT_FAILURE;
        }

        return std::nullopt;
    }
} // namespace arguments

void llvm_init()
{
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
}

auto main(int argc, char **argv) -> int
{
    namespace log = yapl::log;
    using level = log::level;

    if (auto val = arguments::parse(argc, argv); val.has_value())
        return val.value();

    llvm_init();

    auto target = arguments::target.empty() ? llvm::sys::getDefaultTargetTriple() : std::string(arguments::target);
    if (std::string err; llvm::TargetRegistry::lookupTarget(target, err) == nullptr)
    {
        log::println<level::error>("{}", err);
        return EXIT_FAILURE;
    }

    yapl::module mod { target, arguments::input };

    mod.parse();

    // while (true)
    // {
    //     try
    //     {
    //         auto [str, type, line, column] = mod.tokeniser();
    //         if (type == yapl::lexer::token_type::eof)
    //             break;

    //         fmt::println("{:02}:{:02}: '{}' : {}", line, column, str, magic_enum::enum_name(type));
    //     }
    //     catch (const std::exception &e)
    //     {
    //         fmt::println(stderr, "{}", e.what());
    //         return EXIT_FAILURE;
    //     }
    // }

    return EXIT_SUCCESS;
}