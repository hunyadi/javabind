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

namespace javabind
{
    template <typename native_type>
    struct EnumValues
    {
        inline static std::map<native_type, std::string_view> values_to_names;
        inline static std::map<std::string_view, native_type> names_to_values;

        static native_type native_value(std::string_view java_name)
        {
            return names_to_values.at(java_name);
        }

        static std::string_view java_name(native_type native_value)
        {
            return values_to_names.at(native_value);
        }

        static bool contains(native_type native_value)
        {
            return values_to_names.find(native_value) != values_to_names.end();
        }

        static bool contains(std::string_view java_name)
        {
            return names_to_values.find(java_name) != names_to_values.end();
        }

        static void bind(native_type native_value, std::string_view java_name)
        {
            values_to_names[native_value] = java_name;
            names_to_values[java_name] = native_value;
        }
    };

    template <typename T>
    struct EnumTraits;

    template <typename T>
    struct JavaEnumType : AssignableJavaType<T>
    {
        using native_type = T;
        using java_type = jobject;

        constexpr static std::string_view class_name = EnumTraits<T>::class_name;
        constexpr static std::string_view class_path = replace_v<class_name, '.', '/'>;
        constexpr static std::string_view java_name = class_name;

        constexpr static std::string_view class_type_prefix = "L";
        constexpr static std::string_view class_type_suffix = ";";
        constexpr static std::string_view sig = join_v<class_type_prefix, class_path, class_type_suffix>;

        constexpr static std::string_view values_sig_prefix = "()[";
        constexpr static std::string_view values_sig = join_v<values_sig_prefix, sig>;

        static native_type native_value(JNIEnv* env, java_type javaEnumValue)
        {
            LocalClassRef enumClass(env, javaEnumValue);
            Method nameFunc = enumClass.getMethod("name", FunctionTraits<std::string()>::sig);
            jobject nameObject = env->CallObjectMethod(javaEnumValue, nameFunc.ref());
            std::string nameString = arg_type_t<std::string>::native_value(env, static_cast<jstring>(nameObject));
            return native_value(env, nameString);
        }

        static java_type java_value(JNIEnv* env, native_type nativeEnumValue)
        {
            LocalClassRef enumClass(env, class_path);
            StaticField valueField = enumClass.getStaticField(java_value_name(env, nativeEnumValue), sig);
            return env->GetStaticObjectField(enumClass.ref(), valueField.ref());
        }

        static native_type native_value(JNIEnv* env, std::string_view java_name)
        {
            if (!EnumValues<native_type>::contains(java_name))
                javabind::throw_exception(env, msg() << "Enum " << class_name << " has not bound java value " << java_name);

            return EnumValues<native_type>::native_value(java_name);
        }

        static std::string_view java_value_name(JNIEnv* env, native_type nativeEnumValue)
        {
            if (!EnumValues<native_type>::contains(nativeEnumValue))
                javabind::throw_exception(env, msg() << "Enum " << class_name << " has not bound native value " << static_cast<std::underlying_type_t<native_type>>(nativeEnumValue));

            return EnumValues<native_type>::java_name(nativeEnumValue);
        }
    };

    template<typename T>
    struct ArgType<T, std::enable_if_t<std::is_enum_v<T>>>
    {
        using type = JavaEnumType<T>;
    };
}