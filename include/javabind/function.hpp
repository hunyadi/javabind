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
#include "signature.hpp"
#include <functional>

namespace javabind
{
    struct BaseCallback
    {
        virtual ~BaseCallback() {}

        static void deallocate(JNIEnv*, jlong ptr)
        {
            delete reinterpret_cast<BaseCallback*>(ptr);
        }
    };

    template<typename java_result_type, typename java_arg_type>
    struct NativeCallback : BaseCallback
    {
        virtual java_result_type invoke(JNIEnv* env, java_arg_type arg) = 0;
        virtual ~NativeCallback() {}
    };

    template<typename R, typename T>
    struct ForwardingCallback : NativeCallback<
        typename arg_type_t<R>::java_type,
        typename arg_type_t<T>::java_type
    >
    {
        ForwardingCallback(std::function<R(T)>&& func)
            : _func(func)
        {}

        using java_arg_type = typename arg_type_t<T>::java_type;
        using java_result_type = typename arg_type_t<R>::java_type;

        java_result_type invoke(JNIEnv* env, java_arg_type arg) override
        {
            if constexpr (!std::is_same_v<R, void>) {
                auto&& result = _func(arg_type_t<T>::native_value(env, arg));
                return arg_type_t<R>::java_value(env, std::move(result));
            }
            else {
                _func(arg_type_t<T>::native_value(env, arg));
            }
        }

    private:
        std::function<R(T)> _func;
    };

    template <typename WrapperType, typename Result, typename Arg>
    struct JavaFunctionBase
    {
        using native_type = std::function<Result(Arg)>;
        using java_type = jobject;
        using java_arg_type = typename arg_type_t<Arg>::java_type;
        using java_result_type = typename arg_type_t<Result>::java_type;

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
                        if constexpr (!std::is_same_v<Result, void>) {
                            return Result();
                        }
                        else {
                            return;
                        }
                    }

