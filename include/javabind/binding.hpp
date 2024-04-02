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

#include "class.hpp"
#include "record.hpp"
#include "type.hpp"

#include "exception.hpp"
#include "message.hpp"
#include "signature.hpp"
#include "traits.hpp"

namespace javabind
{
    /**
     * Used in static_assert to have the type name printed in the compiler error message.
     */
    template <typename>
    struct fail : std::false_type {};

    /**
     * Argument type traits, and argument type conversion between native and Java.
     *
     * Template substitution fails automatically unless the type is a well-known type or has been
     * declared with DECLARE_NATIVE_CLASS or DECLARE_STATIC_CLASS.
     */
    template <typename T>
    struct ArgType
    {
        static_assert(fail<T>::value, "Unrecognized type detected, ensure that all involved argument types have been declared as a binding type with DECLARE_*_CLASS.");
    };

    template <> struct ArgType<void> { using type = JavaVoidType; };
    template <> struct ArgType<bool> { using type = JavaBooleanType; };
    template <> struct ArgType<int8_t> { using type = JavaByteType; };
    template <> struct ArgType<int16_t> { using type = JavaShortType; };
    template <> struct ArgType<int32_t> { using type = JavaIntegerType; };
    template <> struct ArgType<int64_t> { using type = JavaLongType; };
    template <> struct ArgType<float> { using type = JavaFloatType; };
    template <> struct ArgType<double> { using type = JavaDoubleType; };
    template <> struct ArgType<std::string> { using type = JavaStringType; };
    template <typename T> struct ArgType<T*> { using type = JavaPointerType<T>; };
    template <typename T> struct ArgType<boxed<T>> { using type = JavaBoxedType<typename ArgType<T>::type>; };

    template <> struct ArgType<std::vector<bool>> { using type = JavaBooleanArrayType; };
    template <typename T> struct ArgType<std::vector<T>> { using type = JavaArrayType<T>; };

    template <typename R, typename T>
    struct ArgType<std::function<R(T)>>
    {
        using type = JavaFunctionType<R, T>;
    };

    template <typename R> struct ArgType<std::function<R(int32_t)>> { using type = JavaIntFunctionType<R>; };
    template <typename R> struct ArgType<std::function<R(int64_t)>> { using type = JavaLongFunctionType<R>; };
    template <typename R> struct ArgType<std::function<R(double)>> { using type = JavaDoubleFunctionType<R>; };
    template <typename T> struct ArgType<std::function<int32_t(T)>> { using type = JavaToIntFunctionType<T>; };
    template <typename T> struct ArgType<std::function<int64_t(T)>> { using type = JavaToLongFunctionType<T>; };
    template <typename T> struct ArgType<std::function<double(T)>> { using type = JavaToDoubleFunctionType<T>; };

    /**
     * Declares a class to serve as a data transfer type.
     * Record classes marshal data between native and Java code with copy semantics.
     * The lifecycle of the native and the Java object is not coupled.
     */
    template <typename T>
    struct record_class
    {
        record_class() = default;
        record_class(const record_class&) = delete;
        record_class(record_class&&) = delete;

        template <auto member>
        record_class& field(const char* name) {
            static_assert(std::is_member_object_pointer_v<decltype(member)>, "The template argument is expected to be a member variable pointer type.");
            using member_type = typename FieldType<decltype(member)>::type;

            auto&& bindings = FieldBindings::value[ArgType<T>::type::sig];
            bindings.push_back({
                name,
                ArgType<member_type>::type::sig,
                [](JNIEnv* env, jobject obj, Field& fld, const void* native_object_ptr) {
                    const T* native_object = reinterpret_cast<const T*>(native_object_ptr);
                    ArgType<member_type>::type::java_set_field_value(env, obj, fld, native_object->*member);
                },
                [](JNIEnv* env, jobject obj, Field& fld, void* native_object_ptr) {
                    T* native_object = reinterpret_cast<T*>(native_object_ptr);
                    native_object->*member = ArgType<member_type>::type::native_field_value(env, obj, fld);
                }
                });
            return *this;
        }
    };

    /**
     * Wraps a native function pointer into a function pointer callable from Java.
     * Adapts a function with the signature R(*func)(Args...).
     * @tparam func The callable function pointer.
     * @return A type-safe function pointer to pass to Java's [RegisterNatives] function.
     */
    template <auto func, typename... Args>
    struct Adapter
    {
        template <typename T>
        using java_t = typename ArgType<T>::type::java_type;

        using result_type = decltype(func(std::declval<Args>()...));

