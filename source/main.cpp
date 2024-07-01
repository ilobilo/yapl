// Copyright (C) 2022-2024  ilobilo

#include <argparse/argparse.hpp>
#include <magic_enum.hpp>

#include <fmt/ostream.h>

#include <yapl/yapl.hpp>
#include <yapl/log.hpp>

#include <filesystem>
#include <optional>

#include <llvm/Support/TargetSelect.h>
#include <llvm/TargetParser/Host.h>
#include <llvm/MC/TargetRegistry.h>

namespace arguments
{
    static constexpr std::string_view auto_detect_str = "<auto-detect>";

    static std::string target;
    static std::string input;
    static std::string output;

    std::optional<int> parse(int argc, char **argv)
    {
        argparse::ArgumentParser parser("YAPL", YAPL_VERSION, argparse::default_arguments::all, true);

        parser.add_argument("-t", "--target")
            .default_value(auto_detect_str)
            .help("target triple: <arch><sub>-<vendor>-<sys>-<abi>");

        parser.add_argument("-i", "--input")
            .required()
            .help("specify the input file");

        parser.add_argument("-o", "--output")
            .default_value("a.out")
            .help("specify the output file");

        try {
            parser.parse_args(argc, argv);
        }
        catch (const std::exception &e)
        {
            fmt::println(stderr, "{}", e.what());
            fmt::println(stderr, "{}", fmt::streamed(parser));
            return EXIT_FAILURE;
        }

        if (auto fn = parser.present("-i"))
            arguments::input = *fn;

        arguments::output = parser.get<std::string>("-o");
        arguments::target = parser.get<std::string_view>("-t");

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

    auto target = (arguments::target == arguments::auto_detect_str)
        ? llvm::sys::getDefaultTargetTriple()
        : std::string(arguments::target);

    if (std::string err; llvm::TargetRegistry::lookupTarget(target, err) == nullptr)
    {
        log::println<level::error>("{}", err);
        return EXIT_FAILURE;
    }

    yapl::unit mod { target, arguments::input };

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