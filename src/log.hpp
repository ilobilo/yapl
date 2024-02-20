// Copyright (C) 2022  ilobilo

#pragma once

#include <fmt/color.h>
#include <exception>

namespace yapl::log
{
    enum class level
    {
        note,
        warning,
        error
    };

    inline constexpr auto level2str(level lvl)
    {
        switch (lvl)
        {
            case level::note:
                return fmt::format(fmt::fg(fmt::terminal_color::bright_black), FMT_STRING("note:"));
            case level::warning:
                return fmt::format(fmt::fg(fmt::terminal_color::magenta), FMT_STRING("warning:"));
            case level::error:
                return fmt::format(fmt::fg(fmt::terminal_color::red), FMT_STRING("error:"));
            default:
                __builtin_unreachable();
        }
    }

    template<level lvl, typename ...Args>
    inline auto format(std::string_view file, size_t line, size_t column, const auto &msg, Args &&...args) noexcept
    {
        return fmt::format(fmt::emphasis::bold, FMT_STRING("{}:{}:{}: {} {}"), file, line, column, level2str(lvl), fmt::format(fmt::emphasis::bold, fmt::runtime(msg), std::forward<Args>(args)...));
    }

    template<level lvl, typename ...Args>
    inline void println(std::string_view file, size_t line, size_t column, const auto &msg, Args &&...args) noexcept
    {
        fmt::println("{}", format<lvl>(file, line, column, msg, args...));
    }

    class error : public std::exception
    {
        private:
        std::string _msg;

        public:
        error(std::string_view file, size_t line, size_t column, const auto &msg, auto &&...args) noexcept
            : _msg(format<level::error>(file, line, column, msg, args...)) { }

        error(const error &) noexcept = default;
        const char *what() const noexcept
        {
            return this->_msg.c_str();
        }
    };
} // namespace yapl::log