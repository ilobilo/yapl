// Copyright (C) 2022  ilobilo

#pragma once

#include <optional>
#include <fstream>
#include <sstream>
#include <cstddef>
#include <deque>

namespace yapl::lexer
{
    enum class tokens
    {
        inc,
        dec,

        add,
        sub,
        mul,
        div,
        mod,

        bw_not,
        bw_and,
        bw_or,
        bw_xor,

        shiftl,
        shiftr,

        assign,

        add_assign,
        sub_assign,
        mul_assign,
        div_assign,
        mod_assign,

        and_assign,
        or_assign,
        xor_assign,
        shiftl_assign,
        shiftr_assign,

        log_not,
        log_and,
        log_or,
        log_xor,

        eq,
        ne,
        lt,
        gt,
        le,
        ge,

        question,
        colon,
        semicolon,

        comma,
        dot,

        open_round,
        close_round,

        open_curly,
        close_curly,

        open_square,
        close_square,

        larrow,
        rarrow,

        func,
        ret,

        ttrue,
        tfalse,
        null
    };

    enum class token_type
    {
        eof, // space,

        number,
        string,
        toperator,
        identifier
    };

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
        std::optional<tokens> tok;

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