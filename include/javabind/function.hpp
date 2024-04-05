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
#include "global.hpp"
#include <functional>

namespace javabind
{
    template <typename T>
    struct ArgType;

    template<typename java_result_type, typename java_arg_type>
    struct NativeCallback
    {
        virtual java_result_type invoke(JNIEnv* env, java_arg_type arg) = 0;
        virtual ~NativeCallback() {}
    };

    template<typename R, typename T>
    struct ForwardingCallback : NativeCallback<
        typename ArgType<R>::type::java_type,
        typename ArgType<T>::type::java_type
    >
    {
        ForwardingCallback(std::function<R(T)>&& func)
            : _func(func)
        {}

        using java_arg_type = typename ArgType<T>::type::java_type;
        using java_result_type = typename ArgType<R>::type::java_type;

        java_result_type invoke(JNIEnv* env, java_arg_type arg) override
        {
            auto&& result = _func(ArgType<T>::type::native_value(env, arg));
            return ArgType<R>::type::java_value(env, std::move(result));
        }

    private:
        std::function<R(T)> _func;
    };

    template <typename WrapperType, typename Result, typename Arg>
    struct JavaFunctionBase
    {
        static_assert(!std::is_same_v<Result, void>, "Use a non-void return type.");

        using native_type = std::function<Result(Arg)>;
        using java_type = jobject;
        using java_arg_type = typename ArgType<Arg>::type::java_type;
        using java_result_type = typename ArgType<Result>::type::java_type;

        static native_type native_value(JNIEnv* env, java_type obj)
        {
            GlobalObjectRef fun = GlobalObjectRef(env, obj);
            LocalClassRef cls(env, fun.ref());
            Method invoke = cls.getMethod(WrapperType::apply_fn, WrapperType::apply_sig);  // lifecycle bound to object reference
            return native_type(
                [fun = std::move(fun), invoke = std::move(invoke)]
                (const Arg& arg) -> Result
                {
                    // retrieve an environment reference (which may not be the same as when the function object was created)
                    JNIEnv* env = this_thread.getEnv();
                    if (!env) {
                        assert(!"consistency failure");
                        return Result();
                    }

                    auto ret = WrapperType::native_invoke(env, fun.ref(), invoke.ref(), ArgType<Arg>::type::java_value(env, arg));
                    if constexpr (std::is_same_v<decltype(ret), jobject>) {
                        // ensure proper deallocation for jobject
                        LocalObjectRef res = LocalObjectRef(env, ret);
                        if (env->ExceptionCheck()) {
                            throw JavaException(env);
                        }
                        return ArgType<Result>::type::native_value(env, static_cast<java_result_type>(res.ref()));
                    }
                    else {
                        // no special treatment for primitive types
                        if (env->ExceptionCheck()) {
                            throw JavaException(env);
                        }
                        return ArgType<Result>::type::native_value(env, ret);
                    }
                }
            );
        }

        static java_type java_value(JNIEnv* env, native_type&& fn)
        {
            using callback_type = NativeCallback<java_result_type, java_arg_type>;

            // look up class that wraps native callbacks
            LocalClassRef cls(env, WrapperType::fn_class_path);

            // instantiate Java object by skipping constructor
            jobject obj = env->AllocObject(cls.ref());
            if (obj == nullptr) {
                throw JavaException(env);
            }

            // instantiate native callback
            callback_type* ptr = new ForwardingCallback<Result, Arg>(std::move(fn));

            // store native pointer in Java object field
            Field field = cls.getField("nativePointer", ArgType<callback_type*>::type::sig);
            ArgType<callback_type*>::type::java_set_field_value(env, obj, field, ptr);

            return obj;
        }
    };

    template <typename Result, typename Arg>
    struct JavaFunctionType : JavaFunctionBase<JavaFunctionType<Result, Arg>, Result, Arg>
    {
        static_assert(!std::is_fundamental_v<Result>, "Result type cannot be a C++ fundamental type for an object-to-object Java function.");
        static_assert(!std::is_fundamental_v<Arg>, "Argument type cannot be a C++ fundamental type for an object-to-object Java function.");

        using native_type = std::function<Result(Arg)>;
        using java_type = typename ArgType<Arg>::type::java_type;

        constexpr static std::string_view class_name = "java.util.function.Function";
        constexpr static std::string_view sig = "Ljava/util/function/Function;";
        constexpr static std::string_view fn_class_path = "hu/info/hunyadi/javabind/NativeFunction";

        constexpr static std::string_view apply_fn = "apply";
        constexpr static std::string_view apply_sig = "(Ljava/lang/Object;)Ljava/lang/Object;";

    public:
        static jobject native_invoke(JNIEnv* env, jobject fn, jmethodID m, java_type val)
        {
            // Java `Function` interface has an `apply` method that takes and returns Object instances;
            return env->CallObjectMethod(fn, m, LocalObjectRef(env, val).ref());
        }
    };

