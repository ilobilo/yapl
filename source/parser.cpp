// Copyright (C) 2022-2024  ilobilo

#include <yapl/parser.hpp>
#include <yapl/lexer.hpp>
#include <yapl/yapl.hpp>
#include <yapl/log.hpp>

#include <utility>

namespace yapl::ast
{
    const types::type *parser::get_type(std::string_view name, std::size_t array_size) const
    {
        if (!this->parent.type_registry.normal.contains(name))
            return nullptr;

        auto type = this->parent.type_registry.normal.at(name).get();

        if (array_size > 1)
        {
            auto pair = std::make_pair(name, array_size);
            if (this->parent.type_registry.arrays.contains(pair))
                return this->parent.type_registry.arrays.at(pair).get();

            return (this->parent.type_registry.arrays[pair] = std::make_unique<types::array>(type, array_size)).get();
        }
        else if (array_size == 1)
        {
            if (this->parent.type_registry.pointers.contains(name))
                return this->parent.type_registry.pointers.at(name).get();

            return (this->parent.type_registry.pointers[name] = std::make_unique<types::pointer>(type)).get();
        }

        return type;
    }

#define YAPL_EXPECT(x, exp)                                                                               \
    do {                                                                                                  \
        if (!(x)) {                                                                                       \
            if (should_throw)                                                                             \
                throw log::error(this->parent.filename, line, column, "Expected {}, got '{}'", exp, str); \
            else                                                                                          \
                throw log::empty_error { };                                                               \
        }                                                                                                 \
    } while (0)

#define YAPL_EXPECT_TOK(x, exp) YAPL_EXPECT(type == x, exp)

    std::tuple<std::string, std::size_t> parser::parse_type(lexer::tokeniser &toker_parent, lexer::token tok, bool should_throw)
    {
        auto &[str, type, line, column] = tok;

        YAPL_EXPECT_TOK(lexer::token_type::identifier, "a type");
        auto vtype = str;

        auto tmp_tok = toker_parent;
        tok = tmp_tok.peek();

        std::size_t array_size = 0;
        if (type == lexer::token_type::open_square)
        {
            tmp_tok();
            tok = tmp_tok();

            if (type != lexer::token_type::close_square)
            {
                if (type != lexer::token_type::number || str.starts_with('-') || str.find('.') != std::string::npos)
                    throw log::error(this->parent.filename, line, column, "Array size must be a positive integer");

                array_size = std::stoull(str, nullptr, 0);
                if (array_size < 2)
                    throw log::error(this->parent.filename, line, column, "Array size must be more than 1");

                tok = tmp_tok();

                YAPL_EXPECT_TOK(lexer::token_type::close_square, "']'");
            }
            else array_size = 1; // <- type[] means pointer
        }

        toker_parent = tmp_tok;
        return std::make_tuple(vtype, array_size);
    }

    std::tuple<std::string, std::string, std::size_t> parser::parse_variable(lexer::tokeniser &toker_parent, lexer::token tok, bool should_throw)
    {
        auto [vtype, array_size] = this->parse_type(toker_parent, tok, should_throw);

        auto tmp_tok = toker_parent;
        tok = tmp_tok();

        auto &[str, type, line, column] = tok;

        YAPL_EXPECT_TOK(lexer::token_type::colon, "':'");

        tok = tmp_tok();
        YAPL_EXPECT_TOK(lexer::token_type::identifier, "a variable name");

        toker_parent = tmp_tok;
        return std::make_tuple(str, vtype, array_size);
    }

    std::unique_ptr<expressions::expression> parser::parse_expression(lexer::tokeniser &toker_parent, lexer::token tok, bool should_throw)
    {
        // TODO
        return nullptr;
    }

