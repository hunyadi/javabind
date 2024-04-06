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
#include "argtype.hpp"
#include "exception.hpp"
#include "message.hpp"

namespace javabind
{
    template <typename R, typename T>
    struct CallbackHandler
    {
        using arg_type = typename ArgType<T>::type::java_type;
        using return_type = typename ArgType<R>::type::java_type;
        using callback_type = NativeCallback<return_type, arg_type>;

        static return_type invoke(JNIEnv* env, jobject obj, arg_type arg)
        {
            try {
                LocalClassRef cls(env, obj);
                Field field = cls.getField("nativePointer", ArgType<callback_type*>::type::sig);
                callback_type* ptr = ArgType<callback_type*>::type::native_field_value(env, obj, field);
                return ptr->invoke(env, arg);
            } catch (JavaException& ex) {
                env->Throw(ex.innerException());
                return return_type();
            } catch (std::exception& ex) {
                exception_handler(env, ex);
                return return_type();
            }
        };

        static void deallocate(JNIEnv* env, jobject obj)
        {
            try {
                LocalClassRef cls(env, obj);
                Field field = cls.getField("nativePointer", ArgType<callback_type*>::type::sig);
                callback_type* ptr = ArgType<callback_type*>::type::native_field_value(env, obj, field);
                delete ptr;
            } catch (JavaException& ex) {
                env->Throw(ex.innerException());
            } catch (std::exception& ex) {
                exception_handler(env, ex);
            }
        }
    };

    template <typename R, typename T>
    static jint register_callback(JNIEnv* env)
    {
        using callback_type = typename ArgType<std::function<R(T)>>::type;

        LocalClassRef cls(env, callback_type::native_class_path, std::nothrow);
        if (cls.ref() == nullptr) {
            javabind::throw_exception(env, msg() << "Cannot find Java class definition for native callback function: " << callback_type::native_class_path);
            return JNI_ERR;
        }
        JNINativeMethod methods[] = {
            {
                const_cast<char*>(callback_type::apply_fn.data()),
                const_cast<char*>(callback_type::apply_sig.data()),
                reinterpret_cast<void*>(CallbackHandler<R, T>::invoke)
            },
            {
                const_cast<char*>("close"),
                const_cast<char*>("()V"),
                reinterpret_cast<void*>(CallbackHandler<R, T>::deallocate)
            }
        };
        jint rc = env->RegisterNatives(cls.ref(), methods, static_cast<jint>(std::size(methods)));
        if (rc != JNI_OK) {
            return rc;
        }
        return JNI_OK;
    }
}
