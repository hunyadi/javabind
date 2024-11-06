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
     * Used in static_assert to have the type name printed in the compiler error message.
     */
    template <typename>
    struct fail : std::false_type {};

    /**
     * Argument type traits, and argument type conversion between native and Java.
     *
     * Template substitution fails automatically unless the type is a well-known type or has been
     * declared with DECLARE_NATIVE_CLASS, DECLARE_RECORD_CLASS or DECLARE_STATIC_CLASS.
     */
    template <typename T, typename Enable = void>
    struct ArgType
    {
        static_assert(fail<T>::value, "Unrecognized type detected, ensure that all involved argument types have been declared as a binding type with DECLARE_*_CLASS.");
    };

    template <typename T>
    using arg_type_t = typename ArgType<std::remove_cv_t<std::remove_reference_t<T>>>::type;
}
