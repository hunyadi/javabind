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
#include "type.hpp"
#include "string.hpp"
#include <type_traits>

namespace javabind
{
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
        constexpr static std::string_view param_sig = join_v<arg_type_t<Args>::sig...>;
        constexpr static std::string_view return_sig = arg_type_t<R>::sig;

    public:
        /** Java signature string used internally for type lookup. */
        constexpr static std::string_view sig = callable_sig<param_sig, return_sig>::value;

        template<typename A, std::size_t I>
        struct single_param_display
        {
            static constexpr std::string_view space = " ";
            static constexpr std::string_view arg_name = "arg";
            static constexpr std::string_view value = join_v<arg_type_t<A>::java_name, space, arg_name, to_string<I>::value>;
        };

        template <std::size_t... Is>
        static constexpr std::string_view make_param_display(std::index_sequence<Is...>) {
            return join_sep_v<comma, single_param_display<Args, Is>::value...>;
        }

        constexpr static std::string_view param_display = make_param_display(std::index_sequence_for<Args...>{});
        constexpr static std::string_view return_display = arg_type_t<R>::java_name;
    };

    /**
     * Extracts a Java signature from a native free (non-member) function.
     */
    template <typename R, typename... Args>
    struct FunctionTraits<R(*)(Args...)> : public FunctionTraits<R(Args...)>
    {
    };

    /**
     * Extracts a Java signature from a native member function.
     */
    template <typename T, typename R, typename... Args>
    struct FunctionTraits<R(T::*)(Args...)> : public FunctionTraits<R(Args...)>
    {
    };

    /**
     * Extracts a Java signature from a const-qualified native member function.
     */
    template <typename T, typename R, typename... Args>
    struct FunctionTraits<R(T::*)(Args...) const> : public FunctionTraits<R(Args...)>
    {
    };

    template <std::string_view const& Name, typename... Args>
    struct GenericTraits
    {
    private:
        constexpr static std::string_view lparen = "<";
        constexpr static std::string_view rparen = ">";
        constexpr static std::string_view sep = ", ";
        constexpr static std::string_view param_list = join_sep_v<sep, arg_type_t<Args>::java_name...>;

    public:
        constexpr static std::string_view java_name = join_v<Name, lparen, param_list, rparen>;
    };
}
