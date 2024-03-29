// Copyright (C) 2022-2024  ilobilo

#pragma once

#include <fmt/color.h>

#include <exception>

#include <string_view>
#include <string>

#include <cstdint>
#include <cstddef>

namespace yapl::log
{
    enum class level : std::uint8_t
    {
        note,
        warning,
        error
    };

    consteval auto level2str(level lvl)
    {
        switch (lvl)
        {
            case level::note:
                return "\033[1m\033[90mnote:\033[0m"; // bright black
            case level::warning:
                return "\033[1m\033[35mwarning:\033[0m"; // magenta
            case level::error:
                return "\033[1m\033[31merror:\033[0m"; // red
            default:
                __builtin_unreachable();
        }
    }

    template<level lvl, typename ...Args>
    void println(fmt::format_string<Args...> msg, Args &&...args) noexcept
    {
        fmt::println(lvl == level::error ? stderr : stdout, "{}",
            fmt::format(fmt::emphasis::bold, "{} {}", level2str(lvl),
                fmt::styled(
                    fmt::format(msg, std::forward<Args>(args)...),
                    fmt::emphasis::bold
                )
            )
        );
    }

    namespace parse
    {
        template<level lvl, typename ...Args>
        constexpr auto format(std::string_view file, std::size_t line, std::size_t column, fmt::format_string<Args...> msg, Args &&...args) noexcept
        {
            return fmt::format(fmt::emphasis::bold, "{}:{}:{}: {} {}",
                file, line, column, level2str(lvl),
                fmt::styled(
                    fmt::format(msg, std::forward<Args>(args)...),
                    fmt::emphasis::bold
                )
            );
        }

        class error final : public std::exception
        {
            private:
            std::string _msg;

            public:
            template<typename ...Args>
            constexpr error(std::string_view file, std::size_t line, std::size_t column, fmt::format_string<Args...> msg, Args &&...args) noexcept
                : _msg(format<level::error>(file, line, column, msg, std::forward<Args>(args)...)) { }

            constexpr error(const error &) = default;
            constexpr error(error &&) = default;

            error &operator=(const error &) = default;
            error &operator=(error &&) = default;

            ~error() override = default;

            [[nodiscard]] const char *what() const noexcept override
            {
                return this->_msg.c_str();
            }
        };
    } // namespace parse
} // namespace yapl::log