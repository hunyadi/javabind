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
#include "global.hpp"
#include <sstream>

namespace javabind
{
    struct JavaOutputBuffer : std::stringbuf
    {
        JavaOutputBuffer(JNIEnv* env)
            : _env(env)
            , _out(LocalClassRef(env, "java/lang/System").getStaticObjectField("out", "Ljava/io/PrintStream;"))
            , _print(LocalClassRef(env, "java/io/PrintStream").getMethod("print", "(Ljava/lang/String;)V"))
        {}

        virtual int sync()
        {
            _env->CallVoidMethod(_out.ref(), _print.ref(), LocalObjectRef(_env, _env->NewStringUTF(str().data())).ref());
            str("");
            return 0;
        }

    private:
        JNIEnv* _env;
        LocalObjectRef _out;
        Method _print;
    };

    /**
     * Prints to the Java standard output `System.out`.
     */
    struct JavaOutput
    {
        JavaOutput(JNIEnv* env)
            : _buf(env)
            , _str(&_buf)
        {}

        ~JavaOutput()
        {
            _buf.pubsync();
        }

        std::ostream& stream()
        {
            return _str;
        }

    private:
        JavaOutputBuffer _buf;
        std::ostream _str;
    };
}

#define JAVA_OUTPUT ::javabind::JavaOutput(::javabind::this_thread.getEnv()).stream()
