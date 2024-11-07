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
#include "string.hpp"
#include "view.hpp"
#include <cstdint>
#include <vector>

namespace javabind
{
    /**
     * Represents a raw Java object.
     */
    struct object
    {};

    template <typename T>
    struct boxed
    {
        static_assert(std::is_arithmetic_v<T>, "Only C++ arithmetic types (Java primitive types) can be boxed.");

        boxed() : value() {}
        boxed(const T& value) : value(value) {}

        operator T()
        {
            return value;
        }

        T value;
    };

    struct JavaVoidType
    {
        constexpr static std::string_view class_name = "java.lang.Void";
        constexpr static std::string_view java_name = "void";
        constexpr static std::string_view sig = "V";

        using native_type = void;
        using java_type = void;
    };

    /**
     * Base class for converting primitive types such as [int] or [double].
     */
    template <typename WrapperType, typename NativeType, typename JavaType>
    struct PrimitiveJavaType
    {
        using native_type = NativeType;
        using java_type = JavaType;

        static_assert(sizeof(native_type) == sizeof(java_type), "C++ and JNI types are expected to match in size.");

        static native_type native_value(JNIEnv*, java_type value)
        {
            return static_cast<native_type>(value);
        }

        static java_type java_value(JNIEnv* /*env*/, const native_type& value)
        {
            return static_cast<java_type>(value);
        }

        /**
         * Extracts a native value from a Java object field.
         */
        static native_type native_field_value(JNIEnv* env, jobject obj, Field& fld)
        {
            return WrapperType::native_value(env, WrapperType::java_get_field_value(env, obj, fld));
        }
    };

    struct JavaBooleanType : PrimitiveJavaType<JavaBooleanType, bool, jboolean>
    {
        constexpr static std::string_view class_name = "java.lang.Boolean";
        constexpr static std::string_view java_name = "boolean";
        constexpr static std::string_view sig = "Z";

        using native_type = bool;
        using java_type = jboolean;

        static java_type java_get_field_value(JNIEnv* env, jobject obj, Field& fld)
        {
            return env->GetBooleanField(obj, fld.ref());
        }

        static void java_set_field_value(JNIEnv* env, jobject obj, Field& fld, bool value)
        {
            env->SetBooleanField(obj, fld.ref(), java_value(env, value));
        }

        static java_type java_call(JNIEnv* env, jobject obj, Method& m)
        {
            return env->CallBooleanMethod(obj, m.ref());
        }

        static void native_array_value(JNIEnv* env, jarray arr, native_type* ptr, std::size_t len)
        {
            env->GetBooleanArrayRegion(static_cast<jbooleanArray>(arr), 0, static_cast<jsize>(len), reinterpret_cast<jboolean*>(ptr));
        }

        static jarray java_array_value(JNIEnv* env, const native_type* ptr, std::size_t len)
        {
            jbooleanArray arr = env->NewBooleanArray(static_cast<jsize>(len));
            if (arr == nullptr) {
                throw JavaException(env);
            }
            env->SetBooleanArrayRegion(arr, 0, static_cast<jsize>(len), reinterpret_cast<const jboolean*>(ptr));
            return arr;
        }
    };

    struct JavaByteType : PrimitiveJavaType<JavaByteType, int8_t, jbyte>
    {
        constexpr static std::string_view class_name = "java.lang.Byte";
        constexpr static std::string_view java_name = "byte";
        constexpr static std::string_view sig = "B";

        using native_type = int8_t;
        using java_type = jbyte;

        static java_type java_get_field_value(JNIEnv* env, jobject obj, Field& fld)
        {
            return env->GetByteField(obj, fld.ref());
        }

        static void java_set_field_value(JNIEnv* env, jobject obj, Field& fld, native_type value)
        {
            env->SetByteField(obj, fld.ref(), java_value(env, value));
        }

        static java_type java_call(JNIEnv* env, jobject obj, Method& m)
        {
            return env->CallByteMethod(obj, m.ref());
        }

        static void native_array_value(JNIEnv* env, jarray arr, native_type* ptr, std::size_t len)
        {
            env->GetByteArrayRegion(static_cast<jbyteArray>(arr), 0, static_cast<jsize>(len), reinterpret_cast<jbyte*>(ptr));
        }