        static java_t<result_type> invoke(JNIEnv* env, jclass, java_t<std::decay_t<Args>>... args)
        {
            try {
                if constexpr (!std::is_same_v<result_type, void>) {
                    auto&& result = func(ArgType<std::decay_t<Args>>::type::native_value(env, args)...);
                    return ArgType<result_type>::type::java_value(env, std::move(result));
                }
                else {
                    func(ArgType<std::decay_t<Args>>::type::native_value(env, args)...);
                }
            } catch (JavaException& ex) {
                env->Throw(ex.innerException());
                if constexpr (!std::is_same_v<result_type, void>) {
                    return java_t<result_type>();
                }
            } catch (std::exception& ex) {
                exception_handler(env, ex);
                if constexpr (!std::is_same_v<result_type, void>) {
                    return java_t<result_type>();
                }
            }
        }
    };

    /**
     * Wraps a native member function pointer into a function pointer callable from Java.
     * Adapts a function with the signature R(T::*func)(Args...).
     * @tparam func The callable member function pointer.
     * @return A type-safe function pointer to pass to Java's [RegisterNatives] function.
     */
    template <typename T, auto func, typename... Args>
    struct MemberAdapter
    {
        template <typename T>
        using java_t = typename ArgType<T>::type::java_type;

        using result_type = decltype((std::declval<T>().*func)(std::declval<Args>()...));

        static java_t<result_type> invoke(JNIEnv* env, jobject obj, java_t<std::decay_t<Args>>... args)
        {
            try {
                // look up field that stores native pointer
                LocalClassRef cls(env, obj);
                Field field = cls.getField("nativePointer", ArgType<T*>::type::sig.data());
                T* ptr = ArgType<T*>::type::native_field_value(env, obj, field);

                // invoke native function
                if (!ptr) {
                    throw std::logic_error(msg() << "Object " << ClassTraits<T>::class_name << " has already been disposed of.");
                }
                if constexpr (!std::is_same_v<result_type, void>) {
                    auto&& result = (ptr->*func)(ArgType<std::decay_t<Args>>::type::native_value(env, args)...);
                    return ArgType<result_type>::type::java_value(env, std::move(result));
                }
                else {
                    (ptr->*func)(ArgType<std::decay_t<Args>>::type::native_value(env, args)...);
                }

            } catch (JavaException& ex) {
                env->Throw(ex.innerException());
                if constexpr (!std::is_same_v<result_type, void>) {
                    return java_t<result_type>();
                }
            } catch (std::exception& ex) {
                exception_handler(env, ex);
                if constexpr (!std::is_same_v<result_type, void>) {
                    return java_t<result_type>();
                }
            }
        }
    };

    /**
     * Wraps a native function pointer or a member function pointer into a function pointer callable from Java.
     * @tparam func The callable function or member function pointer.
     * @return A type-erased function pointer to pass to Java's [RegisterNatives] function.
     */
    template <typename T, auto func, typename... Args>
    constexpr void* callable(types<Args...>)
    {
        auto&& f = std::conditional_t<
            std::is_member_function_pointer_v<decltype(func)>,
            MemberAdapter<T, func, Args...>,
            Adapter<func, Args...>
        >::invoke;
        return reinterpret_cast<void*>(f);
    }

    /**
     * Adapts a constructor function to be invoked from Java on object instantiation with a class method.
     */
    template <typename T, typename... Args>
    struct CreateObjectAdapter
    {
        template <typename T>
        using java_t = typename ArgType<T>::type::java_type;

        static jobject invoke(JNIEnv* env, jclass cls, java_t<Args>... args)
        {
            try {
                // instantiate native object
                T* ptr = new T(ArgType<Args>::type::native_value(env, args)...);

                // instantiate Java object by skipping constructor
                LocalClassRef objClass(env, cls);
                jobject obj = env->AllocObject(objClass.ref());
                if (obj == nullptr) {
                    throw JavaException(env);
                }

                // store native pointer in Java object field
                Field field = objClass.getField("nativePointer", ArgType<T*>::type::sig.data());
                ArgType<T*>::type::java_set_field_value(env, obj, field, ptr);

                return obj;
            } catch (JavaException& ex) {
                env->Throw(ex.innerException());
                return nullptr;
            } catch (std::exception& ex) {
                exception_handler(env, ex);
                return nullptr;
            }
        }
    };

