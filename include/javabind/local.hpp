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
#include <jni.h>
#include <cassert>
#include <stdexcept>

namespace javabind
{
    /**
     * An exception that originates from Java.
     */
    struct JavaException : std::exception
    {
        JavaException(JNIEnv* env)
        {
            if (env->ExceptionCheck()) {
                ex = env->ExceptionOccurred();

                // clear the exception to allow calling JNI functions
                env->ExceptionClear();

                // extract exception message using low-level functions
                jclass exceptionClass = env->GetObjectClass(ex);
                jmethodID getMessageFunc = env->GetMethodID(exceptionClass, "getMessage", "()Ljava/lang/String;");
                jstring messageObject = static_cast<jstring>(env->CallObjectMethod(ex, getMessageFunc));
                const char* c_str = env->GetStringUTFChars(messageObject, nullptr);
                message = c_str;
                env->ReleaseStringUTFChars(messageObject, c_str);
                env->DeleteLocalRef(messageObject);
                env->DeleteLocalRef(exceptionClass);
            }
        }

        /**
         * Exceptions must not be copied as they contain a JNI local reference.
         */
        JavaException(const JavaException&) = delete;

        const char* what() const noexcept
        {
            return message.c_str();
        }

        /**
         * Used by the interoperability framework to re-throw the exception in Java before crossing the native to Java
         * boundary, unless the exception has been caught by the user.
         */
        jthrowable innerException() const noexcept
        {
            return ex;
        }

    private:
        jthrowable ex = nullptr;
        std::string message;
    };

    class LocalClassRef;

    /**
     * C++ wrapper class of [jmethodID] for instance methods.
     */
    class Method
    {
        Method(JNIEnv* env, jclass cls, const char* name, const std::string_view& signature)
        {
            _ref = env->GetMethodID(cls, name, signature.data());
            if (_ref == nullptr) {
                throw JavaException(env);  // method not found
            }
        }

        friend LocalClassRef;

        jmethodID _ref = nullptr;

    public:
        Method() = default;

        jmethodID ref() const
        {
            return _ref;
        }
    };

    /**
     * C++ wrapper class of [jmethodID] for class methods.
     */
    class StaticMethod
    {
        StaticMethod(JNIEnv* env, jclass cls, const char* name, const std::string_view& signature)
        {
            _ref = env->GetStaticMethodID(cls, name, signature.data());
            if (_ref == nullptr) {
                throw JavaException(env);  // method not found
            }
        }

        StaticMethod(const StaticMethod&) = delete;

        friend LocalClassRef;

        jmethodID _ref = nullptr;

    public:
        StaticMethod() = default;

        jmethodID ref() const
        {
            return _ref;
        }
    };

    /**
     * C++ wrapper class of [jfieldID] for instance fields.
     */
    class Field
    {
        Field(JNIEnv* env, jclass cls, const char* name, const std::string_view& signature)
        {
            _ref = env->GetFieldID(cls, name, signature.data());
            if (_ref == nullptr) {
                throw JavaException(env);  // field not found
            }
        }

        Field(const Field&) = delete;

        friend LocalClassRef;

        jfieldID _ref = nullptr;

    public:
        Field() = default;

        jfieldID ref() const
        {
            return _ref;
        }
    };

    /**
     * C++ wrapper class of [jfieldID] for class fields.
     */
    class StaticField
    {
        StaticField(JNIEnv* env, jclass cls, const char* name, const std::string_view& signature)
        {
            _ref = env->GetStaticFieldID(cls, name, signature.data());
            if (_ref == nullptr) {
                throw JavaException(env);  // field not found
            }
        }

        StaticField(const StaticField&) = delete;

        friend LocalClassRef;

        jfieldID _ref = nullptr;

    public:
        StaticField() = default;

        jfieldID ref() const
        {
            return _ref;
        }
    };

    /**
     * Scoped C++ wrapper class of a [jobject] that is used only within a single native execution block.
     */
    class LocalObjectRef
    {
    public:
        LocalObjectRef() = default;
        LocalObjectRef(const LocalObjectRef& op) = delete;
        LocalObjectRef& operator=(const LocalObjectRef& op) = delete;

        LocalObjectRef(JNIEnv* env, jobject obj)
            : _env(env), _ref(obj)
        {}

        LocalObjectRef(LocalObjectRef&& op)
            : _env(op._env)
            , _ref(op._ref)
        {
            op._ref = nullptr;
        }

        ~LocalObjectRef()
        {
            if (_ref != nullptr) {
                _env->DeleteLocalRef(_ref);
            }
        }

        jobject ref() const
        {
            return _ref;
        }

    private:
        JNIEnv* _env = nullptr;
        jobject _ref = nullptr;
    };

    /**
     * Scoped C++ wrapper class of [jclass].
     */
    class LocalClassRef
    {
    public:
        LocalClassRef(JNIEnv* env, const char* name)
            : LocalClassRef(env, name, std::nothrow)
        {
            if (_ref == nullptr) {
                throw JavaException(env);  // Java class not found
            }
        }

        LocalClassRef(JNIEnv* env, const char* name, std::nothrow_t)
            : _env(env)
        {
            _ref = env->FindClass(name);
        }

        LocalClassRef(JNIEnv* env, jobject obj)
            : _env(env)
        {
            _ref = env->GetObjectClass(obj);
        }

        LocalClassRef(JNIEnv* env, jclass cls)
            : _env(env)
            , _ref(cls)
        {}

        ~LocalClassRef()
        {
            if (_ref != nullptr) {
                _env->DeleteLocalRef(_ref);
            }
        }

        LocalClassRef(const LocalClassRef&) = delete;
        LocalClassRef& operator=(const LocalClassRef& op) = delete;

        LocalClassRef(LocalClassRef&& op) : _env(op._env), _ref(op._ref)
        {
            op._ref = nullptr;
        }

        Method getMethod(const char* name, const std::string_view& signature)
        {
            return Method(_env, _ref, name, signature);
        }

        Field getField(const char* name, const std::string_view& signature)
        {
            return Field(_env, _ref, name, signature);
        }

        StaticMethod getStaticMethod(const char* name, const std::string_view& signature)
        {
            return StaticMethod(_env, _ref, name, signature);
        }

        StaticField getStaticField(const char* name, const std::string_view& signature)
        {
            return StaticField(_env, _ref, name, signature);
        }

        LocalObjectRef getStaticObjectField(const char* name, const std::string_view& signature)
        {
            StaticField fld = getStaticField(name, signature);
            return LocalObjectRef(_env, _env->GetStaticObjectField(_ref, fld.ref()));
        }

        jclass ref() const
        {
            return _ref;
        }

    private:
        JNIEnv* _env;
        jclass _ref;
    };
}