        static jarray java_array_value(JNIEnv* env, const native_type* ptr, std::size_t len)
        {
            jbyteArray arr = env->NewByteArray(static_cast<jsize>(len));
            if (arr == nullptr) {
                throw JavaException(env);
            }
            env->SetByteArrayRegion(arr, 0, static_cast<jsize>(len), reinterpret_cast<const jbyte*>(ptr));
            return arr;
        }
    };

    struct JavaCharacterType : PrimitiveJavaType<JavaCharacterType, char16_t, jchar>
    {
        constexpr static std::string_view class_name = "java.lang.Character";
        constexpr static std::string_view java_name = "char";
        constexpr static std::string_view sig = "C";

        using native_type = char16_t;
        using java_type = jchar;

        static java_type java_get_field_value(JNIEnv* env, jobject obj, Field& fld)
        {
            return env->GetCharField(obj, fld.ref());
        }

        static void java_set_field_value(JNIEnv* env, jobject obj, Field& fld, native_type value)
        {
            env->SetCharField(obj, fld.ref(), java_value(env, value));
        }

        static java_type java_call(JNIEnv* env, jobject obj, Method& m)
        {
            return env->CallCharMethod(obj, m.ref());
        }

        static void native_array_value(JNIEnv* env, jarray arr, native_type* ptr, std::size_t len)
        {
            env->GetCharArrayRegion(static_cast<jcharArray>(arr), 0, static_cast<jsize>(len), reinterpret_cast<jchar*>(ptr));
        }

        static jarray java_array_value(JNIEnv* env, const native_type* ptr, std::size_t len)
        {
            jcharArray arr = env->NewCharArray(static_cast<jsize>(len));
            if (arr == nullptr) {
                throw JavaException(env);
            }
            env->SetCharArrayRegion(arr, 0, static_cast<jsize>(len), reinterpret_cast<const jchar*>(ptr));
            return arr;
        }
    };

    struct JavaShortType : PrimitiveJavaType<JavaShortType, int16_t, jshort>
    {
        constexpr static std::string_view class_name = "java.lang.Short";
        constexpr static std::string_view java_name = "short";
        constexpr static std::string_view sig = "S";

        using native_type = int16_t;
        using java_type = jshort;

        static java_type java_get_field_value(JNIEnv* env, jobject obj, Field& fld)
        {
            return env->GetShortField(obj, fld.ref());
        }

        static void java_set_field_value(JNIEnv* env, jobject obj, Field& fld, native_type value)
        {
            env->SetShortField(obj, fld.ref(), java_value(env, value));
        }

        static java_type java_call(JNIEnv* env, jobject obj, Method& m)
        {
            return env->CallShortMethod(obj, m.ref());
        }

        static void native_array_value(JNIEnv* env, jarray arr, native_type* ptr, std::size_t len)
        {
            env->GetShortArrayRegion(static_cast<jshortArray>(arr), 0, static_cast<jsize>(len), reinterpret_cast<jshort*>(ptr));
        }

        static jarray java_array_value(JNIEnv* env, const native_type* ptr, std::size_t len)
        {
            jshortArray arr = env->NewShortArray(static_cast<jsize>(len));
            if (arr == nullptr) {
                throw JavaException(env);
            }
            env->SetShortArrayRegion(arr, 0, static_cast<jsize>(len), reinterpret_cast<const jshort*>(ptr));
            return arr;
        }
    };

    struct JavaIntegerType : PrimitiveJavaType<JavaIntegerType, int32_t, jint>
    {
        constexpr static std::string_view class_name = "java.lang.Integer";
        constexpr static std::string_view java_name = "int";
        constexpr static std::string_view sig = "I";

        using native_type = int32_t;
        using java_type = jint;

        static java_type java_get_field_value(JNIEnv* env, jobject obj, Field& fld)
        {
            return env->GetIntField(obj, fld.ref());
        }

        static void java_set_field_value(JNIEnv* env, jobject obj, Field& fld, native_type value)
        {
            env->SetIntField(obj, fld.ref(), java_value(env, value));
        }

        static java_type java_call(JNIEnv* env, jobject obj, Method& m)
        {
            return env->CallIntMethod(obj, m.ref());
        }

