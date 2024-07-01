// Copyright (C) 2022-2024  ilobilo

#include <yapl/yapl.hpp>
#include <fmt/core.h>

namespace yapl
{
    unit::unit(std::string_view target, std::string_view filename) :
        target { target }, filename { filename },
        tokeniser { std::string(filename) }, parser { tokeniser, *this },
        context { }, builder { context }, llmod { filename, context }
    {
        this->llmod.setTargetTriple(this->target);

        {
            auto add_type = [&](auto name, auto tp)
            {
                if (this->type_registry.normal.contains(name) == false)
                    this->type_registry.normal[name] = std::move(tp);
            };

            add_type("i8", std::make_unique<ast::types::number>(ast::detail::num_size::i8, true));
            add_type("u8", std::make_unique<ast::types::number>(ast::detail::num_size::i8, false));

            add_type("i16", std::make_unique<ast::types::number>(ast::detail::num_size::i16, true));
            add_type("u16", std::make_unique<ast::types::number>(ast::detail::num_size::i16, false));

            add_type("i32", std::make_unique<ast::types::number>(ast::detail::num_size::i32, true));
            add_type("u32", std::make_unique<ast::types::number>(ast::detail::num_size::i32, false));

            add_type("i64", std::make_unique<ast::types::number>(ast::detail::num_size::i64, true));
            add_type("u64", std::make_unique<ast::types::number>(ast::detail::num_size::i64, false));

            add_type("f32", std::make_unique<ast::types::number>(ast::detail::num_size::f32, true));
            add_type("f64", std::make_unique<ast::types::number>(ast::detail::num_size::f64, true));


            add_type("bool", std::make_unique<ast::types::boolean>());
            add_type("void", std::make_unique<ast::types::void_type>());

            add_type("string", std::make_unique<ast::types::string>());
        }
    }

    bool unit::parse()
    {
        try {
            this->parser.parse();
        }
        catch (const std::exception &e)
        {
            fmt::println(stderr, "{}", e.what());
            return false;
        }
        return true;
    }
} // namespace yapl