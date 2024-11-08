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
#include "object.hpp"
#include "signature.hpp"
#include <optional>

namespace javabind
{
    template <typename T>
    struct ClassTraits<std::optional<T>>
    {
        constexpr static std::string_view class_name = arg_type_t<T>::class_name;
        constexpr static std::string_view class_path = arg_type_t<T>::class_path;
        constexpr static std::string_view java_name = arg_type_t<T>::java_name;
    };

    /**
     * Converts a C++ optional with an object into a Java List.
     */
    template <typename T>
    struct JavaOptionalType : AssignableJavaType<std::optional<T>>
    {
        using native_type = std::optional<T>;
        using java_type = jobject;

        static native_type native_value(JNIEnv* env, java_type javaOptional)
        {
            if (javaOptional == nullptr) return std::nullopt;
            return arg_type_t<boxed_t<T>>::native_value(env, static_cast<typename arg_type_t<boxed_t<T>>::java_type>(javaOptional));
        }

        static java_type java_value(JNIEnv* env, const native_type& nativeOptional)
        {
            if (!nativeOptional.has_value()) return nullptr;
            return arg_type_t<boxed_t<T>>::java_value(env, nativeOptional.value());
        }
    };

    template <typename T> struct ArgType<std::optional<T>> { using type = JavaOptionalType<T>; };
}