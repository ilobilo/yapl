// Copyright (C) 2022  ilobilo

#pragma once

#include <string_view>
#include <string>
#include <deque>

namespace yapl::lexer
{
    enum class token_type
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
    inline constexpr bool is_operator(token_type type)
    {
        return type > token_type::operators_start && type < token_type::operators_end;
    }

    enum class digit_type
    {
        binary,
        decimal,
        octal,
        hexadecimal
    };

    enum class char_type
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

        size_t line;
        size_t column;
    };

    template<typename Stream>
    struct tokeniser
    {
        private:
        Stream _stream;
        size_t _line;
        size_t _column;

        std::string _file;

        int peekc();
        int getc();

        std::deque<token> peek_queue;
        token next();

        public:
        tokeniser(std::string_view file, Stream &stream) : _stream(std::move(stream)), _line(0), _column(0), _file(file), peek_queue() { }
        tokeniser(Stream &stream) : _stream(std::move(stream)), _line(0), _column(0), _file("in_memory"), peek_queue() { }

        tokeniser(const tokeniser &) = delete;
        void operator=(const tokeniser &) = delete;

        token peek(size_t n = 1);
        token get();

        auto operator()()
        {
            return this->get();
        }

        std::string_view file();
        size_t line();
        size_t column();
    };

    template<typename Stream>
    tokeniser(std::string_view, Stream &) -> tokeniser<Stream>;
} // namespace yapl::lexer