        static void native_array_value(JNIEnv* env, jarray arr, native_type* ptr, std::size_t len)
        {
            env->GetIntArrayRegion(static_cast<jintArray>(arr), 0, static_cast<jsize>(len), reinterpret_cast<jint*>(ptr));
        }

        static jarray java_array_value(JNIEnv* env, const native_type* ptr, std::size_t len)
        {
            jintArray arr = env->NewIntArray(static_cast<jsize>(len));
            if (arr == nullptr) {
                throw JavaException(env);
            }
            env->SetIntArrayRegion(arr, 0, static_cast<jsize>(len), reinterpret_cast<const jint*>(ptr));
            return arr;
        }
    };

    struct JavaLongType : PrimitiveJavaType<JavaLongType, int64_t, jlong>
    {
        constexpr static std::string_view class_name = "java.lang.Long";
        constexpr static std::string_view java_name = "long";
        constexpr static std::string_view sig = "J";

        using native_type = int64_t;
        using java_type = jlong;

        static java_type java_get_field_value(JNIEnv* env, jobject obj, Field& fld)
        {
            return env->GetLongField(obj, fld.ref());
        }

        static void java_set_field_value(JNIEnv* env, jobject obj, Field& fld, native_type value)
        {
            env->SetLongField(obj, fld.ref(), java_value(env, value));
        }

        static java_type java_call(JNIEnv* env, jobject obj, Method& m)
        {
            return env->CallLongMethod(obj, m.ref());
        }

        static void native_array_value(JNIEnv* env, jarray arr, native_type* ptr, std::size_t len)
        {
            env->GetLongArrayRegion(static_cast<jlongArray>(arr), 0, static_cast<jsize>(len), reinterpret_cast<jlong*>(ptr));
        }

        static jarray java_array_value(JNIEnv* env, const native_type* ptr, std::size_t len)
        {
            jlongArray arr = env->NewLongArray(static_cast<jsize>(len));
            if (arr == nullptr) {
                throw JavaException(env);
            }
            env->SetLongArrayRegion(arr, 0, static_cast<jsize>(len), reinterpret_cast<const jlong*>(ptr));
            return arr;
        }
    };

    struct JavaFloatType : PrimitiveJavaType<JavaFloatType, float, jfloat>
    {
        constexpr static std::string_view class_name = "java.lang.Float";
        constexpr static std::string_view java_name = "float";
        constexpr static std::string_view sig = "F";

        using native_type = float;
        using java_type = jfloat;

        static jfloat java_get_field_value(JNIEnv* env, jobject obj, Field& fld)
        {
            return env->GetFloatField(obj, fld.ref());
        }

        static void java_set_field_value(JNIEnv* env, jobject obj, Field& fld, native_type value)
        {
            env->SetFloatField(obj, fld.ref(), java_value(env, value));
        }

        static jfloat java_call(JNIEnv* env, jobject obj, Method& m)
        {
            return env->CallFloatMethod(obj, m.ref());
        }

        static void native_array_value(JNIEnv* env, jarray arr, native_type* ptr, std::size_t len)
        {
            env->GetFloatArrayRegion(static_cast<jfloatArray>(arr), 0, static_cast<jsize>(len), ptr);
        }

        static jarray java_array_value(JNIEnv* env, const native_type* ptr, std::size_t len)
        {
            jfloatArray arr = env->NewFloatArray(static_cast<jsize>(len));
            if (arr == nullptr) {
                throw JavaException(env);
            }
            env->SetFloatArrayRegion(arr, 0, static_cast<jsize>(len), ptr);
            return arr;
        }
    };

    struct JavaDoubleType : PrimitiveJavaType<JavaDoubleType, double, jdouble>
    {
        constexpr static std::string_view class_name = "java.lang.Double";
        constexpr static std::string_view java_name = "double";
        constexpr static std::string_view sig = "D";

        using native_type = double;
        using java_type = jdouble;

        static jdouble java_get_field_value(JNIEnv* env, jobject obj, Field& fld)
        {
            return env->GetDoubleField(obj, fld.ref());
        }

        static void java_set_field_value(JNIEnv* env, jobject obj, Field& fld, native_type value)
        {
            env->SetDoubleField(obj, fld.ref(), java_value(env, value));
        }

