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
#include "local.hpp"
#include "string.hpp"

namespace javabind
{
    template <typename T>
    struct ArgType;

    /**
     * Stores information about a native type and Java class binding.
     * Typically extended by DECLARE_NATIVE_CLASS or DECLARE_STATIC_CLASS via template specialization.
     */
    template <typename T>
    struct ClassTraits
    {};

    /**
     * Base class for all object types such as strings, lists, maps, native and record classes.
     */
    template <typename T>
    struct ObjectJavaType
    {
        using native_type = T;
        using java_type = jobject;

    private:
        constexpr static std::string_view class_type_prefix = "L";
        constexpr static std::string_view class_type_suffix = ";";
        constexpr static std::string_view lparen = "(";
        constexpr static std::string_view rparen = ")";

    public:
        constexpr static std::string_view class_name = ClassTraits<T>::class_name;
        constexpr static std::string_view class_path = replace_v<class_name, '.', '/'>;
        constexpr static std::string_view java_name = ClassTraits<T>::class_name;
        constexpr static std::string_view sig = join_v<class_type_prefix, class_path, class_type_suffix>;
    };

    /**
     * Base class for all assignable object types.
     */
    template <typename T>
    struct AssignableJavaType : ObjectJavaType<T>
    {
        using native_type = T;
        using java_type = jobject;

        static native_type native_field_value(JNIEnv* env, jobject obj, Field& fld)
        {
            LocalObjectRef objFieldValue(env, env->GetObjectField(obj, fld.ref()));
            return ArgType<T>::type::native_value(env, objFieldValue.ref());
        }

        static void java_set_field_value(JNIEnv* env, jobject obj, Field& fld, native_type value)
        {
            // use local reference to ensure temporary object is released
            LocalObjectRef objFieldValue(env, ArgType<T>::type::java_value(env, value));
            env->SetObjectField(obj, fld.ref(), objFieldValue.ref());
        }
    };
}
