// Copyright (C) 2022-2024  ilobilo

#pragma once

#include <fstream>
#include <deque>

#include <string_view>
#include <string>

#include <cstdint>
#include <cstddef>

namespace yapl::lexer
{
    enum class token_type : std::uint8_t
    {
        eof, // space,

        operators_start,

        inc, dec,

        add, add_assign,
        sub, sub_assign,
        mul, mul_assign,
        div, div_assign,
        mod, mod_assign,

        bw_not,
        bw_and, bw_and_assign,
        bw_or,  bw_or_assign,
        bw_xor, bw_xor_assign,

        shiftl, shiftl_assign,
        shiftr, shiftr_assign,

        assign,

        log_not,
        log_and, log_and_assign,
        log_or,  log_or_assign,
        log_xor, log_xor_assign,

        eq, ne, lt,
        gt, le, ge,

        question,
        colon, semicolon,

        comma, dot,

        open_round,  close_round,
        open_curly,  close_curly,
        open_square, close_square,

        larrow, rarrow,

        operators_end,

        func,
        ret,

        _true,
        _false,
        null,

        number,
        string,
        identifier
    };
    constexpr bool is_operator(token_type type)
    {
        return type > token_type::operators_start && type < token_type::operators_end;
    }

    enum class digit_type : std::uint8_t
    {
        binary,
        decimal,
        octal,
        hexadecimal
    };

    enum class char_type : std::uint8_t
    {
        eof,
        space,
        punct,
        other
    };

    struct token
    {
        std::string name;
        token_type type;

        std::size_t line;
        std::size_t column;
    };

    struct tokeniser
    {
        using stream_type = std::ifstream;

        private:
        stream_type _stream;
        std::size_t _line;
        std::size_t _column;

        std::string _filename;

        int peekc();
        int getc();

        std::deque<token> peek_queue;
        token next();

        public:
        tokeniser(std::string_view filename, stream_type &stream) :
            _stream(std::move(stream)),
            _line(0), _column(0), _filename(filename) { }

        tokeniser(const tokeniser &) = delete;
        tokeniser &operator=(const tokeniser &) = delete;

        tokeniser(tokeniser &&) = default;
        tokeniser &operator=(tokeniser &&) = default;

        ~tokeniser() = default;

        token peek(std::size_t n = 1);
        token get();

        auto operator()() -> token
        {
            return this->get();
        }

        std::string_view filename() const
        {
            return this->_filename;
        }

        std::size_t line() const
        {
            return this->_line + 1;
        }
        std::size_t column() const
        {
            return this->_column - 1;
        }
    };
} // namespace yapl::lexer