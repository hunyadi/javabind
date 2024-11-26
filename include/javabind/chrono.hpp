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
    template <typename T>
    struct JavaDurationTraits;

    template <>
    struct JavaDurationTraits<std::chrono::nanoseconds>
    {
        constexpr static std::string_view to_native_method = "toNanos";
        constexpr static std::string_view of_native_method = "ofNanos";
        constexpr static jlong factor = 1;
    };

    template <>
    struct JavaDurationTraits<std::chrono::microseconds>
    {
        constexpr static std::string_view to_native_method = "toNanos";
        constexpr static std::string_view of_native_method = "ofNanos";
        constexpr static jlong factor = 1000;
    };

    template <>
    struct JavaDurationTraits<std::chrono::milliseconds>
    {
        constexpr static std::string_view to_native_method = "toMillis";
        constexpr static std::string_view of_native_method = "ofMillis";
        constexpr static jlong factor = 1;
    };

    template <>
    struct JavaDurationTraits<std::chrono::seconds>
    {
        constexpr static std::string_view to_native_method = "getSeconds";
        constexpr static std::string_view of_native_method = "ofSeconds";
        constexpr static jlong factor = 1;
    };

    template <>
    struct JavaDurationTraits<std::chrono::minutes>
    {
        constexpr static std::string_view to_native_method = "toMinutes";
        constexpr static std::string_view of_native_method = "ofMinutes";
        constexpr static jlong factor = 1;
    };

    template <>
    struct JavaDurationTraits<std::chrono::hours>
    {
        constexpr static std::string_view to_native_method = "toHours";
        constexpr static std::string_view of_native_method = "ofHours";
        constexpr static jlong factor = 1;
    };

    template <typename T>
    struct JavaDurationType : public AssignableJavaType<T>
    {
        using native_type = T;
        using java_type = jobject;

        static native_type native_value(JNIEnv* env, java_type javaValue)
        {
            LocalClassRef durationClass(env, javaValue);
            auto toNative = durationClass.getMethod(JavaDurationTraits<native_type>::to_native_method, "()J");
            return native_type { env->CallLongMethod(javaValue, toNative.ref()) / JavaDurationTraits<native_type>::factor };
        }

        static java_type java_value(JNIEnv* env, const native_type& nativeValue)
        {
            LocalClassRef durationClass(env, AssignableJavaType<T>::class_path);
            auto ofNative = durationClass.getStaticMethod(JavaDurationTraits<native_type>::of_native_method, "(J)Ljava/time/Duration;");
            return env->CallStaticObjectMethod(durationClass.ref(), ofNative.ref(), static_cast<jlong>(nativeValue.count()) * JavaDurationTraits<native_type>::factor);
        }
    };

    template <typename T>
    struct JavaInstantType : public AssignableJavaType<T>
    {
        using native_type = T;
        using java_type = jobject;

        static native_type native_value(JNIEnv* env, java_type javaValue)
        {
            LocalClassRef instantClass(env, javaValue);
            auto getEpochSecond = instantClass.getMethod("getEpochSecond", "()J");
            auto getNano = instantClass.getMethod("getNano", "()I");
            auto seconds = env->CallLongMethod(javaValue, getEpochSecond.ref());
            auto nanoseconds = env->CallIntMethod(javaValue, getNano.ref());
            return native_type { std::chrono::seconds { seconds } + std::chrono::nanoseconds { nanoseconds } };
        }

        static java_type java_value(JNIEnv* env, const native_type& nativeValue)
        {
            LocalClassRef instantClass(env, AssignableJavaType<T>::class_path);
            auto ofMillis = instantClass.getStaticMethod("ofEpochSecond", "(JJ)Ljava/time/Instant;");
            auto nativeValueSeconds = std::chrono::time_point_cast<std::chrono::seconds>(nativeValue);
            auto nativeValueNanos = std::chrono::duration_cast<std::chrono::nanoseconds>(nativeValue - nativeValueSeconds);
            auto seconds = nativeValueSeconds.time_since_epoch().count();
            auto nanoseconds = nativeValueNanos.count();
            return env->CallStaticObjectMethod(instantClass.ref(), ofMillis.ref(), static_cast<jlong>(seconds), static_cast<jlong>(nanoseconds));
        }
    };

    struct DurationClassTraits
    {
        constexpr static std::string_view class_name = "java.time.Duration";
    };

    template <> struct ClassTraits<std::chrono::nanoseconds> : public DurationClassTraits {};
    template <> struct ClassTraits<std::chrono::microseconds> : public DurationClassTraits {};
    template <> struct ClassTraits<std::chrono::milliseconds> : public DurationClassTraits {};
    template <> struct ClassTraits<std::chrono::seconds> : public DurationClassTraits {};
    template <> struct ClassTraits<std::chrono::minutes> : public DurationClassTraits {};
    template <> struct ClassTraits<std::chrono::hours> : public DurationClassTraits {};

    template <> struct ArgType<std::chrono::nanoseconds> { using type = JavaDurationType<std::chrono::nanoseconds>; };
    template <> struct ArgType<std::chrono::microseconds> { using type = JavaDurationType<std::chrono::microseconds>; };
    template <> struct ArgType<std::chrono::milliseconds> { using type = JavaDurationType<std::chrono::milliseconds>; };
    template <> struct ArgType<std::chrono::seconds> { using type = JavaDurationType<std::chrono::seconds>; };
    template <> struct ArgType<std::chrono::minutes> { using type = JavaDurationType<std::chrono::minutes>; };
    template <> struct ArgType<std::chrono::hours> { using type = JavaDurationType<std::chrono::hours>; };

    template <>
    struct ClassTraits<std::chrono::system_clock::time_point>
    {
        constexpr static std::string_view class_name = "java.time.Instant";
    };

    template <> struct ArgType<std::chrono::system_clock::time_point> { using type = JavaInstantType<std::chrono::system_clock::time_point>; };
}