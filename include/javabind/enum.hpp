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
#include "exception.hpp"
#include "message.hpp"
#include "object.hpp"
#include "signature.hpp"
#include <algorithm>
#include <optional>
#include <type_traits>
#include <unordered_map>

namespace javabind
{
    /**
     * Captures properties of a Java enumeration value.
     *
     * The reference to the Java enumeration value object is allocated as a global reference.
     */
    struct JavaEnumValue
    {
        jobject object;
        jint ordinal;
    };

    /**
     * Maps a C++ enumeration type to a Java enumeration class.
     */
    template <typename native_type>
    struct EnumValues
    {
        static_assert(std::is_enum_v<native_type>, "The template argument is expected to be an enumeration type.");

        inline static std::unordered_map<native_type, std::string_view> bindings;
        inline static std::unordered_map<native_type, jobject> values_to_objects;
        inline static std::unordered_map<jint, native_type> ordinals_to_values;

        static void bind(native_type native_value, std::string_view java_name)
        {
            bindings.emplace(native_value, java_name);
        }

        static void initialize(const std::unordered_map<std::string, JavaEnumValue>& values)
        {
            for (const auto [native_value, java_name] : bindings) {
                const auto& value = values.at(std::string(java_name));
                values_to_objects.emplace(native_value, value.object);
                ordinals_to_values.emplace(value.ordinal, native_value);
            }
        }
    };

    inline jint enum_value_ordinal(LocalClassRef& enum_class, JNIEnv* env, jobject object)
    {
        Method ordinal = enum_class.getMethod("ordinal", FunctionTraits<int()>::sig);
        return env->CallIntMethod(object, ordinal.ref());
    }

    inline std::string enum_value_name(LocalClassRef& enum_class, JNIEnv* env, jobject object)
    {
        Method nameFunc = enum_class.getMethod("name", FunctionTraits<std::string()>::sig);
        jobject nameObject = env->CallObjectMethod(object, nameFunc.ref());
        return arg_type_t<std::string>::native_value(env, static_cast<jstring>(nameObject));
    }

    template <typename T>
    struct EnumClassJavaType : AssignableJavaType<T>
    {
        using native_type = T;
        using java_type = jobject;

        using AssignableJavaType<T>::class_path;
        using AssignableJavaType<T>::class_name;
        using AssignableJavaType<T>::sig;

        static native_type native_value(JNIEnv* env, java_type javaEnumValue)
        {
            if (javaEnumValue == nullptr) {
                throw JavaNullPointerException(env, msg() << "Enum " << class_name << " is null");
            }
            LocalClassRef enum_class(env, javaEnumValue);

            try {
                jint ordinal = enum_value_ordinal(enum_class, env, javaEnumValue);
                return EnumValues<T>::ordinals_to_values.at(ordinal);
            } catch (const std::out_of_range&) {
                throw std::runtime_error(msg() << "Enum " << class_name << " has not bound java value " << enum_value_name(enum_class, env, javaEnumValue));
            }
        }

        static java_type java_value(JNIEnv*, native_type nativeEnumValue)
        {
            try {
                return EnumValues<T>::values_to_objects.at(nativeEnumValue);
            } catch (const std::out_of_range&) {
                throw std::runtime_error(msg() << "Enum " << class_name << " has not bound native value " << static_cast<std::underlying_type_t<native_type>>(nativeEnumValue));
            }
        }
    };
}
