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

#include "core.hpp"
#include "class.hpp"
#include "record.hpp"
#include "function.hpp"
#include "collection.hpp"
#include "optional.hpp"
#include "enum.hpp"

#include "exception.hpp"
#include "message.hpp"
#include "signature.hpp"
#include "traits.hpp"

namespace javabind
{
    /**
     * Declares a class to serve as a data transfer type.
     * Record classes marshal data between native and Java code with copy semantics.
     * The lifecycle of the native and the Java object is not coupled.
     */
    template <typename T>
    struct record_class
    {
        record_class()
        {
            auto result = FieldBindings::value.emplace(arg_type_t<T>::sig, FieldBindings::value_type());
            if (!result.second) {
                throw std::runtime_error(msg() << "Record class '" << arg_type_t<T>::class_name << "' is defined more than once in C++ code");
            }
        }

        record_class(const record_class&) = delete;
        record_class(record_class&&) = delete;

        template <auto member>
        record_class& field(const char* name) {
            static_assert(std::is_member_object_pointer_v<decltype(member)>, "The template argument is expected to be a member variable pointer type.");
            using member_type = typename FieldType<decltype(member)>::type;

            auto&& bindings = FieldBindings::value.at(arg_type_t<T>::sig);
            bindings.push_back({
                name,
                arg_type_t<member_type>::java_name,
                arg_type_t<member_type>::sig,
                [](JNIEnv* env, jobject obj, Field& fld, const void* native_object_ptr) {
                    const T* native_object = reinterpret_cast<const T*>(native_object_ptr);
                    arg_type_t<member_type>::java_set_field_value(env, obj, fld, native_object->*member);
                },
                [](JNIEnv* env, jobject obj, Field& fld, void* native_object_ptr) {
                    T* native_object = reinterpret_cast<T*>(native_object_ptr);
                    native_object->*member = arg_type_t<member_type>::native_field_value(env, obj, fld);
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
        using java_t = typename arg_type_t<T>::java_type;

        using result_type = decltype(func(std::declval<Args>()...));

        static java_t<result_type> invoke(JNIEnv* env, jclass, java_t<std::decay_t<Args>>... args)
        {
            try {
                if constexpr (!std::is_same_v<result_type, void>) {
                    auto&& result = func(arg_type_t<Args>::native_value(env, args)...);
                    return static_cast<java_t<result_type>>(arg_type_t<result_type>::java_value(env, std::move(result)));
                } else {
                    func(arg_type_t<Args>::native_value(env, args)...);
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
        template <typename R>
        using java_t = typename arg_type_t<R>::java_type;

        using result_type = decltype((std::declval<T>().*func)(std::declval<Args>()...));

        static java_t<result_type> invoke(JNIEnv* env, jobject obj, java_t<std::decay_t<Args>>... args)
        {
            try {
                // look up field that stores native pointer
                LocalClassRef cls(env, obj);
                Field field = cls.getField("nativePointer", arg_type_t<T*>::sig);
                T* ptr = arg_type_t<T*>::native_field_value(env, obj, field);

                // invoke native function
                if (!ptr) {
                    throw std::logic_error(msg() << "Object " << ClassTraits<T>::class_name << " has already been disposed of.");
                }
                if constexpr (!std::is_same_v<result_type, void>) {
                    auto&& result = (ptr->*func)(arg_type_t<Args>::native_value(env, args)...);
                    return arg_type_t<result_type>::java_value(env, std::move(result));
                } else {
                    (ptr->*func)(arg_type_t<Args>::native_value(env, args)...);
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
        template <typename R>
        using java_t = typename arg_type_t<R>::java_type;

        static jobject invoke(JNIEnv* env, jclass cls, java_t<Args>... args)
        {
            try {
                // instantiate native object
                T* ptr = new T(arg_type_t<Args>::native_value(env, args)...);

                // instantiate Java object by skipping constructor
                LocalClassRef objClass(env, cls);
                jobject obj = env->AllocObject(objClass.ref());
                if (obj == nullptr) {
                    throw JavaException(env);
                }

                // store native pointer in Java object field
                Field field = objClass.getField("nativePointer", arg_type_t<T*>::sig);
                arg_type_t<T*>::java_set_field_value(env, obj, field, ptr);

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
                Field field = cls.getField("nativePointer", arg_type_t<T*>::sig);
                T* ptr = arg_type_t<T*>::native_field_value(env, obj, field);

                // release native object
                delete ptr;

                // prevent accidental duplicate delete
                arg_type_t<T*>::java_set_field_value(env, obj, field, nullptr);
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
        std::string_view param_display;
        std::string_view return_display;
    };

    struct FunctionBindings {
        using key_type = std::string_view;
        using value_type = std::vector<FunctionBinding>;

        inline static std::map<key_type, value_type> value;
    };

    template <typename T>
    struct static_class
    {
        static_class()
        {
            auto result = FunctionBindings::value.emplace(ClassTraits<T>::class_name, FunctionBindings::value_type());
            if (!result.second) {
                throw std::runtime_error(msg() << "Static class '" << ClassTraits<T>::class_name << "' is defined more than once in C++ code");
            }
        }

        static_class(const static_class&) = delete;
        static_class(static_class&&) = delete;

        template <auto func>
        static_class& function(const std::string_view& name)
        {
            using func_type = decltype(func);

            // check if signature is R(*func)(Args...)
            static_assert(is_unbound_function_pointer<func_type>::value, "The template argument is expected to be an unbound function pointer type.");

            auto&& bindings = FunctionBindings::value.at(ClassTraits<T>::class_name);
            bindings.push_back(
                {
                    name,
                    FunctionTraits<func_type>::sig,
                    false,
                    callable<T, func>(args_t<func_type>{}),
                    FunctionTraits<func_type>::param_display,
                    FunctionTraits<func_type>::return_display
                }
            );
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
            auto result = FunctionBindings::value.emplace(ClassTraits<T>::class_name, FunctionBindings::value_type());
            if (!result.second) {
                throw std::runtime_error(msg() << "Native class '" << ClassTraits<T>::class_name << "' is defined more than once in C++ code");
            }

            auto&& bindings = result.first->second;
            bindings.push_back(
                {
                    "close",
                    FunctionTraits<void()>::sig,
                    true,
                    object_termination<T>(),
                    "",
                    "void"
                }
            );
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

            auto&& bindings = FunctionBindings::value.at(ClassTraits<T>::class_name);
            bindings.push_back(
                {
                    name,
                    FunctionTraits<F>::sig,
                    false,
                    object_initialization<T>(args_t<F>{}),
                    FunctionTraits<F>::param_display,
                    FunctionTraits<F>::return_display
                }
            );
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

            auto&& bindings = FunctionBindings::value.at(ClassTraits<T>::class_name);
            bindings.push_back(
                {
                    name,
                    FunctionTraits<func_type>::sig,
                    is_member,
                    callable<T, func>(args_t<func_type>{}),
                    FunctionTraits<func_type>::param_display,
                    FunctionTraits<func_type>::return_display
                }
            );
            return *this;
        }
    };

    struct EnumBinding
    {
        using value_map_type = std::unordered_map<std::string, JavaEnumValue>;
        using initializer_function = std::function<void(const value_map_type&)>;

        EnumBinding(initializer_function&& initializer)
            : _initializer(std::move(initializer))
        {
        }

        const std::vector<std::string_view>& names() const
        {
            return _names;
        }

        void add(std::string_view name)
        {
            if (!contains(name)) {
                _names.push_back(name);
            }
        }

        bool contains(const std::string_view& name) const
        {
            return std::find(_names.begin(), _names.end(), name) != _names.end();
        }

        void initialize(const value_map_type& values)
        {
            _initializer(values);
        }

    private:
        initializer_function _initializer;
        std::vector<std::string_view> _names;
    };

    struct EnumBindings
    {
        inline static std::map< std::string_view, EnumBinding > value;
    };

    /**
     * Represents an enum class in Java.
     */
    template <typename T>
    struct enum_class
    {
        enum_class()
        {
            auto result = EnumBindings::value.emplace(arg_type_t<T>::class_name, EnumBinding(&EnumValues<T>::initialize));
            if (!result.second) {
                throw std::runtime_error(msg() << "Enum class '" << arg_type_t<T>::class_name << "' is defined more than once in C++ code");
            }
        }

        enum_class<T>& value(T native_value, const std::string_view& java_name)
        {
            EnumBindings::value.at(arg_type_t<T>::class_name).add(java_name);
            EnumValues<T>::bind(native_value, java_name);
            return *this;
        }

        enum_class(const enum_class&) = delete;
        enum_class(enum_class&&) = delete;
    };
}
