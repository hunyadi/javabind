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
#include <memory>

namespace javabind
{
    /**
     * Represents the JNI environment in which the extension module is executing.
     */
    struct Environment
    {
        /** Triggered by the function `JNI_OnLoad`. */
        static void load(JavaVM* vm)
        {
            assert(_vm == nullptr);
            _vm = vm;
        }

        /** Triggered by the function `JNI_OnUnload`. */
        static void unload(JavaVM* vm)
        {
            assert(_vm == vm);
            (void)vm;
            _vm = nullptr;
        }

        bool hasEnv()
        {
            return _vm != nullptr;
        }

        void setEnv(JNIEnv* env)
        {
            assert(_vm != nullptr);
            assert(_env == nullptr || _env == env);
            _env = env;
        }

        JNIEnv* getEnv()
        {
            assert(_vm != nullptr);

            if (_env == nullptr) {
                // attach thread to obtain an environment
                switch (_vm->GetEnv(reinterpret_cast<void**>(&_env), JNI_VERSION_1_6)) {
                case JNI_OK:
                    break;
                case JNI_EDETACHED:
                    if (_vm->AttachCurrentThread(reinterpret_cast<void**>(&_env), nullptr) == JNI_OK) {
                        assert(_env != nullptr);
                        _attached = true;
                    } else {
                        // failed to attach thread
                        return nullptr;
                    }
                    break;
                case JNI_EVERSION:
                default:
                    // unsupported JVM version or other error
                    return nullptr;
                }
            }

            return _env;
        }

        ~Environment()
        {
            if (!_env) {
                return;
            }

            // only threads explicitly attached by native code should be released
            if (_vm != nullptr && _attached) {
                // detach thread
                _vm->DetachCurrentThread();
            }
        }

    private:
        inline static JavaVM* _vm = nullptr;
        JNIEnv* _env = nullptr;
        bool _attached = false;
    };

    /**
     * Ensures that Java resources allocated by the thread are released when the thread terminates.
     */
    static thread_local Environment this_thread;

    /**
     * An adapter for an object reference handle that remains valid as the native-to-Java boundary is crossed.
     */
    class GlobalObjectRef
    {
    public:
        using jobject_struct = std::pointer_traits<jobject>::element_type;

        GlobalObjectRef(JNIEnv* env, jobject obj)
        {
            _ref = std::shared_ptr<jobject_struct>(
                env->NewGlobalRef(obj),
                [](jobject ref) {
                    JNIEnv* env = this_thread.getEnv();
                    if (env != nullptr) {
                        env->DeleteGlobalRef(ref);
                    }
                }
            );
        }

        jobject ref() const
        {
            return _ref.get();
        }

    private:
        std::shared_ptr<jobject_struct> _ref;
    };
}
