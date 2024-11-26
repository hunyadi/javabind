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
#include <chrono>

namespace javabind
{
    template<typename T>
    struct JavaDurationTraits;

    template<>
    struct JavaDurationTraits<std::chrono::nanoseconds>
    {
        constexpr static std::string_view to_native_method = "toNanos";
        constexpr static std::string_view of_native_method = "ofNanos";
    };

    template<>
    struct JavaDurationTraits<std::chrono::milliseconds>
    {
        constexpr static std::string_view to_native_method = "toMillis";
        constexpr static std::string_view of_native_method = "ofMillis";
    };

    template<>
    struct JavaDurationTraits<std::chrono::seconds>
    {
        constexpr static std::string_view to_native_method = "getSeconds";
        constexpr static std::string_view of_native_method = "ofSeconds";
    };

    template<>
    struct JavaDurationTraits<std::chrono::minutes>
    {
        constexpr static std::string_view to_native_method = "toMinutes";
        constexpr static std::string_view of_native_method = "ofMinutes";
    };

    template<>
    struct JavaDurationTraits<std::chrono::hours>
    {
        constexpr static std::string_view to_native_method = "toHours";
        constexpr static std::string_view of_native_method = "ofHours";
    };

    template<typename T>
    struct JavaDurationType
    {
        using native_type = T;
        using java_type = jobject;

        constexpr static std::string_view class_name = "java.time.Duration";
        constexpr static std::string_view class_path = "java/time/Duration";
        constexpr static std::string_view java_name = "java.time.Duration";
        constexpr static std::string_view sig = "Ljava/time/Duration;";

        static native_type native_field_value(JNIEnv* env, jobject obj, Field& fld)
        {
            LocalObjectRef objFieldValue(env, env->GetObjectField(obj, fld.ref()));
            return arg_type_t<T>::native_value(env, objFieldValue.ref());
        }

        static void java_set_field_value(JNIEnv* env, jobject obj, Field& fld, native_type value)
        {
            LocalObjectRef objFieldValue(env, arg_type_t<T>::java_value(env, value));
            env->SetObjectField(obj, fld.ref(), objFieldValue.ref());
        }

        static native_type native_value(JNIEnv* env, java_type javaValue)
        {
            LocalClassRef durationClass(env, javaValue);
            auto toNative = durationClass.getMethod(JavaDurationTraits<native_type>::to_native_method, "()J");
            return native_type { env->CallLongMethod(javaValue, toNative.ref()) };
        }

        static java_type java_value(JNIEnv* env, const native_type& nativeValue)
        {
            LocalClassRef durationClass(env, class_path);
            auto ofNative = durationClass.getStaticMethod(JavaDurationTraits<native_type>::of_native_method, "(J)Ljava/time/Duration;");
            return env->CallStaticObjectMethod(durationClass.ref(), ofNative.ref(), static_cast<jlong>(nativeValue.count()));
        }
    };

    template<>
    struct ClassTraits<std::chrono::system_clock::time_point>
    {
        constexpr static std::string_view class_name = "java.time.Instant";
        constexpr static std::string_view class_path = "java/time/Instant";
        constexpr static std::string_view java_name = "java.time.Instant";
    };

    struct JavaInstantType : public AssignableJavaType<std::chrono::system_clock::time_point>
    {
        using native_type = std::chrono::system_clock::time_point;
        using java_type = jobject;

        static native_type native_value(JNIEnv* env, java_type javaValue)
        {
            LocalClassRef instantClass(env, javaValue);
            auto toMillis = instantClass.getMethod("toEpochMilli", "()J");
            auto millis = env->CallLongMethod(javaValue, toMillis.ref());
            return native_type { std::chrono::milliseconds { millis } };
        }

        static java_type java_value(JNIEnv* env, const native_type& nativeValue)
        {
            LocalClassRef instantClass(env, class_path);
            auto ofMillis = instantClass.getStaticMethod("ofEpochMilli", "(J)Ljava/time/Instant;");
            auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(nativeValue.time_since_epoch()).count();
            return env->CallStaticObjectMethod(instantClass.ref(), ofMillis.ref(), static_cast<jlong>(millis));
        }
    };

    template <> struct ArgType<std::chrono::nanoseconds> { using type = JavaDurationType<std::chrono::nanoseconds>; };
    template <> struct ArgType<std::chrono::milliseconds> { using type = JavaDurationType<std::chrono::milliseconds>; };
    template <> struct ArgType<std::chrono::seconds> { using type = JavaDurationType<std::chrono::seconds>; };
    template <> struct ArgType<std::chrono::minutes> { using type = JavaDurationType<std::chrono::minutes>; };
    template <> struct ArgType<std::chrono::hours> { using type = JavaDurationType<std::chrono::hours>; };

    template <> struct ArgType<std::chrono::system_clock::time_point> { using type = JavaInstantType; };
}