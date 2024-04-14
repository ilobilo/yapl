// Copyright (C) 2022-2024  ilobilo

#pragma once

#include <fstream>
#include <deque>

#include <string_view>
#include <string>

#include <cstdint>
#include <cstddef>
#include <cassert>

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

        expressions_start,

        ret,

        _true,
        _false,
        null,

        number,
        string,
        identifier,

        expressions_end
    };
    constexpr bool is_operator(token_type type)
    {
        return type > token_type::operators_start && type < token_type::operators_end;
    }
    constexpr bool is_expression(token_type type)
    {
        return type > token_type::expressions_start && type < token_type::expressions_end;
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
        inline static std::size_t ids = 0;

        mutable stream_type _stream;
        mutable std::deque<token> _peek_queue;

        std::size_t _line;
        std::size_t _column;

        std::string _filename;
        std::size_t _id;

        int peekc();
        int getc();

        token next();

        public:
        tokeniser(std::string filename) :
            _stream { filename }, _peek_queue { }, _line { 0 }, _column { 0 },
            _filename { filename }, _id { ids++ } { }

        tokeniser(const tokeniser &other) :
            _stream { other._filename }, _peek_queue { other._peek_queue }, _line { other._line },
            _column { other._column }, _filename { other._filename }, _id { other._id }
        {
            this->_stream.seekg(other._stream.tellg());
        };

        tokeniser &operator=(const tokeniser &other)
        {
            assert(this->_id == other._id);

            this->_stream.seekg(other._stream.tellg());
            this->_peek_queue.swap(other._peek_queue);

            this->_column = other._column;
            this->_line = other._line;

            return *this;
        }

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