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

namespace javabind
{
    /**
     * True if the function pointer is not bound to a class, e.g. free function or static member function.
     */
    template <typename F>
    struct is_unbound_function_pointer
        : std::integral_constant<bool, std::is_pointer_v<F>&& std::is_function_v<std::remove_pointer_t<F>>>
    {};

    template <typename...>
    struct types
    {
        using type = types;
    };

    /**
     * Gets a list of call argument types from a function-like type signature.
     */
    template <typename Sig>
    struct args;

    template <typename R, typename... Args>
    struct args<R(Args...)> : types<Args...> {};

    template <typename R, typename... Args>
    struct args<R(*)(Args...)> : args<R(Args...)> {};

    template <typename T, typename R, typename... Args>
    struct args<R(T::*)(Args...)> : args<R(Args...)> {};

    template <typename T, typename R, typename... Args>
    struct args<R(T::*)(Args...) const> : args<R(Args...)> {};

    template <typename Sig>
    using args_t = typename args<Sig>::type;
}
