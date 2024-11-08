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
#include "callback.hpp"
#include "message.hpp"
#include <algorithm>

namespace javabind
{
    /**
     * Prints all registered Java bindings.
     */
    inline void print_registered_bindings() {
        JavaOutput output(this_thread.getEnv());
        std::ostream& os = output.stream();

        // imports
        os << "import hu.info.hunyadi.javabind.NativeObject;\n\n";

        for (auto&& [class_name, bindings] : FunctionBindings::value) {
            std::string simple_class_name;
            std::size_t found = class_name.rfind('/');
            if (found != std::string::npos) {
                simple_class_name = class_name.substr(found + 1);
            }
            else {
                simple_class_name = class_name;
            }

            // class definition
            os << "public class " << simple_class_name << " extends NativeObject {\n";

            // static methods
            for (auto&& binding : bindings) {
                if (!binding.is_member) {
                    os << "    public static native " << binding.return_display << " " << binding.name << "(" << binding.param_display << ");\n";
                }
            }

            // instance methods
            for (auto&& binding : bindings) {
                if (binding.is_member) {
                    os << "    public native " << binding.return_display << " " << binding.name << "(" << binding.param_display << ");\n";
                }
            }

            // end of class definition
            os << "}\n";
        }
    }
}

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
    Environment::load(vm);
    this_thread.setEnv(env);

    try {
        // invoke user-defined function
        initializer();

        // register callback bindings
        rc = CallbackRegistry(env)
            .add<bool, object>()
            .add<bool, int32_t>()
            .add<bool, int64_t>()
            .add<bool, double>()
            .add<object, object>()
            .add<object, int32_t>()
            .add<object, int64_t>()
            .add<object, double>()
            .add<int32_t, object>()
            .add<int64_t, object>()
            .add<double, object>()
            .add<bool, object>()
            .add<bool, int32_t>()
            .add<bool, int64_t>()
            .add<bool, double>()
            .add<void, object>()
            .add<void, int32_t>()
            .add<void, int64_t>()
            .add<void, double>()
            .code();

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

        // check property bindings
        for (auto&& [class_name, bindings] : FieldBindings::value) {
            // find the native class; JNI_OnLoad is called from the correct class loader context for this to work
            LocalClassRef cls(env, class_name, std::nothrow);
            if (cls.ref() == nullptr) {
                javabind::throw_exception(env,
                    msg() << "Cannot find Java record class '" << class_name << "' registered as a plain data class in C++ code"
                );
                return JNI_ERR;
            }

            // try to look up registered fields
            for (auto&& binding : bindings) {
                jfieldID ref = env->GetFieldID(cls.ref(), binding.name.data(), binding.signature.data());
                if (ref != nullptr) {
                    continue;  // everything OK, field exists
                }

                javabind::throw_exception(env,
                    msg() << "Cannot find field '" << binding.name << "' with type signature '" << binding.signature << "' in registered class '" << class_name << "'"
                );
                return JNI_ERR;
            }
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

#define DECLARE_RECORD_CLASS(record_type, java_class_qualifier) \
    template <> struct javabind::ClassTraits<record_type> { \
        constexpr static std::string_view class_name = java_class_qualifier; \
    }; \
    template <> struct javabind::ArgType<record_type> { \
        using type = ::javabind::RecordClassJavaType<record_type>; \
    };

//
// Establishes a mapping between a composite native type and a Java class with
// only static methods exposed to native code.
//
#define DECLARE_STATIC_CLASS(static_type, java_class_qualifier) \
    template <> struct javabind::ClassTraits<static_type> { \
        constexpr static std::string_view class_name = java_class_qualifier; \
    }; \
    template <> struct javabind::ArgType<static_type> { \
        using type = ::javabind::StaticClassJavaType<static_type>; \
    };

//
// Establishes a mapping between a composite native type and a Java class.
// This object lives primarily in the native code space, and exposed to Java as an opaque handle.
//
#define DECLARE_NATIVE_CLASS(native_type, java_class_qualifier) \
    template <> struct javabind::ClassTraits<native_type> { \
        constexpr static std::string_view class_name = java_class_qualifier; \
    }; \
    template <> struct javabind::ArgType<native_type> { \
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
