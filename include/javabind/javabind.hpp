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
#if !defined(_MSC_VER) && __cplusplus < 201703L
#error Including <javabind.hpp> requires building with -std=c++17 or newer.
#endif

#include "global.hpp"
#include "output.hpp"
#include "binding.hpp"
#include "message.hpp"
#include <algorithm>

 /**
  * Implements the Java [JNI_OnLoad] initialization routine.
  * @param initializer A user-defined function where bindings are registered, e.g. with [native_class].
  */
static jint java_initialization_impl(JavaVM* vm, void (*initializer)())
{
    using namespace javabind;

    JNIEnv* env;
    jint rc = vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6);
    if (rc != JNI_OK) {
        return rc;
    }

    // register Java environment
    javabind::Environment::load(vm);
    javabind::this_thread.setEnv(env);

    try {
        // invoke user-defined function
        initializer();

        // register function bindings
        for (auto&& [class_name, bindings] : FunctionBindings::value) {
            // find the native class; JNI_OnLoad is called from the correct class loader context for this to work
            std::string cn(class_name.data(), class_name.size());
            std::replace(cn.begin(), cn.end(), '.', '/');
            LocalClassRef cls(env, cn.data(), std::nothrow);
            if (cls.ref() == nullptr) {
                javabind::throw_exception(env,
                    msg() << "Cannot find Java class '" << class_name << "' registered as a native class in C++ code"
                );
                return JNI_ERR;
            }

            // register native methods of the class
            std::vector<JNINativeMethod> functions;
            for (auto it = bindings.begin(); it != bindings.end(); ++it) {
                JNINativeMethod m = {
                    const_cast<char*>(it->name.data()),
                    const_cast<char*>(it->signature.data()),
                    it->function_entry_point
                };
                functions.push_back(m);
            }
            rc = env->RegisterNatives(cls.ref(), functions.data(), static_cast<jint>(functions.size()));
            if (rc != JNI_OK) {
                return rc;
            }
        }

        if (env->ExceptionCheck()) {
            return JNI_ERR;
        }
    } catch (std::exception&) {
        // ensure no native exception is propagated to Java
        return JNI_ERR;
    }

    return JNI_VERSION_1_6;
}

/**
 * Implements the Java [JNI_OnUnload] termination routine.
 */
static void java_termination_impl(JavaVM* vm)
{
    javabind::Environment::unload(vm);
}

//
// Establishes a mapping between a composite native type and a Java class with
// only static methods exposed to native code.
//
#define DECLARE_STATIC_CLASS(static_type, java_class_qualifier) \
    template <> struct ::javabind::ClassTraits<static_type> { \
        constexpr static std::string_view class_name = java_class_qualifier; \
    }; \
    template <> struct ::javabind::ArgType<static_type> { \
        using type = ::javabind::StaticClassJavaType<static_type>; \
    };

//
// Establishes a mapping between a composite native type and a Java class.
// This object lives primarily in the native code space, and exposed to Java as an opaque handle.
//
#define DECLARE_NATIVE_CLASS(native_type, java_class_qualifier) \
    template <> struct ::javabind::ClassTraits<native_type> { \
        constexpr static std::string_view class_name = java_class_qualifier; \
    }; \
    template <> struct ::javabind::ArgType<native_type> { \
        using type = ::javabind::NativeClassJavaType<native_type>; \
    };

//
// Registers the library with Java, and binds user-defined native functions to Java instance and class methods.
//
#define JAVA_EXTENSION_MODULE() \
    static void java_bindings_initializer(); \
    JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* /*reserved*/) { return java_initialization_impl(vm, java_bindings_initializer); } \
    JNIEXPORT void JNI_OnUnload(JavaVM *vm, void* /*reserved*/) { java_termination_impl(vm); } \
    void java_bindings_initializer()