    template <typename Result>
    struct JavaIntFunctionType : JavaFunctionBase<JavaIntFunctionType<Result>, Result, int32_t>
    {
        using native_type = std::function<Result(int32_t)>;

        constexpr static std::string_view class_name = "java.util.function.IntFunction";
        constexpr static std::string_view sig = "Ljava/util/function/IntFunction;";
        constexpr static std::string_view fn_class_path = "hu/info/hunyadi/javabind/NativeIntFunction";

        constexpr static std::string_view apply_fn = "apply";
        constexpr static std::string_view apply_sig = "(I)Ljava/lang/Object;";

    public:
        static jobject native_invoke(JNIEnv* env, jobject fn, jmethodID m, jint val)
        {
            return env->CallObjectMethod(fn, m, val);
        }
    };

    template <typename Result>
    struct JavaLongFunctionType : JavaFunctionBase<JavaLongFunctionType<Result>, Result, int64_t>
    {
        using native_type = std::function<Result(int64_t)>;

        constexpr static std::string_view class_name = "java.util.function.LongFunction";
        constexpr static std::string_view sig = "Ljava/util/function/LongFunction;";
        constexpr static std::string_view fn_class_path = "hu/info/hunyadi/javabind/NativeLongFunction";

        constexpr static std::string_view apply_fn = "apply";
        constexpr static std::string_view apply_sig = "(J)Ljava/lang/Object;";

    public:
        static jobject native_invoke(JNIEnv* env, jobject fn, jmethodID m, jlong val)
        {
            return env->CallObjectMethod(fn, m, val);
        }
    };

    template <typename Result>
    struct JavaDoubleFunctionType : JavaFunctionBase<JavaDoubleFunctionType<Result>, Result, double>
    {
        using native_type = std::function<Result(double)>;

        constexpr static std::string_view class_name = "java.util.function.DoubleFunction";
        constexpr static std::string_view sig = "Ljava/util/function/DoubleFunction;";
        constexpr static std::string_view fn_class_path = "hu/info/hunyadi/javabind/NativeDoubleFunction";

        constexpr static std::string_view apply_fn = "apply";
        constexpr static std::string_view apply_sig = "(D)Ljava/lang/Object;";

    public:
        static jobject native_invoke(JNIEnv* env, jobject fn, jmethodID m, jdouble val)
        {
            return env->CallObjectMethod(fn, m, val);
        }
    };

    template <typename Arg>
    struct JavaToIntFunctionType : JavaFunctionBase<JavaToIntFunctionType<Arg>, int32_t, Arg>
    {
        using native_type = std::function<int32_t(Arg)>;

        constexpr static std::string_view class_name = "java.util.function.ToIntFunction";
        constexpr static std::string_view sig = "Ljava/util/function/ToIntFunction;";
        constexpr static std::string_view fn_class_path = "hu/info/hunyadi/javabind/NativeToIntFunction";

        constexpr static std::string_view apply_fn = "applyAsInt";
        constexpr static std::string_view apply_sig = "(Ljava/lang/Object;)I";

    public:
        static jint native_invoke(JNIEnv* env, jobject fn, jmethodID m, jobject val)
        {
            return env->CallIntMethod(fn, m, val);
        }
    };

    template <typename Arg>
    struct JavaToLongFunctionType : JavaFunctionBase<JavaToLongFunctionType<Arg>, int64_t, Arg>
    {
        using native_type = std::function<int64_t(Arg)>;

        constexpr static std::string_view class_name = "java.util.function.ToLongFunction";
        constexpr static std::string_view sig = "Ljava/util/function/ToLongFunction;";
        constexpr static std::string_view fn_class_path = "hu/info/hunyadi/javabind/NativeToLongFunction";

        constexpr static std::string_view apply_fn = "applyAsLong";
        constexpr static std::string_view apply_sig = "(Ljava/lang/Object;)J";

    public:
        static jlong native_invoke(JNIEnv* env, jobject fn, jmethodID m, jobject val)
        {
            return env->CallLongMethod(fn, m, val);
        }
    };

    template <typename Arg>
    struct JavaToDoubleFunctionType : JavaFunctionBase<JavaToDoubleFunctionType<Arg>, double, Arg>
    {
        using native_type = std::function<double(Arg)>;

        constexpr static std::string_view class_name = "java.util.function.ToDoubleFunction";
        constexpr static std::string_view sig = "Ljava/util/function/ToDoubleFunction;";
        constexpr static std::string_view fn_class_path = "hu/info/hunyadi/javabind/NativeToDoubleFunction";

        constexpr static std::string_view apply_fn = "applyAsDouble";
        constexpr static std::string_view apply_sig = "(Ljava/lang/Object;)D";

    public:
        static jdouble native_invoke(JNIEnv* env, jobject fn, jmethodID m, jobject val)
        {
            return env->CallDoubleMethod(fn, m, val);
        }
    };
}
