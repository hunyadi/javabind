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
#include <map>
#include <vector>

namespace javabind
{
    template <typename T>
    struct ArgType;

    /**
     * Meta-information about a native class member variable.
     */
    struct FieldBinding
    {
        /** The field name as it appears in the class definition. */
        std::string_view name;
        /** The Java type signature associated with the field */
        std::string_view signature;
        /** A function that extracts a value from a Java object field. */
        void (*get_by_value)(JNIEnv* env, jobject obj, Field& fld, const void* native_object_ptr);
        /** A function that persists a value to a Java object field. */
        void (*set_by_value)(JNIEnv* env, jobject obj, Field& fld, void* native_object_ptr);
    };

    /**
     * Stores meta-information about the member variables that a native class type has.
     */
    struct FieldBindings
    {
        inline static std::map< std::string_view, std::vector<FieldBinding> > value;
    };

    /**
     * Marshals types that are passed by value between C++ and Java.
     */
    template <typename T>
    struct RecordClassJavaType : ObjectJavaType<T>
    {
        using native_type = T;

        constexpr static std::string_view java_name = ObjectJavaType<T>::java_name;
        constexpr static std::string_view sig = ObjectJavaType<T>::sig;

        static T native_value(JNIEnv* env, jobject obj)
        {
            LocalClassRef objClass(env, obj);

            T native_object = T();
            auto&& bindings = FieldBindings::value[sig];
            for (auto&& binding : bindings) {
                Field fld = objClass.getField(binding.name, binding.signature);
                binding.set_by_value(env, obj, fld, &native_object);
            }
            return native_object;
        }

        static jobject java_value(JNIEnv* env, const T& native_object)
        {
            LocalClassRef objClass(env, sig);
            jobject obj = env->AllocObject(objClass.ref());
            if (obj == nullptr) {
                throw JavaException(env);
            }

            auto&& bindings = FieldBindings::value[sig];
            for (auto&& binding : bindings) {
                Field fld = objClass.getField(binding.name, binding.signature);
                binding.get_by_value(env, obj, fld, &native_object);
            }
            return obj;
        }

        static jarray java_array_value(JNIEnv* env, const native_type* ptr, std::size_t len)
        {
            LocalClassRef elementClass(env, ArgType<T>::class_name);
            jobjectArray arr = env->NewObjectArray(len, elementClass.ref(), nullptr);
            if (arr == nullptr) {
                throw JavaException(env);
            }
            for (std::size_t k = 0; k < len; ++k) {
                LocalObjectRef objElement(env, ArgType<T>::java_value(env, ptr[k]));
                env->SetObjectArrayElement(arr, k, objElement.ref());
            }
            return arr;
        }
    };
}