    std::unique_ptr<func::function> parser::parse_function(lexer::tokeniser &toker_parent, lexer::token tok, bool should_throw)
    {
        auto &[str, type, line, column] = tok;
        YAPL_EXPECT_TOK(lexer::token_type::func, "a function entry");

        auto tmp_tok = toker_parent;
        tok = tmp_tok();

        YAPL_EXPECT_TOK(lexer::token_type::identifier, "a function name");
        auto func_name = str;

        auto read_params = [&]
        {
            YAPL_EXPECT_TOK(lexer::token_type::open_round, "'('");

            std::vector<std::unique_ptr<statements::variable>> parameters;
            bool first_param = true;
            while (true)
            {
                tok = tmp_tok();
                if (type == lexer::token_type::close_round)
                    break;

                if (first_param == false)
                {
                    YAPL_EXPECT_TOK(lexer::token_type::comma, "','");
                    tok = tmp_tok();
                }
                else first_param = false;

                auto [param_name, param_type, array_size] = this->parse_variable(tmp_tok, tok, should_throw);

                auto ptype = this->get_type(param_type, array_size);
                if (ptype == nullptr)
                    throw log::error(this->parent.filename, line, column, "Type '{}' does not exist", str);

                parameters.emplace_back(
                    std::make_unique<statements::variable>(
                        ptype, param_name
                    )
                );
            }
            YAPL_EXPECT_TOK(lexer::token_type::close_round, "')'");

            return parameters;
        };

        tok = tmp_tok();
        auto parameters = read_params();
        tok = tmp_tok();

        const types::type *ret_type = nullptr;
        bool is_ret_void = true;

        if (type == lexer::token_type::rarrow)
        {
            tok = tmp_tok();

            auto [type_name, array_size] = this->parse_type(tmp_tok, tok, should_throw);

            ret_type = this->get_type(type_name, array_size);
            if (ret_type == nullptr)
                throw log::error(this->parent.filename, line, column, "Type '{}' does not exist", type_name);

            is_ret_void = (type_name == "void");

            tok = tmp_tok();
        }
        YAPL_EXPECT_TOK(lexer::token_type::open_curly, "'{'");
        tok = tmp_tok();

        std::vector<statements::statement *> body;
        std::size_t levels = 0;

        if (type == lexer::token_type::close_curly)
            goto skip;

        while (true) // TODO: wth is this abomination
        {
            if (type == lexer::token_type::ret)
            {
                tok = tmp_tok();

                if (is_ret_void == false)
                {
                    YAPL_EXPECT(lexer::is_expression(type) || type == lexer::token_type::open_curly, "an expression");

                    body.push_back(new statements::return_statement(this->parse_expression(tmp_tok, tok)));
                    tok = tmp_tok();
                }
                else body.push_back(new statements::return_statement(nullptr));

                YAPL_EXPECT_TOK(lexer::token_type::semicolon, "';'");
            }
            else
            {
                try {
                    auto [vname, vtypename, array_size] = this->parse_variable(tmp_tok, tok, false);
                    std::unique_ptr<expressions::expression> value { nullptr };

                    tok = tmp_tok();
                    if (type == lexer::token_type::assign)
                    {
                        tok = tmp_tok();
                        value = this->parse_expression(tmp_tok, tok);
                        tok = tmp_tok();
                    }
                    YAPL_EXPECT_TOK(lexer::token_type::semicolon, "';'");

                    auto vtype = this->get_type(vtypename, array_size);
                    if (vtype == nullptr)
                        throw log::error(this->parent.filename, line, column, "Type '{}' does not exist", vtypename);

                    body.emplace_back(new statements::variable(vtype, vname));

                    goto end;
                }
                catch (const log::empty_error &) { }
                catch (...) { throw; }
            }
            end:

            tok = tmp_tok();
            if (levels == 0 && type == lexer::token_type::close_curly)
                break;
        }

        skip:
        toker_parent = tmp_tok;
        return std::make_unique<func::function>(func_name, std::move(parameters), ret_type, std::move(body));
    }
#undef YAPL_EXPECT_TOK
#undef YAPL_EXPECT

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
} // namespace yapl::ast