        static jdouble java_call(JNIEnv* env, jobject obj, Method& m)
        {
            return env->CallDoubleMethod(obj, m.ref());
        }

        static void native_array_value(JNIEnv* env, jarray arr, native_type* ptr, std::size_t len)
        {
            env->GetDoubleArrayRegion(static_cast<jdoubleArray>(arr), 0, static_cast<jsize>(len), ptr);
        }

        static jarray java_array_value(JNIEnv* env, const native_type* ptr, std::size_t len)
        {
            jdoubleArray arr = env->NewDoubleArray(static_cast<jsize>(len));
            if (arr == nullptr) {
                throw JavaException(env);
            }
            env->SetDoubleArrayRegion(arr, 0, static_cast<jsize>(len), ptr);
            return arr;
        }
    };

    /**
     * Specialized type for storing native pointers in Java.
     * Reserved for use by the interoperability framework.
     */
    template <typename T>
    struct JavaPointerType
    {
        using native_type = T*;
        using java_type = jlong;

        static_assert(sizeof(void*) == sizeof(jlong), "C++ pointer must be assignable to Java long type.");

        constexpr static std::string_view class_name = "java.lang.Long";
        constexpr static std::string_view java_name = "long";
        constexpr static std::string_view sig = "J";

        static native_type native_value(JNIEnv*, java_type value)
        {
            return reinterpret_cast<native_type>(value);
        }

        static java_type java_value(JNIEnv*, native_type value)
        {
            return reinterpret_cast<java_type>(value);
        }

        static T* native_field_value(JNIEnv* env, jobject obj, Field& fld)
        {
            return native_value(env, env->GetLongField(obj, fld.ref()));
        }

        static void java_set_field_value(JNIEnv* env, jobject obj, Field& fld, native_type value)
        {
            env->SetLongField(obj, fld.ref(), java_value(env, value));
        }
    };

    template <typename WrappedType>
    struct JavaBoxedType
    {
        using native_type = typename WrappedType::native_type;
        using java_type = jobject;

        constexpr static std::string_view class_name = WrappedType::class_name;
        constexpr static std::string_view class_path = replace_v<class_name, '.', '/'>;
        constexpr static std::string_view java_name = WrappedType::class_name;

    private:
        constexpr static std::string_view class_type_prefix = "L";
        constexpr static std::string_view class_type_suffix = ";";

    public:
        constexpr static std::string_view sig = join_v<class_type_prefix, class_path, class_type_suffix>;

    private:
        constexpr static std::string_view lparen = "(";
        constexpr static std::string_view rparen = ")";

        /** Used in looking up the appropriate `valueOf()` function to instantiate object wrappers of values of a primitive type. */
        constexpr static std::string_view value_initializer = join_v<lparen, WrappedType::sig, rparen, sig>;

        /** Used in looking up the appropriate `primitiveValue()` function to fetch the primitive type (e.g. `intValue()` for `int`). */
        constexpr static std::string_view get_value_func_suffix = "Value";
        constexpr static std::string_view get_value_func = join_v<WrappedType::java_name, get_value_func_suffix>;
        constexpr static std::string_view get_value_func_sig = join_v<lparen, rparen, WrappedType::sig>;

    public:
        /**
         * Unwraps a primitive type (e.g. int) from an object type (e.g. Integer).
         */
        static native_type native_value(JNIEnv* env, java_type obj)
        {
            // unbox from object
            LocalClassRef cls(env, obj);
            Method getValue = cls.getMethod(get_value_func.data(), get_value_func_sig);
            auto java_value = WrappedType::java_call(env, obj, getValue);

            // convert primitive type
            return WrappedType::native_value(env, java_value);
        }

        /**
         * Wraps the primitive type (e.g. int) into an object type (e.g. Integer).
         */
        static java_type java_value(JNIEnv* env, const native_type& value)
        {
            // convert primitive type
            auto java_value = WrappedType::java_value(env, value);

            // box into object
            LocalClassRef cls(env, class_path);
            StaticMethod valueOf = cls.getStaticMethod("valueOf", value_initializer);
            return env->CallStaticObjectMethod(cls.ref(), valueOf.ref(), java_value);
        }
    };

    struct JavaObjectType
    {
        using java_type = jobject;

