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

namespace javabind
{
    template <typename T>
    struct StaticClassJavaType : ObjectJavaType<T>
    {};

    /**
     * Marshals types that live primarily in the native space, and accessed through an opaque handle in Java.
     */
    template <typename T>
    struct NativeClassJavaType : AssignableJavaType<T>
    {
        static T& native_value(JNIEnv* env, jobject obj)
        {
            // look up field that stores native pointer
            LocalClassRef cls(env, obj);
            Field field = cls.getField("nativePointer", ArgType<T*>::type::sig);
            T* ptr = ArgType<T*>::native_field_value(env, obj, field);
            return *ptr;
        }

        static jobject java_value(JNIEnv* env, T&& native_object)
        {
            // instantiate native object using copy or move constructor
            T* ptr = new T(std::forward<T>(native_object));

            // instantiate Java object by skipping constructor
            LocalClassRef objClass(env, ClassTraits<T>::class_name);
            jobject obj = env->AllocObject(objClass.ref());
            if (obj == nullptr) {
                throw JavaException(env);
            }

            // store native pointer in Java object field
            Field field = objClass.getField("nativePointer", ArgType<T*>::type::sig);
            ArgType<T*>::java_set_field_value(env, obj, field, ptr);

            return obj;
        }
    };
}
