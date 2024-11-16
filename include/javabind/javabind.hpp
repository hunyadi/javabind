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
#include <iostream>

namespace javabind
{
    /**
     * Prints all registered Java bindings.
     */
    inline void print_registered_bindings(std::ostream& os) {
        // imports
        os << "import hu.info.hunyadi.javabind.NativeObject;\n\n";

        for (auto&& [enum_name, bindings] : EnumBindings::value) {
            std::string_view simple_enum_name = strip_until_last(enum_name, '/');

            // enum definition
            os << "public enum " << simple_enum_name << " {\n";

            // enum values
            for (std::size_t i = 0; i < bindings.names().size(); i++) {
                if (i != bindings.names().size() - 1) {
                    os << "    " << bindings.names()[i] << ",\n";
                } else {
                    os << "    " << bindings.names()[i] << "\n";
                }
            }

            // end of class definition
            os << "}\n";
        }

        for (auto&& [class_name, bindings] : FunctionBindings::value) {
            std::string_view simple_class_name = strip_until_last(class_name, '/');

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

    inline void print_registered_bindings() {
        JavaOutput output(this_thread.getEnv());
        print_registered_bindings(output.stream());
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

        // initialize enum bindings
        for (auto&& [class_name, bindings] : EnumBindings::value) {
            // find the enum class; JNI_OnLoad is called from the correct class loader context for this to work
            std::string cn(class_name.data(), class_name.size());
            std::replace(cn.begin(), cn.end(), '.', '/');
            LocalClassRef cls(env, cn, std::nothrow);
            if (cls.ref() == nullptr) {
                javabind::throw_exception(env,
                    msg() << "Cannot find Java class '" << class_name << "' registered as an enum class in C++ code"
                );
                return JNI_ERR;
            }

            std::string values_sig = std::string("()[L").append(cn).append(";");
            jmethodID values_ref = env->GetStaticMethodID(cls.ref(), "values", values_sig.data());
            if (values_ref == nullptr) {
                javabind::throw_exception(env,
                    msg() << "Cannot find static method 'values' with signature '" << values_sig << "' in registered enum class '" << class_name
                );
                return JNI_ERR;
            }

            jmethodID ordinal_ref = env->GetMethodID(cls.ref(), "ordinal", "()I");
            if (ordinal_ref == nullptr) {
                javabind::throw_exception(env,
                    msg() << "Cannot find method 'ordinal' with signature '()I' in enum value of class '" << class_name << "'"
                );
                return JNI_ERR;
            }

            jmethodID name_ref = env->GetMethodID(cls.ref(), "name", "()Ljava/lang/String;");
            if (name_ref == nullptr) {
                javabind::throw_exception(env,
                    msg() << "Cannot find method 'name' with signature '()Ljava/lang/String;' in enum value of class '" << class_name << "'"
                );
                return JNI_ERR;
            }

            jarray values_arr = static_cast<jarray>(env->CallStaticObjectMethod(cls.ref(), values_ref));
            if (values_arr == nullptr) {
                javabind::throw_exception(env,
                    msg() << "Static method 'values' with signature '" << values_sig << "' in registered enum class '" << class_name << "' returned null"
                );
                return JNI_ERR;
            }

            std::unordered_map<std::string, JavaEnumValue> values;

            for (jsize i = 0; i < env->GetArrayLength(values_arr); ++i) {
                jobject value = env->GetObjectArrayElement(static_cast<jobjectArray>(values_arr), i);
                if (value == nullptr) {
                    javabind::throw_exception(env,
                        msg() << "Element " << i << " of static method 'values' in enum class '" << class_name << "' returned null"
                    );
                    return JNI_ERR;
                }

                std::string name = arg_type_t<std::string>::native_value(env, static_cast<jstring>(env->CallObjectMethod(value, name_ref)));

                if (!bindings.contains(name)) {
                    javabind::throw_exception(env,
                        msg() << "Enum value '" << name << "' in class '" << class_name << "' is not registered in C++ code"
                    );
                    return JNI_ERR;
                }

                jint ordinal = env->CallIntMethod(value, ordinal_ref);
                values.emplace(name, JavaEnumValue{ env->NewGlobalRef(value), ordinal });
            }

            bindings.initialize(values);
        }
    } catch (const std::exception& ex) {
        // ensure no native exception is propagated to Java
        javabind::throw_exception(env, ex.what());
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
// Establishes a mapping between a native enum and a Java enum class.
//
#define DECLARE_ENUM_CLASS(native_type, java_class_qualifier) \
    template <> struct javabind::ClassTraits<native_type> { \
        constexpr static std::string_view class_name = java_class_qualifier; \
    }; \
    template <> struct javabind::ArgType<native_type> { \
        using type = ::javabind::EnumClassJavaType<native_type>; \
    };

//
// Registers the library with Java, and binds user-defined native functions to Java instance and class methods.
//
#define JAVA_EXTENSION_MODULE() \
    static void java_bindings_initializer(); \
    JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* /*reserved*/) { return java_initialization_impl(vm, java_bindings_initializer); } \
    JNIEXPORT void JNI_OnUnload(JavaVM *vm, void* /*reserved*/) { java_termination_impl(vm); } \
    void java_bindings_initializer()
