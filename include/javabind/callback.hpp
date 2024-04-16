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
#include "type.hpp"
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
    };

    struct CallbackRegistry
    {
        CallbackRegistry(JNIEnv* env)
            : env(env)
        {
            LocalClassRef cls(env, "hu/info/hunyadi/javabind/NativeCallback", std::nothrow);
            if (cls.ref() == nullptr) {
                javabind::throw_exception(env, "Cannot find Java base class definition for native callback function");
                rc = JNI_ERR;
                return;
            }
            JNINativeMethod methods[] = {
                {
                    const_cast<char*>("deallocate"),
                    const_cast<char*>("(J)V"),
                    reinterpret_cast<void*>(BaseCallback::deallocate)
                }
            };
            rc = env->RegisterNatives(cls.ref(), methods, static_cast<jint>(std::size(methods)));
        }

        template <typename R, typename T>
        CallbackRegistry& add()
        {
            if (rc != JNI_OK) {
                return *this;
            }

            using callback_type = typename ArgType<std::function<R(T)>>::type;

            LocalClassRef cls(env, callback_type::native_class_path, std::nothrow);
            if (cls.ref() == nullptr) {
                javabind::throw_exception(env, msg() << "Cannot find Java class definition for native callback function: " << callback_type::native_class_path);
                rc = JNI_ERR;
                return *this;
            }
            JNINativeMethod methods[] = {
                {
                    const_cast<char*>(callback_type::apply_fn.data()),
                    const_cast<char*>(callback_type::apply_sig.data()),
                    reinterpret_cast<void*>(CallbackHandler<R, T>::invoke)
                }
            };
            rc = env->RegisterNatives(cls.ref(), methods, static_cast<jint>(std::size(methods)));
            return *this;
        }

        jint code() const
        {
            return rc;
        }

    private:
        JNIEnv* env;
        jint rc;
    };
}
