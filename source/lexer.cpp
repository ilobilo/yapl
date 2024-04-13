// Copyright (C) 2022-2024  ilobilo

#include <frozen/unordered_map.h>
#include <frozen/map.h>
#include <frozen/string.h>

#include <magic_enum.hpp>

#include <yapl/lexer.hpp>
#include <yapl/log.hpp>

#include <cassert>
#include <cstdio>

namespace yapl::lexer
{
    namespace
    {
        constexpr auto lookup = frozen::make_unordered_map<frozen::string, token_type>
        ({
            { "++", token_type::inc },
            { "--", token_type::dec },

            { "+", token_type::add },
            { "+=", token_type::add_assign },
            { "-", token_type::sub },
            { "-=", token_type::sub_assign },
            { "*", token_type::mul },
            { "*=", token_type::mul_assign },
            { "/", token_type::div },
            { "/=", token_type::div_assign },
            { "%", token_type::mod },
            { "%=", token_type::mod_assign },

            { "~", token_type::bw_not },

            { "&", token_type::bw_and },
            { "&=", token_type::bw_and_assign },
            { "|", token_type::bw_or },
            { "|=", token_type::bw_or_assign },
            { "^", token_type::bw_xor },
            { "^=", token_type::bw_xor_assign },

            { "<<", token_type::shiftl },
            { "<<=", token_type::shiftl_assign },
            { ">>", token_type::shiftr },
            { ">>=", token_type::shiftr_assign },

            { "=", token_type::assign },

            { "!", token_type::log_not },
            { "not", token_type::log_not },

            { "&&", token_type::log_and },
            { "&&=", token_type::log_and_assign },
            { "and", token_type::log_and },

            { "||", token_type::log_or },
            { "||=", token_type::log_or_assign },
            { "or", token_type::log_or },

            { "^^", token_type::log_xor },
            { "^^=", token_type::log_xor_assign },
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
            { "true", token_type::_true },
            { "false", token_type::_false },
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

        char_type get_char_type(auto chr)
        {
            if (chr == EOF)
                return char_type::eof;

            if (std::isspace(chr))
                return char_type::space;

            if (std::ispunct(chr) && chr != '_')
                return char_type::punct;

            return char_type::other;
        }

        bool is_num_type(digit_type type, auto chr)
        {
            switch (type)
            {
                case digit_type::binary:
                    return chr == '0' || chr == '1';
                case digit_type::decimal:
                    return std::isdigit(chr);
                case digit_type::octal:
                    return std::isdigit(chr) && (chr < '8');
                case digit_type::hexadecimal:
                    return std::isxdigit(chr);
            }
            __builtin_unreachable();
        }
    } // namespace

    int tokeniser::peekc()
    {
        return this->_stream.peek();
    }

    int tokeniser::getc()
    {
        auto chr = this->_stream.get();
        if (chr == '\n')
        {
            this->_line++;
            this->_column = 0;
        }
        else this->_column++;

        return chr;
    }

    token tokeniser::next()
    {
        while (true)
        {
            bool extra = false;
            switch (auto chr = this->getc(); get_char_type(chr))
            {
                case char_type::eof:
                    return { "eof", token_type::eof, this->line(), this->column() };
                case char_type::space:
                    // return { " ", token_type::space, std::nullopt, this->line(), this->column() };
                    break;
                case char_type::punct:
                    switch (chr)
                    {
                        case '"': // "strings"
                        {
                            bool escape = false;
                            std::string str;

                            auto sline = this->line();
                            auto scolumn = this->column();

                            for (chr = this->getc(); get_char_type(chr) != char_type::eof; chr = this->getc())
                            {
                                if (chr != '\\')
                                {
                                    if (escape == true)
                                    {
                                        if (auto iter = escapes.find(chr); iter != escapes.end())
                                            str += iter->second;
                                        else
                                            str += chr;

                                        escape = false;
                                    }
                                    else
                                    {
                                        if (chr == '"')
                                            return { str, token_type::string, sline, scolumn };
                                        str += chr;
                                    }
                                }
                                else escape = true;
                            }
                            throw log::lex::error(this->filename(), sline, scolumn, "Expected closing '\"'");
                        }
                        case '/':
                        {
                            auto nchr = this->peekc();
                            if (nchr == '/') // line comment
                            {
                                chr = this->getc();
                                while (get_char_type(chr) != char_type::eof && chr != '\n')
                                    chr = this->getc();
                                break;
                            }

                            if (nchr == '*') /* block comment */
                            {
                                bool close = false;

                                for (chr = this->getc(); get_char_type(chr) != char_type::eof; chr = this->getc())
                                {
                                    if (close == true && chr == '/')
                                        break;
                                    close = (chr == '*');
                                }
                                break;
                            }
                        }
                        case '-':
                            if (std::isdigit(this->peekc()))
                                goto negative_number;

                            [[fallthrough]];
                        default: // operators
                        {
                            std::string name(1, chr);

                            auto sline = this->line();
                            auto scolumn = this->column();

                            for (chr = this->peekc(); get_char_type(chr) == char_type::punct; chr = this->peekc())
                            {
                                name += chr;
                                if (lookup.find(frozen::string(name)) == lookup.end())
                                {
                                    name.erase(name.length() - 1);
                                    break;
                                }
                                this->getc();
                            }

                            const auto *iter = lookup.find(frozen::string(name));
                            if (iter == lookup.end())
                                throw log::lex::error(this->filename(), sline, scolumn, "Unknown operator '{}'", name);

                            return { name, iter->second, sline, scolumn };
                        }
                    }
                    break;
                default:
                {
                    goto not_negative_number;

                    negative_number:
                    extra = true;

                    not_negative_number:
                    std::string str(1, chr);

                    if (extra == true)
                        str += (chr = this->getc());

                    auto sline = this->line();
                    auto scolumn = this->column();

                    const bool digit = std::isdigit(chr);
                    auto type = digit_type::decimal;

                    if (digit == true) // numbers
                    {
                        bool invalid_number = false;

                        auto fits_64bit = [&]
                        {
                            errno = 0;
                            std::strtoull(str.c_str(), nullptr, 0);
                            if (errno == ERANGE)
                                return false;
                            return true;
                        };

                        auto oldc = chr;
                        chr = this->peekc();

                        if (get_char_type(chr) != char_type::other)
                        {
                            if (fits_64bit() == false)
                            {
                                invalid_number = true;
                                goto num_invalid;
                            }
                            return { str, token_type::number, sline, scolumn };
                        }

                        if (oldc == '0')
                        {
                            if (chr == 'b')
                                type = digit_type::binary;
                            else if (chr == 'x' || chr == 'X')
                                type = digit_type::hexadecimal;
                            else
                            {
                                type = digit_type::octal;
                                if (is_num_type(digit_type::octal, chr) == false)
                                    invalid_number = true;
                            }
                        }

                        if (type != digit_type::decimal)
                        {
                            str += this->getc();
                            chr = this->peekc();

                            if (is_num_type(type, chr) == false)
                                invalid_number = true;
                        }

                        do {
                            if (get_char_type(chr) == char_type::other && is_num_type(type, chr) == false)
                                invalid_number = true;

                            str += this->getc();
                            chr = this->peekc();
                        } while (get_char_type(chr) == char_type::other);

                        if (fits_64bit() == false)
                            invalid_number = true;

                        num_invalid:
                        if (invalid_number == true)
                            throw log::lex::error(this->filename(), sline, scolumn, "Invalid {} number '{}'", magic_enum::enum_name(type), str);
                    }
                    else // identifiers and more operators
                    {
                        for (chr = this->peekc(); get_char_type(chr) == char_type::other; chr = this->peekc())
                            str += this->getc();

                        if (const auto *iter = lookup.find(frozen::string(str)); iter != lookup.end())
                            return { str, iter->second, sline, scolumn };
                    }

                    return { str, digit ? token_type::number : token_type::identifier, sline, scolumn };
                }
            }
        }
    }

    token tokeniser::peek(std::size_t n)
    {
        assert(n > 0);

        auto consume = [&]
        {
            const token tok = this->next();
            this->peek_queue.push_back(tok);
            if (tok.type == token_type::eof)
            {
                n = this->peek_queue.size();
                return false;
            }
            return true;
        };

        for (std::size_t i = this->peek_queue.size(); i < n; i++)
            if (consume() == false)
                break;

        return this->peek_queue.at(n - 1);
    }

    token tokeniser::get()
    {
        if (this->peek_queue.empty() == false)
        {
            auto ret = this->peek_queue.front();
            this->peek_queue.pop_front();
            return ret;
        }
        return this->next();
    }
} // namespace yapl::lexer