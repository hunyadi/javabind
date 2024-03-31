/**
 * javabind: effective C++ and Java interoperability
 * @see https://github.com/hunyadi/javabind
 *
 * Copyright (c) 2024 Levente Hunyadi
 *
 * This work is licensed under the terms of the MIT license.
 * For a copy, see <https://opensource.org/licenses/MIT>.
 */

#pragma once
#include "string.hpp"

namespace javabind
{
    template <typename T>
    struct ArgType;

    template <typename>
    struct FunctionTraits;

    template <typename R, typename... Args>
    struct FunctionTraits<R(Args...)>
    {
    private:
        template <std::string_view const& param_sig, std::string_view const& return_sig>
        struct callable_sig {
            constexpr static std::string_view lparen = "(";
            constexpr static std::string_view rparen = ")";
            constexpr static std::string_view value = join_v<lparen, param_sig, rparen, return_sig>;
        };

        constexpr static std::string_view param_sig = join_v<ArgType<Args>::type::sig...>;
        constexpr static std::string_view return_sig = ArgType<R>::type::sig;

    public:
        /** Java signature string used internally for type lookup. */
        constexpr static std::string_view sig = callable_sig<param_sig, return_sig>::value;
    };

    /**
     * Extracts a Java signature from a native free (non-member) function.
     */
    template <typename R, typename... Args>
    struct FunctionTraits<R(*)(Args...)>
    {
        constexpr static std::string_view sig = FunctionTraits<R(std::decay_t<Args>...)>::sig;
    };

    /**
     * Extracts a Java signature from a native member function.
     */
    template <typename T, typename R, typename... Args>
    struct FunctionTraits<R(T::*)(Args...)>
    {
        constexpr static std::string_view sig = FunctionTraits<R(std::decay_t<Args>...)>::sig;
    };

    /**
     * Extracts a Java signature from a const-qualified native member function.
     */
    template <typename T, typename R, typename... Args>
    struct FunctionTraits<R(T::*)(Args...) const>
    {
        constexpr static std::string_view sig = FunctionTraits<R(std::decay_t<Args>...)>::sig;
    };
}
