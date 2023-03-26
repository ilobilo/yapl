// Copyright (C) 2022  ilobilo

#include <frozen/unordered_map.h>
#include <frozen/string.h>
#include <frozen/map.h>

#include <magic_enum.hpp>

#include <lexer/lexer.hpp>
#include <log.hpp>

namespace yapl::lexer
{
    constexpr auto lookup = frozen::make_unordered_map<frozen::string, std::pair<token_type, tokens>>
    ({
        { "++", { token_type::toperator, tokens::inc } },
        { "--", { token_type::toperator, tokens::dec } },

        { "+", { token_type::toperator, tokens::add } },
        { "-", { token_type::toperator, tokens::sub } },
        { "*", { token_type::toperator, tokens::mul } },
        { "/", { token_type::toperator, tokens::div } },
        { "%", { token_type::toperator, tokens::mod } },

        { "~", { token_type::toperator, tokens::bw_not } },
        { "bitnot", { token_type::toperator, tokens::bw_not } },

        { "&", { token_type::toperator, tokens::bw_and } },
        { "bitand", { token_type::toperator, tokens::bw_and } },

        { "|", { token_type::toperator, tokens::bw_or } },
        { "bitor", { token_type::toperator, tokens::bw_or } },

        { "^", { token_type::toperator, tokens::bw_xor } },
        { "bitxor", { token_type::toperator, tokens::bw_xor } },

        { "<<", { token_type::toperator, tokens::shiftl } },
        { ">>", { token_type::toperator, tokens::shiftr } },

        { "=", { token_type::toperator, tokens::assign } },

        { "+=", { token_type::toperator, tokens::add_assign } },
        { "-=", { token_type::toperator, tokens::sub_assign } },
        { "*=", { token_type::toperator, tokens::mul_assign } },
        { "/=", { token_type::toperator, tokens::div_assign } },
        { "%=", { token_type::toperator, tokens::mod_assign } },

        { "&=", { token_type::toperator, tokens::and_assign } },
        { "and_eq", { token_type::toperator, tokens::and_assign } },

        { "|=", { token_type::toperator, tokens::or_assign } },
        { "or_eq", { token_type::toperator, tokens::or_assign } },

        { "^=", { token_type::toperator, tokens::xor_assign } },
        { "xor_eq", { token_type::toperator, tokens::xor_assign } },

        { "<<=", { token_type::toperator, tokens::shiftl_assign } },
        { ">>=", { token_type::toperator, tokens::shiftr_assign } },

        { "!", { token_type::toperator, tokens::log_not } },
        { "not", { token_type::toperator, tokens::log_not } },

        { "&&", { token_type::toperator, tokens::log_and } },
        { "and", { token_type::toperator, tokens::log_and } },

        { "||", { token_type::toperator, tokens::log_or } },
        { "or", { token_type::toperator, tokens::log_or } },

        { "^^", { token_type::toperator, tokens::log_xor } },
        { "xor", { token_type::toperator, tokens::log_xor } },

        { "==", { token_type::toperator, tokens::eq } },
        { "!=", { token_type::toperator, tokens::ne } },
        { "<", { token_type::toperator, tokens::lt } },
        { ">", { token_type::toperator, tokens::gt } },
        { "<=", { token_type::toperator, tokens::le } },
        { ">= ", { token_type::toperator, tokens::ge } },

        { "?", { token_type::toperator, tokens::question } },
        { ":", { token_type::toperator, tokens::colon } },
        { ";", { token_type::toperator, tokens::semicolon } },

        { ",", { token_type::toperator, tokens::comma } },
        { ".", { token_type::toperator, tokens::dot } },

        { "(", { token_type::toperator, tokens::open_round } },
        { ")", { token_type::toperator, tokens::close_round } },

        { "{", { token_type::toperator, tokens::open_curly } },
        { "}", { token_type::toperator, tokens::close_curly } },

        { "[", { token_type::toperator, tokens::open_square } },
        { "]", { token_type::toperator, tokens::close_square } },

        { "<-", { token_type::toperator, tokens::larrow } },
        { "->", { token_type::toperator, tokens::rarrow } },

        { "fun", { token_type::identifier, tokens::func } },
        { "return", { token_type::identifier, tokens::ret } },
        { "true", { token_type::identifier, tokens::ttrue } },
        { "false", { token_type::identifier, tokens::tfalse } },
        { "null", { token_type::identifier, tokens::null } }
    });

    constexpr auto escapes = frozen::make_map<char, char>
    ({
        { '0', '\x00' },
        { 'a', '\a' },
        { 'b', '\b' },
        { 't', '\t' },
        { 'n', '\n' },
        { 'v', '\v' },
        { 'f', '\f' },
        { 'r', '\r' },
        { 'e', '\x1b' }
    });

    static char_type get_char_type(auto c)
    {
        if (c == EOF)
            return char_type::eof;
        else if (std::isspace(c))
            return char_type::space;
        else if (std::ispunct(c) && c != '_')
            return char_type::punct;
        else
            return char_type::other;
    }

    static bool is_num_type(digit_type type, auto c)
    {
        switch (type)
        {
            case digit_type::binary:
                return c == '0' || c == '1';
            case digit_type::decimal:
                return std::isdigit(c);
            case digit_type::octal:
                return std::isdigit(c) && (c < '8');
            case digit_type::hexadecimal:
                return std::isxdigit(c);
        }
        __builtin_unreachable();
    }

    template<typename Stream>
    int tokeniser<Stream>::peekc()
    {
        return this->_stream.peek();
    }

    template<typename Stream>
    int tokeniser<Stream>::getc()
    {
        auto c = this->_stream.get();
        if (c == '\n')
        {
            this->_line++;
            this->_column = 0;
        }
        else this->_column++;

        return c;
    }