    /**
     * Adapts a destructor function to be invoked from Java when the object is being disposed of.
     * This function is bound to the method close() inherited from the interface AutoCloseable.
     */
    template <typename T>
    struct DestroyObjectAdapter
    {
        static void invoke(JNIEnv* env, jobject obj)
        {
            try {
                // look up field that stores native pointer
                LocalClassRef cls(env, obj);
                Field field = cls.getField("nativePointer", ArgType<T*>::type::sig.data());
                T* ptr = ArgType<T*>::type::native_field_value(env, obj, field);

                // release native object
                delete ptr;

                // prevent accidental duplicate delete
                ArgType<T*>::type::java_set_field_value(env, obj, field, nullptr);
            } catch (JavaException& ex) {
                env->Throw(ex.innerException());
            } catch (std::exception& ex) {
                exception_handler(env, ex);
            }
        }
    };

    template <typename T, typename... Args>
    constexpr void* object_initialization(types<Args...>)
    {
        return reinterpret_cast<void*>(CreateObjectAdapter<T, Args...>::invoke);
    }

    template <typename T>
    constexpr void* object_termination()
    {
        return reinterpret_cast<void*>(DestroyObjectAdapter<T>::invoke);
    }

    struct FunctionBinding {
        std::string_view name;
        std::string_view signature;
        bool is_member;
        void* function_entry_point;
        std::string_view friendly_signature;
    };

    struct FunctionBindings {
        inline static std::map< std::string_view, std::vector<FunctionBinding> > value;
    };

    template <typename T>
    struct static_class
    {
        static_class() = default;
        static_class(const static_class&) = delete;
        static_class(static_class&&) = delete;

        template <auto func>
        static_class& function(const std::string_view& name)
        {
            using func_type = decltype(func);

            // check if signature is R(*func)(Args...)
            static_assert(is_unbound_function_pointer<func_type>::value, "The template argument is expected to be an unbound function pointer type.");

            auto&& bindings = FunctionBindings::value[ClassTraits<T>::class_name];
            bindings.push_back({
                name,
                FunctionTraits<func_type>::sig,
                false,
                callable<T, func>(args_t<func_type>{}),
                FunctionTraits<func_type>::sig
                });
            return *this;
        }
    };

    /**
     * Represents a native class in Java.
     * The Java object holds an opaque pointer to the native object.
     * The lifecycle of the object is governed by Java.
     */
    template <typename T>
    struct native_class
    {
        native_class()
        {
            auto&& bindings = FunctionBindings::value[ClassTraits<T>::class_name];
            bindings.push_back({
                "close",
                FunctionTraits<void()>::sig,
                true,
                object_termination<T>(),
                FunctionTraits<void()>::sig
                });
        }

        native_class(const native_class&) = delete;
        native_class(native_class&&) = delete;

        /**
         * Registers a native object constructor.
         *
         * The object must have a corresponding static function declared in Java:
         * ```
         * public static native Sample create();
         * ```
         * The Java signature is expected to take arguments that are compatible with the native
         * class constructor, and is expected to return an object of the class type.
         *
         * @param name The name of the static function in Java.
         */
        template <typename F>
        native_class& constructor(const std::string_view& name)
        {
            static_assert(std::is_function_v<F>, "Use a function signature such as Sample(int, std::string) to identify a constructor.");

            auto&& bindings = FunctionBindings::value[ClassTraits<T>::class_name];
            bindings.push_back({
                name,
                FunctionTraits<F>::sig,
                false,
                object_initialization<T>(args_t<F>{}),
                FunctionTraits<F>::sig
                });
            return *this;
        }

        /**
         * Registers a native object function.
         *
         * The object must have a corresponding member or static function declared in Java:
         * ```
         * public native void memberFn(int val, String str);
         * public static native void staticFn(int val, String str);
         * ```
         *
         * @param name The name of the member or static function in Java.
         */
        template <auto func>
        native_class& function(const std::string_view& name)
        {
            using func_type = decltype(func);

            // check if signature is R(*func)(Args...)
            constexpr bool is_unbound = is_unbound_function_pointer<func_type>::value;
            // check if signature is R(T::*func)(Args...)
            constexpr bool is_member = std::is_member_function_pointer<func_type>::value;

            static_assert(is_unbound || is_member, "The non-type template argument is expected to be of a free function or a compatible member function pointer type.");

            auto&& bindings = FunctionBindings::value[ClassTraits<T>::class_name];
            bindings.push_back({
                name,
                FunctionTraits<func_type>::sig,
                is_member,
                callable<T, func>(args_t<func_type>{}),
                FunctionTraits<func_type>::sig
                });
            return *this;
        }
    };
}
