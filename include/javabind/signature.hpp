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
    struct FieldType;

    template <typename T, typename R>
    struct FieldType<R(T::*)> {
        using type = R;
    };

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

        constexpr static std::string_view comma = ", ";
        constexpr static std::string_view param_sig = join_v<ArgType<Args>::type::sig...>;
        constexpr static std::string_view return_sig = ArgType<R>::type::sig;

    public:
        /** Java signature string used internally for type lookup. */
        constexpr static std::string_view sig = callable_sig<param_sig, return_sig>::value;

        constexpr static std::string_view param_display = join_sep_v<comma, ArgType<Args>::type::java_name...>;
        constexpr static std::string_view return_display = ArgType<R>::type::java_name;
    };

    /**
     * Extracts a Java signature from a native free (non-member) function.
     */
    template <typename R, typename... Args>
    struct FunctionTraits<R(*)(Args...)>
    {
        constexpr static std::string_view sig = FunctionTraits<R(std::decay_t<Args>...)>::sig;
        constexpr static std::string_view param_display = FunctionTraits<R(std::decay_t<Args>...)>::param_display;
        constexpr static std::string_view return_display = FunctionTraits<R(std::decay_t<Args>...)>::return_display;
    };

    /**
     * Extracts a Java signature from a native member function.
     */
    template <typename T, typename R, typename... Args>
    struct FunctionTraits<R(T::*)(Args...)>
    {
        constexpr static std::string_view sig = FunctionTraits<R(std::decay_t<Args>...)>::sig;
        constexpr static std::string_view param_display = FunctionTraits<R(std::decay_t<Args>...)>::param_display;
        constexpr static std::string_view return_display = FunctionTraits<R(std::decay_t<Args>...)>::return_display;
    };

    /**
     * Extracts a Java signature from a const-qualified native member function.
     */
    template <typename T, typename R, typename... Args>
    struct FunctionTraits<R(T::*)(Args...) const>
    {
        constexpr static std::string_view sig = FunctionTraits<R(std::decay_t<Args>...)>::sig;
        constexpr static std::string_view param_display = FunctionTraits<R(std::decay_t<Args>...)>::param_display;
        constexpr static std::string_view return_display = FunctionTraits<R(std::decay_t<Args>...)>::return_display;
    };

    template <std::string_view const& Name, typename... Args>
    struct GenericTraits
    {
    private:
        constexpr static std::string_view lparen = "<";
        constexpr static std::string_view rparen = ">";
        constexpr static std::string_view sep = ", ";
        constexpr static std::string_view param_list = join_sep_v<sep, ArgType<Args>::type::java_name...>;

    public:
        constexpr static std::string_view java_name = join_v<Name, lparen, param_list, rparen>;
    };
}