                    if constexpr (!std::is_same_v<Result, void>) {
                        auto ret = WrapperType::native_invoke(env, fun.ref(), invoke.ref(), arg_type_t<Arg>::java_value(env, arg));
                        if constexpr (std::is_same_v<decltype(ret), jobject>) {
                            // ensure proper deallocation for jobject
                            LocalObjectRef res = LocalObjectRef(env, ret);
                            if (env->ExceptionCheck()) {
                                throw JavaException(env);
                            }
                            return arg_type_t<Result>::native_value(env, static_cast<java_result_type>(res.ref()));
                        }
                        else {
                            // no special treatment for primitive types
                            if (env->ExceptionCheck()) {
                                throw JavaException(env);
                            }
                            return arg_type_t<Result>::native_value(env, ret);
                        }
                    }
                    else {
                        WrapperType::native_invoke(env, fun.ref(), invoke.ref(), arg_type_t<Arg>::java_value(env, arg));
                        if (env->ExceptionCheck()) {
                            throw JavaException(env);
                        }
                    }
                }
            );
        }

        static java_type java_value(JNIEnv* env, native_type&& fn)
        {
            using callback_type = NativeCallback<java_result_type, java_arg_type>;

            // look up class that wraps native callbacks
            LocalClassRef cls(env, WrapperType::native_class_path);

            // instantiate native callback
            callback_type* ptr = new ForwardingCallback<Result, Arg>(std::move(fn));

            // instantiate Java object via constructor
            Method constructor = cls.getMethod("<init>", "(J)V");
            jobject obj = env->NewObject(cls.ref(), constructor.ref(), ptr);
            if (obj == nullptr) {
                throw JavaException(env);
            }
            return obj;
        }
    };

    template <typename Arg>
    struct JavaPredicateType : JavaFunctionBase<JavaPredicateType<Arg>, bool, Arg>
    {
        using native_type = std::function<bool(Arg)>;

        constexpr static std::string_view class_name = "java.util.function.Predicate";
        constexpr static std::string_view java_name = GenericTraits<class_name, Arg>::java_name;
        constexpr static std::string_view sig = "Ljava/util/function/Predicate;";
        constexpr static std::string_view native_class_path = "hu/info/hunyadi/javabind/NativePredicate";

        constexpr static std::string_view apply_fn = "test";
        constexpr static std::string_view apply_sig = FunctionTraits<bool(object)>::sig;

    public:
        static jboolean native_invoke(JNIEnv* env, jobject fn, jmethodID m, jobject val)
        {
            return env->CallBooleanMethod(fn, m, val);
        }
    };

    struct JavaIntPredicateType : JavaFunctionBase<JavaIntPredicateType, bool, int32_t>
    {
        using native_type = std::function<bool(int32_t)>;

        constexpr static std::string_view class_name = "java.util.function.IntPredicate";
        constexpr static std::string_view java_name = class_name;
        constexpr static std::string_view sig = "Ljava/util/function/IntPredicate;";
        constexpr static std::string_view native_class_path = "hu/info/hunyadi/javabind/NativeIntPredicate";

        constexpr static std::string_view apply_fn = "test";
        constexpr static std::string_view apply_sig = "(I)Z";

    public:
        static jboolean native_invoke(JNIEnv* env, jobject fn, jmethodID m, jlong val)
        {
            return env->CallBooleanMethod(fn, m, val);
        }
    };

    struct JavaLongPredicateType : JavaFunctionBase<JavaLongPredicateType, bool, int64_t>
    {
        using native_type = std::function<bool(int64_t)>;

        constexpr static std::string_view class_name = "java.util.function.LongPredicate";
        constexpr static std::string_view java_name = class_name;
        constexpr static std::string_view sig = "Ljava/util/function/LongPredicate;";
        constexpr static std::string_view native_class_path = "hu/info/hunyadi/javabind/NativeLongPredicate";

        constexpr static std::string_view apply_fn = "test";
        constexpr static std::string_view apply_sig = "(J)Z";

    public:
        static jboolean native_invoke(JNIEnv* env, jobject fn, jmethodID m, jlong val)
        {
            return env->CallBooleanMethod(fn, m, val);
        }
    };

    struct JavaDoublePredicateType : JavaFunctionBase<JavaDoublePredicateType, bool, double>
    {
        using native_type = std::function<bool(double)>;

        constexpr static std::string_view class_name = "java.util.function.DoublePredicate";
        constexpr static std::string_view java_name = class_name;
        constexpr static std::string_view sig = "Ljava/util/function/DoublePredicate;";
        constexpr static std::string_view native_class_path = "hu/info/hunyadi/javabind/NativeDoublePredicate";

        constexpr static std::string_view apply_fn = "test";
        constexpr static std::string_view apply_sig = "(D)Z";

    public:
        static jboolean native_invoke(JNIEnv* env, jobject fn, jmethodID m, jdouble val)
        {
            return env->CallBooleanMethod(fn, m, val);
        }
    };

    template <typename Result, typename Arg>
    struct JavaFunctionType : JavaFunctionBase<JavaFunctionType<Result, Arg>, Result, Arg>
    {
        static_assert(!std::is_fundamental_v<Result>, "Result type cannot be a C++ fundamental type for an object-to-object Java function.");
        static_assert(!std::is_fundamental_v<Arg>, "Argument type cannot be a C++ fundamental type for an object-to-object Java function.");

        using native_type = std::function<Result(Arg)>;
        using java_type = typename arg_type_t<Arg>::java_type;

        constexpr static std::string_view class_name = "java.util.function.Function";
        constexpr static std::string_view java_name = GenericTraits<class_name, Arg, Result>::java_name;
        constexpr static std::string_view sig = "Ljava/util/function/Function;";
        constexpr static std::string_view native_class_path = "hu/info/hunyadi/javabind/NativeFunction";

        constexpr static std::string_view apply_fn = "apply";
        constexpr static std::string_view apply_sig = FunctionTraits<object(object)>::sig;

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
        constexpr static std::string_view java_name = GenericTraits<class_name, Result>::java_name;
        constexpr static std::string_view sig = "Ljava/util/function/IntFunction;";
        constexpr static std::string_view native_class_path = "hu/info/hunyadi/javabind/NativeIntFunction";

        constexpr static std::string_view apply_fn = "apply";
        constexpr static std::string_view apply_sig = FunctionTraits<object(int32_t)>::sig;

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
        constexpr static std::string_view java_name = GenericTraits<class_name, Result>::java_name;
        constexpr static std::string_view sig = "Ljava/util/function/LongFunction;";
        constexpr static std::string_view native_class_path = "hu/info/hunyadi/javabind/NativeLongFunction";

        constexpr static std::string_view apply_fn = "apply";
        constexpr static std::string_view apply_sig = FunctionTraits<object(int64_t)>::sig;

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
        constexpr static std::string_view java_name = GenericTraits<class_name, Result>::java_name;
        constexpr static std::string_view sig = "Ljava/util/function/DoubleFunction;";
        constexpr static std::string_view native_class_path = "hu/info/hunyadi/javabind/NativeDoubleFunction";

        constexpr static std::string_view apply_fn = "apply";
        constexpr static std::string_view apply_sig = FunctionTraits<object(double)>::sig;

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
        constexpr static std::string_view java_name = GenericTraits<class_name, Arg>::java_name;
        constexpr static std::string_view sig = "Ljava/util/function/ToIntFunction;";
        constexpr static std::string_view native_class_path = "hu/info/hunyadi/javabind/NativeToIntFunction";

        constexpr static std::string_view apply_fn = "applyAsInt";
        constexpr static std::string_view apply_sig = FunctionTraits<int32_t(object)>::sig;

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
        constexpr static std::string_view java_name = GenericTraits<class_name, Arg>::java_name;
        constexpr static std::string_view sig = "Ljava/util/function/ToLongFunction;";
        constexpr static std::string_view native_class_path = "hu/info/hunyadi/javabind/NativeToLongFunction";

        constexpr static std::string_view apply_fn = "applyAsLong";
        constexpr static std::string_view apply_sig = FunctionTraits<int64_t(object)>::sig;

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
        constexpr static std::string_view java_name = GenericTraits<class_name, Arg>::java_name;
        constexpr static std::string_view sig = "Ljava/util/function/ToDoubleFunction;";
        constexpr static std::string_view native_class_path = "hu/info/hunyadi/javabind/NativeToDoubleFunction";

        constexpr static std::string_view apply_fn = "applyAsDouble";
        constexpr static std::string_view apply_sig = FunctionTraits<double(object)>::sig;

    public:
        static jdouble native_invoke(JNIEnv* env, jobject fn, jmethodID m, jobject val)
        {
            return env->CallDoubleMethod(fn, m, val);
        }
    };

    template <typename Arg>
    struct JavaConsumerType : JavaFunctionBase<JavaConsumerType<Arg>, void, Arg>
    {
        using native_type = std::function<void(Arg)>;

        constexpr static std::string_view class_name = "java.util.function.Consumer";
        constexpr static std::string_view java_name = GenericTraits<class_name, std::decay_t<Arg>>::java_name;
        constexpr static std::string_view sig = "Ljava/util/function/Consumer;";
        constexpr static std::string_view native_class_path = "hu/info/hunyadi/javabind/NativeConsumer";

        constexpr static std::string_view apply_fn = "accept";
        constexpr static std::string_view apply_sig = FunctionTraits<void(object)>::sig;

    public:
        static void native_invoke(JNIEnv* env, jobject fn, jmethodID m, jobject val)
        {
            return env->CallVoidMethod(fn, m, val);
        }
    };

    struct JavaIntConsumerType : JavaFunctionBase<JavaIntConsumerType, void, int32_t>
    {
        using native_type = std::function<void(int32_t)>;

        constexpr static std::string_view class_name = "java.util.function.IntConsumer";
        constexpr static std::string_view java_name = class_name;
        constexpr static std::string_view sig = "Ljava/util/function/IntConsumer;";
        constexpr static std::string_view native_class_path = "hu/info/hunyadi/javabind/NativeIntConsumer";

        constexpr static std::string_view apply_fn = "accept";
        constexpr static std::string_view apply_sig = "(I)V";

    public:
        static void native_invoke(JNIEnv* env, jobject fn, jmethodID m, jint val)
        {
            return env->CallVoidMethod(fn, m, val);
        }
    };

    struct JavaLongConsumerType : JavaFunctionBase<JavaLongConsumerType, void, int64_t>
    {
        using native_type = std::function<void(int64_t)>;

        constexpr static std::string_view class_name = "java.util.function.LongConsumer";
        constexpr static std::string_view java_name = class_name;
        constexpr static std::string_view sig = "Ljava/util/function/LongConsumer;";
        constexpr static std::string_view native_class_path = "hu/info/hunyadi/javabind/NativeLongConsumer";

        constexpr static std::string_view apply_fn = "accept";
        constexpr static std::string_view apply_sig = "(J)V";

    public:
        static void native_invoke(JNIEnv* env, jobject fn, jmethodID m, jlong val)
        {
            return env->CallVoidMethod(fn, m, val);
        }
    };

    struct JavaDoubleConsumerType : JavaFunctionBase<JavaDoubleConsumerType, void, double>
    {
        using native_type = std::function<void(double)>;

        constexpr static std::string_view class_name = "java.util.function.DoubleConsumer";
        constexpr static std::string_view java_name = class_name;
        constexpr static std::string_view sig = "Ljava/util/function/DoubleConsumer;";
        constexpr static std::string_view native_class_path = "hu/info/hunyadi/javabind/NativeDoubleConsumer";

        constexpr static std::string_view apply_fn = "accept";
        constexpr static std::string_view apply_sig = "(D)V";

    public:
        static void native_invoke(JNIEnv* env, jobject fn, jmethodID m, jdouble val)
        {
            return env->CallVoidMethod(fn, m, val);
        }
    };

    template <typename R, typename T>
    struct ArgType<std::function<R(T)>>
    {
        using type = JavaFunctionType<R, T>;
    };

    template <typename T> struct ArgType<std::function<bool(T)>> { using type = JavaPredicateType<T>; };
    template <> struct ArgType<std::function<bool(int32_t)>> { using type = JavaIntPredicateType; };
    template <> struct ArgType<std::function<bool(int64_t)>> { using type = JavaLongPredicateType; };
    template <> struct ArgType<std::function<bool(double)>> { using type = JavaDoublePredicateType; };

    template <typename R> struct ArgType<std::function<R(int32_t)>> { using type = JavaIntFunctionType<R>; };
    template <typename R> struct ArgType<std::function<R(int64_t)>> { using type = JavaLongFunctionType<R>; };
    template <typename R> struct ArgType<std::function<R(double)>> { using type = JavaDoubleFunctionType<R>; };
    template <typename T> struct ArgType<std::function<int32_t(T)>> { using type = JavaToIntFunctionType<T>; };
    template <typename T> struct ArgType<std::function<int64_t(T)>> { using type = JavaToLongFunctionType<T>; };
    template <typename T> struct ArgType<std::function<double(T)>> { using type = JavaToDoubleFunctionType<T>; };

    template <typename T> struct ArgType<std::function<void(T)>> { using type = JavaConsumerType<T>; };
    template <> struct ArgType<std::function<void(int32_t)>> { using type = JavaIntConsumerType; };
    template <> struct ArgType<std::function<void(int64_t)>> { using type = JavaLongConsumerType; };
    template <> struct ArgType<std::function<void(double)>> { using type = JavaDoubleConsumerType; };
}
