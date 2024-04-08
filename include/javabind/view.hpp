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
#include <string_view>

namespace javabind
{
    /**
     * Represents a UTF-8 string that lives in the Java execution context.
     */
    struct wrapped_string_view
    {
        wrapped_string_view(JNIEnv* env, jstring str)
            : _env(env)
            , _str(str)
        {
            jsize len = env->GetStringUTFLength(str);
            if (len > 0) {
                const char* ptr = env->GetStringUTFChars(str, nullptr);
                _view = std::string_view(ptr, len);
            }
        }

        wrapped_string_view(const wrapped_string_view&) = delete;

        ~wrapped_string_view()
        {
            _env->ReleaseStringUTFChars(_str, _view.data());
        }

        std::string_view view() const
        {
            return _view;
        }

        operator std::string_view() const
        {
            return _view;
        }

    private:
        JNIEnv* _env;
        jstring _str;
        std::string_view _view;
    };

    /**
     * Represents a modified UTF-16 string that lives in the Java execution context.
     */
    struct wrapped_u16string_view
    {
        static_assert(sizeof(jchar) == sizeof(char16_t));

        wrapped_u16string_view(JNIEnv* env, jstring str)
            : _env(env)
            , _str(str)
        {
            jsize len = env->GetStringLength(str);
            if (len > 0) {
                const char16_t* ptr = reinterpret_cast<const char16_t*>(env->GetStringCritical(str, nullptr));
                _view = std::u16string_view(ptr, len);
            }
        }

        wrapped_u16string_view(const wrapped_u16string_view&) = delete;

        ~wrapped_u16string_view()
        {
            _env->ReleaseStringCritical(_str, reinterpret_cast<const jchar*>(_view.data()));
        }

        std::u16string_view view() const
        {
            return _view;
        }

        operator std::u16string_view() const
        {
            return _view;
        }

    private:
        JNIEnv* _env;
        jstring _str;
        std::u16string_view _view;
    };

    /**
     * Represents an array that lives in the Java execution context.
     */
    template <typename T>
    struct wrapped_array_view
    {
        wrapped_array_view(JNIEnv* env, jarray arr)
            : _env(env)
            , _arr(arr)
        {
            jsize len = env->GetArrayLength(arr);
            if (len > 0) {
                const T* ptr = reinterpret_cast<const T*>(env->GetPrimitiveArrayCritical(arr, nullptr));
                _view = std::basic_string_view<T>(ptr, len);
            }
        }

        wrapped_array_view(const wrapped_array_view&) = delete;

        ~wrapped_array_view()
        {
            _env->ReleasePrimitiveArrayCritical(_arr, const_cast<T*>(_view.data()), JNI_ABORT);
        }

        std::basic_string_view<T> view() const
        {
            return _view;
        }

        operator std::basic_string_view<T>() const
        {
            return _view;
        }

    private:
        JNIEnv* _env;
        jarray _arr;
        std::basic_string_view<T> _view;
    };
}