        constexpr static std::string_view class_name = "java.lang.Object";
        constexpr static std::string_view java_name = "Object";
        constexpr static std::string_view sig = "Ljava/lang/Object;";
    };

    /**
     * Converts a C++ string (represented in UTF-8) into a java.lang.String.
     */
    struct JavaStringType
    {
        using native_type = std::string;
        using java_type = jstring;

        constexpr static std::string_view class_name = "java.lang.String";
        constexpr static std::string_view java_name = "String";
        constexpr static std::string_view sig = "Ljava/lang/String;";

        static native_type native_value(JNIEnv* env, java_type value)
        {
            jsize len = env->GetStringUTFLength(value);
            std::string s;
            if (len > 0) {
                s.resize(len);
                env->GetStringUTFRegion(value, 0, len, s.data());
            }
            return s;
        }

        static java_type java_value(JNIEnv* env, const native_type& value)
        {
            return env->NewStringUTF(value.data());
        }

        static native_type native_field_value(JNIEnv* env, jobject obj, Field& fld)
        {
            LocalObjectRef objFieldValue(env, env->GetObjectField(obj, fld.ref()));
            return native_value(env, static_cast<jstring>(objFieldValue.ref()));
        }

        static void java_set_field_value(JNIEnv* env, jobject obj, Field& fld, const native_type& value)
        {
            LocalObjectRef objFieldValue(env, java_value(env, value));
            env->SetObjectField(obj, fld.ref(), objFieldValue.ref());
        }
    };

    struct JavaUTF8StringViewType
    {
        using native_type = std::string_view;
        using java_type = jstring;

        constexpr static std::string_view class_name = "java.lang.String";
        constexpr static std::string_view java_name = "String";
        constexpr static std::string_view sig = "Ljava/lang/String;";

        static wrapped_string_view native_value(JNIEnv* env, java_type value)
        {
            return wrapped_string_view(env, value);
        }

        static java_type java_value(JNIEnv* env, const native_type& value)
        {
            return env->NewStringUTF(value.data());
        }
    };

    struct JavaUTF16StringViewType
    {
        using native_type = std::u16string_view;
        using java_type = jstring;

        constexpr static std::string_view class_name = "java.lang.String";
        constexpr static std::string_view java_name = "String";
        constexpr static std::string_view sig = "Ljava/lang/String;";

        static wrapped_u16string_view native_value(JNIEnv* env, java_type value)
        {
            return wrapped_u16string_view(env, value);
        }

        static java_type java_value(JNIEnv* env, const native_type& value)
        {
            return env->NewString(reinterpret_cast<const jchar*>(value.data()), static_cast<jsize>(value.size()));
        }
    };

    template <typename WrapperType, typename ElementType>
    struct JavaArrayBase
    {
        using native_type = std::vector<ElementType>;
        using java_type = jarray;

        static native_type native_field_value(JNIEnv* env, jobject obj, Field& fld)
        {
            LocalObjectRef objFieldValue(env, env->GetObjectField(obj, fld.ref()));
            return WrapperType::native_value(env, static_cast<jarray>(objFieldValue.ref()));
        }

        static void java_set_field_value(JNIEnv* env, jobject obj, Field& fld, const native_type& value)
        {
            LocalObjectRef objFieldValue(env, WrapperType::java_value(env, value));
            env->SetObjectField(obj, fld.ref(), objFieldValue.ref());
        }
    };

    /**
     * Converts a C++ vector into a Java primitive array type.
     */
    template <typename T>
    struct JavaArrayType : JavaArrayBase<JavaArrayType<T>, T>
    {
        using native_type = std::vector<T>;
        using java_type = jarray;

        constexpr static std::string_view array_type_prefix = "[";
        constexpr static std::string_view array_type_suffix = "]";
        constexpr static std::string_view java_name = join_v<arg_type_t<T>::java_name, array_type_prefix, array_type_suffix>;
        constexpr static std::string_view sig = join_v<array_type_prefix, arg_type_t<T>::sig>;

        static native_type native_value(JNIEnv* env, jarray arr)
        {
            std::size_t len = env->GetArrayLength(arr);
            native_type vec(len);
            arg_type_t<T>::native_array_value(env, arr, vec.data(), vec.size());
            return vec;
        }

