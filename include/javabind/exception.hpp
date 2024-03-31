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
#include <stdexcept>

namespace javabind
{
    /**
     * Converts a native exception into a Java exception.
     */
    inline void exception_handler(JNIEnv* env, std::exception& ex)
    {
        // ensure that no unhandled Java exception is waiting to be thrown
        if (!env->ExceptionCheck()) {
            LocalClassRef cls(env, "java/lang/Exception");
            env->ThrowNew(cls.ref(), ex.what());
        }
    }

    inline void throw_exception(JNIEnv* env, const std::string& reason)
    {
        if (env->ExceptionCheck()) {
            env->ExceptionClear();  // thrown by a previously failed Java call
        }
        LocalClassRef exClass(env, "java/lang/Exception");
        env->ThrowNew(exClass.ref(), reason.c_str());
    }
}