    template<typename Stream>
    token tokeniser<Stream>::next()
    {
        while (true)
        {
            switch (auto c = this->getc(); get_char_type(c))
            {
                case char_type::eof:
                    return { "", token_type::eof, std::nullopt, this->line(), this->column() };
                case char_type::space:
                    // return { " ", token_type::space, std::nullopt, this->line(), this->column() };
                    break;
                case char_type::punct:
                    switch (c)
                    {
                        case '"':
                        {
                            bool escape = false;
                            std::string str;

                            auto sline = this->line();
                            auto scolumn = this->column();

                            for (c = this->getc(); get_char_type(c) != char_type::eof; c = this->getc())
                            {
                                if (c == '\\')
                                    escape = true;
                                else
                                {
                                    if (escape == true)
                                    {
                                        if (auto it = escapes.find(c); it != escapes.end())
                                            str += it->second;
                                        else
                                            str += c;

                                        escape = false;
                                    }
                                    else
                                    {
                                        if (c == '"')
                                            return { str, token_type::string, std::nullopt, sline, scolumn };
                                        else
                                            str += c;

                                    }
                                }
                            }
                            throw log::error(this->file(), sline, scolumn, "Expected closing '\"'");
                        }
                        case '/':
                        {
                            auto nc = this->peekc();
                            if (nc == '/')
                            {
                                for (c = this->getc(); get_char_type(c) != char_type::eof && c != '\n'; c = this->getc()) { }
                                break;
                            }
                            else if (nc == '*')
                            {
                                bool close = false;

                                for (c = this->getc(); get_char_type(c) != char_type::eof; c = this->getc())
                                {
                                    if (close == true && c == '/')
                                        break;
                                    close = (c == '*');
                                }
                                break;
                            }
                        }
                        default:
                        {
                            std::string name(1, c);

                            auto sline = this->line();
                            auto scolumn = this->column();

                            for (c = this->peekc(); get_char_type(c) == char_type::punct; c = this->peekc())
                            {
                                name += c;
                                if (lookup.find(frozen::string(name)) == lookup.end())
                                {
                                    name.erase(name.length() - 1);
                                    break;
                                }
                                this->getc();
                            }

                            auto it = lookup.find(frozen::string(name));
                            if (it == lookup.end())
                                throw log::error(this->file(), sline, scolumn, "Unknown operator '{}'", name);

                            auto [type, tok] = it->second;
                            return { name, type, tok, sline, scolumn };
                        }
                    }
                    break;
                default:
                {
                    std::string str(1, c);

                    auto sline = this->line();
                    auto scolumn = this->column();

                    bool digit = std::isdigit(c);
                    auto type = digit_type::decimal;

                    if (digit == true)
                    {
                        auto oldc = c;
                        c = this->peekc();

                        if (get_char_type(c) != char_type::other)
                            return { str, token_type::number, std::nullopt, sline, scolumn };

                        bool invalid_number = false;

                        if (oldc == '0')
                        {
                            if (c == 'b')
                                type = digit_type::binary;
                            else if (c == 'x' || c == 'X')
                                type = digit_type::hexadecimal;
                            else
                            {
                                type = digit_type::octal;
                                if (is_num_type(digit_type::octal, c) == false)
                                    invalid_number = true;
                            }
                        }

                        if (type != digit_type::decimal)
                        {
                            str += this->getc();
                            c = this->peekc();

                            if (invalid_number == false && is_num_type(type, c) == false)
                                invalid_number = true;
                        }

                        do {
                            if (invalid_number == false && get_char_type(c) == char_type::other && is_num_type(type, c) == false)
                                invalid_number = true;

                            str += this->getc();
                            c = this->peekc();
                        } while (get_char_type(c) == char_type::other);

                        if (invalid_number == true)
                            throw log::error(this->file(), sline, scolumn, "Invalid {} number '{}'", magic_enum::enum_name(type), str);
                    }
                    else
                    {
                        for (c = this->peekc(); get_char_type(c) == char_type::other; c = this->peekc())
                            str += this->getc();

                        if (auto it = lookup.find(frozen::string(str)); it != lookup.end())
                        {
                            auto [type, tok] = it->second;
                            return { str, type, tok, sline, scolumn };
                        }
                    }

                    return { str, digit ? token_type::number : token_type::identifier, std::nullopt, sline, scolumn };
                }
            }
        }
    }

    template<typename Stream>
    token tokeniser<Stream>::peek(size_t n)
    {
        assert(n > 0);

        auto consume = [&]
        {
            token tok = this->next();
            this->peek_queue.push_back(tok);
            if (tok.type == token_type::eof)
            {
                n = this->peek_queue.size();
                return false;
            }
            return true;
        };

        for (size_t i = this->peek_queue.size(); i < n; i++)
            if (consume() == false)
                break;

        return this->peek_queue.at(n - 1);
    }

    template<typename Stream>
    token tokeniser<Stream>::get()
    {
        if (this->peek_queue.empty() == false)
        {
            auto ret = this->peek_queue.front();
            this->peek_queue.pop_front();
            return ret;
        }
        return this->next();
    }

    template<typename Stream>
    std::string_view tokeniser<Stream>::file()
    {
        return this->_file;
    }

    template<typename Stream>
    size_t tokeniser<Stream>::line()
    {
        return this->_line + 1;
    }

    template<typename Stream>
    size_t tokeniser<Stream>::column()
    {
        return this->_column - 1;
    }

    template struct tokeniser<std::ifstream>;
    template struct tokeniser<std::stringstream>;
} // namespace yapl::lexer