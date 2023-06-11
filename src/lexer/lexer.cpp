// Copyright (C) 2022  ilobilo

#include <frozen/unordered_map.h>
#include <frozen/string.h>
#include <frozen/map.h>

#include <magic_enum.hpp>

#include <lexer/lexer.hpp>
#include <log.hpp>

namespace yapl::lexer
{
    constexpr auto lookup = frozen::make_unordered_map<frozen::string, token_type>
    ({
        { "++", token_type::inc },
        { "--", token_type::dec },

        { "+", token_type::add },
        { "-", token_type::sub },
        { "*", token_type::mul },
        { "/", token_type::div },
        { "%", token_type::mod },

        { "~", token_type::bw_not },
        { "bitnot", token_type::bw_not },

        { "&", token_type::bw_and },
        { "bitand", token_type::bw_and },

        { "|", token_type::bw_or },
        { "bitor", token_type::bw_or },

        { "^", token_type::bw_xor },
        { "bitxor", token_type::bw_xor },

        { "<<", token_type::shiftl },
        { ">>", token_type::shiftr },

        { "=", token_type::assign },

        { "+=", token_type::add_assign },
        { "-=", token_type::sub_assign },
        { "*=", token_type::mul_assign },
        { "/=", token_type::div_assign },
        { "%=", token_type::mod_assign },

        { "&=", token_type::and_assign },
        { "and_eq", token_type::and_assign },

        { "|=", token_type::or_assign },
        { "or_eq", token_type::or_assign },

        { "^=", token_type::xor_assign },
        { "xor_eq", token_type::xor_assign },

        { "<<=", token_type::shiftl_assign },
        { ">>=", token_type::shiftr_assign },

        { "!", token_type::log_not },
        { "not", token_type::log_not },

        { "&&", token_type::log_and },
        { "and", token_type::log_and },

        { "||", token_type::log_or },
        { "or", token_type::log_or },

        { "^^", token_type::log_xor },
        { "xor", token_type::log_xor },

        { "==", token_type::eq },
        { "!=", token_type::ne },
        { "<", token_type::lt },
        { ">", token_type::gt },
        { "<=", token_type::le },
        { ">= ", token_type::ge },

        { "?", token_type::question },
        { ":", token_type::colon },
        { ";", token_type::semicolon },

        { ",", token_type::comma },
        { ".", token_type::dot },

        { "(", token_type::open_round },
        { ")", token_type::close_round },

        { "{", token_type::open_curly },
        { "}", token_type::close_curly },

        { "[", token_type::open_square },
        { "]", token_type::close_square },

        { "<-", token_type::larrow },
        { "->", token_type::rarrow },

        { "fun", token_type::func },
        { "return", token_type::ret },
        { "true", token_type::ttrue },
        { "false", token_type::tfalse },
        { "null", token_type::null }
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
                    return { "", token_type::eof, this->line(), this->column() };
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
                                            return { str, token_type::string, sline, scolumn };
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

                            return { name, it->second, sline, scolumn };
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
                            return { str, token_type::number, sline, scolumn };

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
                            return { str, it->second, sline, scolumn };
                        }
                    }

                    return { str, digit ? token_type::number : token_type::identifier, sline, scolumn };
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