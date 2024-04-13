// Copyright (C) 2022-2024  ilobilo

#include <yapl/parser.hpp>
#include <yapl/lexer.hpp>
#include <yapl/yapl.hpp>
#include <yapl/log.hpp>

#include <utility>

namespace yapl::ast
{
#define YAPL_EXPECT(x, exp)                                                                                  \
    do {                                                                                                     \
        if (!(x)) {                                                                                          \
            throw log::parse::error(this->parent.filename, line, column, "Expected {}, got '{}'", exp, str); \
        }                                                                                                    \
    } while (0);

    const types::type *parser::get_type(std::string_view name) const
    {
        if (!this->parent.type_registry.contains(name))
            return nullptr;

        return this->parent.type_registry.at(name).get();
    }

    std::pair<std::string, std::string> parser::parse_variable(lexer::tokeniser &parent_toker, lexer::token tok)
    {
        auto &[str, type, line, column] = tok;

        YAPL_EXPECT(type == lexer::token_type::identifier, "variable type")
        auto vtype = str;

        auto tmp_tok = parent_toker;
        tok = tmp_tok();

        bool array = false;
        std::size_t array_size = 0;
        if (type == lexer::token_type::open_square)
        {
            array = true;
            tok = tmp_tok();
            if (type == lexer::token_type::number)
            {
                if (str.find('.') != std::string::npos)
                    throw log::parse::error(this->parent.filename, line, column, "Array size must be an integer");

                array_size = std::stoull(str, nullptr, 0);
                tok = tmp_tok();
            }
            YAPL_EXPECT(type == lexer::token_type::close_square, "']'")

            tok = tmp_tok();
        }
        YAPL_EXPECT(type == lexer::token_type::colon, "':'")

        tok = tmp_tok();
        YAPL_EXPECT(type == lexer::token_type::identifier, "variable name")

        auto vname = str;

        if (array == true)
        {
            (void)array_size;
            // TODO
        }

        parent_toker = tmp_tok;
        return std::make_pair(vtype, vname);
    }

    std::unique_ptr<func::function> parser::parse_function(lexer::tokeniser &parent_toker, lexer::token tok)
    {
        auto &[str, type, line, column] = tok;
        YAPL_EXPECT(type == lexer::token_type::func, "function entry")

        auto tmp_tok = parent_toker;
        tok = tmp_tok();

        YAPL_EXPECT(type == lexer::token_type::identifier, "function name")
        auto func_name = str;

        auto read_params = [&]
        {
            YAPL_EXPECT(type == lexer::token_type::open_round, "'('")

            std::vector<std::unique_ptr<func::parameter>> parameters;
            bool first_param = true;
            while (true)
            {
                tok = tmp_tok();
                if (type == lexer::token_type::close_round)
                    break;

                if (first_param == false)
                {
                    YAPL_EXPECT(type == lexer::token_type::comma, "','")
                    tok = tmp_tok();
                }
                else first_param = false;

                auto [param_type, param_name] = this->parse_variable(tmp_tok, tok);

                auto type = this->get_type(param_type);
                if (type == nullptr)
                    throw log::parse::error(this->parent.filename, line, column, "Type '{}' does not exist", str);

                parameters.emplace_back(
                    std::make_unique<func::parameter>(
                        type,
                        param_name
                    )
                );
            }
            YAPL_EXPECT(type == lexer::token_type::close_round, "')'")

            return parameters;
        };

        tok = tmp_tok();
        auto parameters = read_params();

        tok = tmp_tok();
        YAPL_EXPECT(type == lexer::token_type::rarrow, "'->'")

        tok = tmp_tok();
        YAPL_EXPECT(type == lexer::token_type::identifier, "return type")

        auto ret_type = this->get_type(str);
        if (ret_type == nullptr)
            throw log::parse::error(this->parent.filename, line, column, "Type '{}' does not exist", str);

        tok = tmp_tok();
        YAPL_EXPECT(type == lexer::token_type::open_curly, "'{'")

        tok = tmp_tok();
        while (true) // TMP
        {
            YAPL_EXPECT(lexer::is_expression(type) || type == lexer::token_type::semicolon, "an expression")

            try {
                auto var = this->parse_variable(tmp_tok, tok);
                log::println("variable '{}' '{}'", var.first, var.second);

                goto end;
            }
            catch (const log::lex::error &e) { throw; }
            catch (...) { }

            try {
                YAPL_EXPECT(type == lexer::token_type::ret, "'return'")
                tok = tmp_tok();

                YAPL_EXPECT(lexer::is_expression(type) || type == lexer::token_type::semicolon, "an expression")

                log::println("return '{}'", str);

                goto end;
            }
            catch (const log::lex::error &e) { throw; }
            catch (...) { }

            end:

            tok = tmp_tok();
            YAPL_EXPECT(type == lexer::token_type::semicolon, "';'")

            tok = tmp_tok();
            if (type == lexer::token_type::close_curly)
                break;
        }
        YAPL_EXPECT(type == lexer::token_type::close_curly, "'}'")

        parent_toker = tmp_tok;
        return std::make_unique<func::function>(func_name, std::move(parameters), ret_type);
    }

    void parser::parse()
    {
        auto tok = this->tokeniser();
        auto &[str, type, line, column] = tok;

        while (true)
        {
            if (type == lexer::token_type::eof)
                break;

            this->parent.func_registry.push_back(this->parse_function(this->tokeniser, tok));
            tok = this->tokeniser();
        }
    }
#undef YAPL_EXPECT
} // namespace yapl::ast