        static jarray java_value(JNIEnv* env, const native_type& vec)
        {
            return arg_type_t<T>::java_array_value(env, vec.data(), vec.size());
        }
    };

    struct JavaBooleanArrayType : JavaArrayBase<JavaBooleanArrayType, bool>
    {
        using native_type = std::vector<bool>;
        using java_type = jarray;

        constexpr static std::string_view java_name = "boolean[]";
        constexpr static std::string_view sig = "[Z";

        static native_type native_value(JNIEnv* env, jarray arr)
        {
            std::size_t len = env->GetArrayLength(arr);
            native_type vec;
            vec.reserve(len);

            jboolean* bool_arr = reinterpret_cast<jboolean*>(env->GetPrimitiveArrayCritical(arr, nullptr));

            // check in case the VM tried to make a copy
            if (bool_arr == nullptr) {
                // out of memory exception thrown
                throw JavaException(env);
            }

            // convert elements
            for (jboolean* ptr = bool_arr; len > 0; ++ptr, --len) {
                vec.push_back(*ptr);
            }

            env->ReleasePrimitiveArrayCritical(arr, bool_arr, JNI_ABORT);
            return vec;
        }

        static jarray java_value(JNIEnv* env, const native_type& vec)
        {
            jbooleanArray arr = env->NewBooleanArray(static_cast<jsize>(vec.size()));
            if (arr == nullptr) {
                throw JavaException(env);
            }

            jboolean* bool_array = reinterpret_cast<jboolean*>(env->GetPrimitiveArrayCritical(arr, nullptr));
            if (bool_array == nullptr) {
                throw JavaException(env);
            }

            // convert elements
            jboolean* ptr = bool_array;
            for (auto it = vec.begin(); it != vec.end(); ++it) {
                *ptr++ = *it;
            }

            env->ReleasePrimitiveArrayCritical(arr, bool_array, 0);
            return arr;
        }
    };

    template <typename T>
    struct JavaArrayViewType
    {
        using native_type = std::basic_string_view<T>;
        using java_type = jarray;

        constexpr static std::string_view array_type_prefix = "[";
        constexpr static std::string_view array_type_suffix = "]";
        constexpr static std::string_view java_name = join_v<arg_type_t<T>::java_name, array_type_prefix, array_type_suffix>;
        constexpr static std::string_view sig = join_v<array_type_prefix, arg_type_t<T>::sig>;

        static wrapped_array_view<T> native_value(JNIEnv* env, jarray arr)
        {
            return wrapped_array_view<T>(env, arr);
        }

        static jarray java_value(JNIEnv* env, const native_type& vec)
        {
            return arg_type_t<T>::java_array_value(env, vec.data(), vec.size());
        }
    };

    template <> struct ArgType<void> { using type = JavaVoidType; };
    template <> struct ArgType<bool> { using type = JavaBooleanType; };
    template <> struct ArgType<int8_t> { using type = JavaByteType; };
    template <> struct ArgType<char16_t> { using type = JavaCharacterType; };
    template <> struct ArgType<int16_t> { using type = JavaShortType; };
    template <> struct ArgType<int32_t> { using type = JavaIntegerType; };
    template <> struct ArgType<int64_t> { using type = JavaLongType; };
    template <> struct ArgType<float> { using type = JavaFloatType; };
    template <> struct ArgType<double> { using type = JavaDoubleType; };
    template <> struct ArgType<std::string> { using type = JavaStringType; };
    template <> struct ArgType<std::string_view> { using type = JavaUTF8StringViewType; };
    template <> struct ArgType<std::u16string_view> { using type = JavaUTF16StringViewType; };
    template <typename T> struct ArgType<T*> { using type = JavaPointerType<T>; };
    template <typename T> struct ArgType<boxed<T>> { using type = JavaBoxedType<arg_type_t<T>>; };
    template <> struct ArgType<object> { using type = JavaObjectType; };

    template <> struct ArgType<std::vector<bool>> { using type = JavaBooleanArrayType; };
    template <typename T> struct ArgType<std::vector<T>, std::enable_if_t<std::is_arithmetic_v<T>>> { using type = JavaArrayType<T>; };
    template <typename T> struct ArgType<std::basic_string_view<T>, std::enable_if_t<std::is_arithmetic_v<T>>> { using type = JavaArrayViewType<T>; };
}
