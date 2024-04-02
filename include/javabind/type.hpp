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
#include <functional>
#include <string_view>
#include <vector>

namespace javabind
{
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
        constexpr static std::string_view primitive_name = "boolean";
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
        constexpr static std::string_view primitive_name = "byte";
        constexpr static std::string_view sig = "B";

        using native_type = int8_t;
        using java_type = jbyte;

        static java_type java_get_field_value(JNIEnv* env, jobject obj, Field& fld)
        {
            return env->GetByteField(obj, fld.ref());
        }

        static void java_set_field_value(JNIEnv* env, jobject obj, Field& fld, int8_t value)
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

    struct JavaShortType : PrimitiveJavaType<JavaShortType, int16_t, jshort>
    {
        constexpr static std::string_view class_name = "java.lang.Short";
        constexpr static std::string_view primitive_name = "short";
        constexpr static std::string_view sig = "S";

        using native_type = int16_t;
        using java_type = jshort;

        static java_type java_get_field_value(JNIEnv* env, jobject obj, Field& fld)
        {
            return env->GetShortField(obj, fld.ref());
        }

        static void java_set_field_value(JNIEnv* env, jobject obj, Field& fld, int16_t value)
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
        constexpr static std::string_view primitive_name = "int";
        constexpr static std::string_view sig = "I";

        using native_type = int32_t;
        using java_type = jint;

        static java_type java_get_field_value(JNIEnv* env, jobject obj, Field& fld)
        {
            return env->GetIntField(obj, fld.ref());
        }

        static void java_set_field_value(JNIEnv* env, jobject obj, Field& fld, int32_t value)
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
        constexpr static std::string_view primitive_name = "long";
        constexpr static std::string_view sig = "J";

        using native_type = int64_t;
        using java_type = jlong;

        static java_type java_get_field_value(JNIEnv* env, jobject obj, Field& fld)
        {
            return env->GetLongField(obj, fld.ref());
        }

        static void java_set_field_value(JNIEnv* env, jobject obj, Field& fld, int value)
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
        constexpr static std::string_view primitive_name = "float";
        constexpr static std::string_view sig = "F";

        using native_type = float;
        using java_type = jfloat;

        static jfloat java_get_field_value(JNIEnv* env, jobject obj, Field& fld)
        {
            return env->GetFloatField(obj, fld.ref());
        }

        static void java_set_field_value(JNIEnv* env, jobject obj, Field& fld, float value)
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
        constexpr static std::string_view primitive_name = "double";
        constexpr static std::string_view sig = "D";

        using native_type = double;
        using java_type = jdouble;

        static jdouble java_get_field_value(JNIEnv* env, jobject obj, Field& fld)
        {
            return env->GetDoubleField(obj, fld.ref());
        }

        static void java_set_field_value(JNIEnv* env, jobject obj, Field& fld, double value)
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
    struct JavaPointerType : PrimitiveJavaType<JavaPointerType<T>, std::intptr_t, jlong>
    {
        constexpr static std::string_view class_name = "java.lang.Long";
        constexpr static std::string_view primitive_name = "long";
        constexpr static std::string_view sig = "J";

        using base_type = PrimitiveJavaType<JavaPointerType<T>, std::intptr_t, jlong>;

        static T* native_field_value(JNIEnv* env, jobject obj, Field& fld)
        {
            return reinterpret_cast<T*>(base_type::native_value(env, env->GetLongField(obj, fld.ref())));
        }

        static void java_set_field_value(JNIEnv* env, jobject obj, Field& fld, T* value)
        {
            env->SetLongField(obj, fld.ref(), base_type::java_value(env, reinterpret_cast<intptr_t>(value)));
        }
    };

    template <typename WrappedType>
    struct JavaBoxedType
    {
        using native_type = typename WrappedType::native_type;
        using java_type = jobject;

        constexpr static std::string_view class_name = WrappedType::class_name;
        constexpr static std::string_view class_path = replace_v<class_name, '.', '/'>;

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
        constexpr static std::string_view get_value_func = join_v<WrappedType::primitive_name, get_value_func_suffix>;
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
            LocalClassRef cls(env, class_path.data());
            StaticMethod valueOf = cls.getStaticMethod("valueOf", value_initializer);
            return env->CallStaticObjectMethod(cls.ref(), valueOf.ref(), java_value);
        }
    };

    /**
     * Converts a C++ string (represented in UTF-8) into a java.lang.String.
     */
    struct JavaStringType
    {
        using native_type = std::string;
        using java_type = jstring;

        constexpr static std::string_view class_name = "java.lang.String";
        constexpr static std::string_view sig = "Ljava/lang/String;";

        static native_type native_value(JNIEnv* env, java_type value)
        {
            jsize len = env->GetStringUTFLength(value);
            std::string s;
            if (len > 0) {
                const char* chars = env->GetStringUTFChars(value, nullptr);
                s.assign(chars, len);
                env->ReleaseStringUTFChars(value, chars);
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
        constexpr static std::string_view sig = join_v<array_type_prefix, ArgType<T>::type::sig>;

        static native_type native_value(JNIEnv* env, jarray arr)
        {
            std::size_t len = env->GetArrayLength(arr);
            native_type vec(len);
            ArgType<T>::type::native_array_value(env, arr, vec.data(), vec.size());
            return vec;
        }

        static jarray java_value(JNIEnv* env, const native_type& vec)
        {
            return ArgType<T>::type::java_array_value(env, vec.data(), vec.size());
        }
    };

    struct JavaBooleanArrayType : JavaArrayBase<JavaBooleanArrayType, bool>
    {
        using native_type = std::vector<bool>;
        using java_type = jarray;

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

    template <typename WrapperType, typename Result, typename Arg>
    struct JavaFunctionBase
    {
        static_assert(!std::is_same_v<Result, void>, "Use a non-void return type.");

        using native_type = std::function<Result(Arg)>;
        using java_type = jobject;
        using result_java_type = typename ArgType<Result>::type::java_type;

        static native_type native_value(JNIEnv* env, java_type obj)
        {
            GlobalObjectRef fun = GlobalObjectRef(env, obj);
            LocalClassRef cls(env, fun.ref());
            Method invoke = cls.getMethod(WrapperType::apply_fn, WrapperType::apply_sig);  // lifecycle bound to object reference
            return native_type(
                [fun = std::move(fun), invoke = std::move(invoke)]
                (const Arg& arg) -> Result
                {
                    // retrieve an environment reference (which may not be the same as when the function object was created)
                    JNIEnv* env = this_thread.getEnv();
                    if (!env) {
                        assert(!"consistency failure");
                        return Result();
                    }

                    auto ret = WrapperType::native_invoke(env, fun.ref(), invoke.ref(), ArgType<Arg>::type::java_value(env, arg));
                    if constexpr (std::is_same_v<decltype(ret), jobject>) {
                        // ensure proper deallocation for jobject
                        LocalObjectRef res = LocalObjectRef(env, ret);
                        if (env->ExceptionCheck()) {
                            throw JavaException(env);
                        }
                        return ArgType<Result>::type::native_value(env, static_cast<result_java_type>(res.ref()));
                    }
                    else {
                        // no special treatment for primitive types
                        if (env->ExceptionCheck()) {
                            throw JavaException(env);
                        }
                        return ArgType<Result>::type::native_value(env, ret);
                    }
                }
            );
        }

        static java_type java_value(JNIEnv*, const native_type&)
        {
            throw std::runtime_error("C++ functions returning a function object to Java are not supported.");
        }
    };

    template <typename Result, typename Arg>
    struct JavaFunctionType : JavaFunctionBase<JavaFunctionType<Result, Arg>, Result, Arg>
    {
        static_assert(!std::is_fundamental_v<Result>, "Result type cannot be a C++ fundamental type for an object-to-object Java function.");
        static_assert(!std::is_fundamental_v<Arg>, "Argument type cannot be a C++ fundamental type for an object-to-object Java function.");

        using native_type = std::function<Result(Arg)>;
        using java_type = typename ArgType<Arg>::type::java_type;

        constexpr static std::string_view class_name = "java.util.function.Function";
        constexpr static std::string_view sig = "Ljava/util/function/Function;";

        constexpr static std::string_view apply_fn = "apply";
        constexpr static std::string_view apply_sig = "(Ljava/lang/Object;)Ljava/lang/Object;";

    public:
        static jobject native_invoke(JNIEnv* env, jobject fn, jmethodID m, java_type val)
        {
            // Java `Function` interface has an `apply` method that takes and returns Object instances;
            return env->CallObjectMethod(fn, m, LocalObjectRef(env, val).ref());
        }
    };

    template <typename Result>
    struct JavaIntFunctionType : JavaFunctionBase<JavaIntFunctionType<Result>, Result, int32_t>
    {
        using native_type = std::function<Result(int32_t)>;

        constexpr static std::string_view class_name = "java.util.function.IntFunction";
        constexpr static std::string_view sig = "Ljava/util/function/IntFunction;";

        constexpr static std::string_view apply_fn = "apply";
        constexpr static std::string_view apply_sig = "(I)Ljava/lang/Object;";

    public:
        static jobject native_invoke(JNIEnv* env, jobject fn, jmethodID m, jint val)
        {
            return env->CallObjectMethod(fn, m, val);
        }
    };

    template <typename Result>
    struct JavaLongFunctionType : JavaFunctionBase<JavaLongFunctionType<Result>, Result, int64_t>
    {
        using native_type = std::function<Result(int64_t)>;

        constexpr static std::string_view class_name = "java.util.function.LongFunction";
        constexpr static std::string_view sig = "Ljava/util/function/LongFunction;";

        constexpr static std::string_view apply_fn = "apply";
        constexpr static std::string_view apply_sig = "(J)Ljava/lang/Object;";

    public:
        static jobject native_invoke(JNIEnv* env, jobject fn, jmethodID m, jlong val)
        {
            return env->CallObjectMethod(fn, m, val);
        }
    };

    template <typename Result>
    struct JavaDoubleFunctionType : JavaFunctionBase<JavaDoubleFunctionType<Result>, Result, double>
    {
        using native_type = std::function<Result(double)>;

        constexpr static std::string_view class_name = "java.util.function.DoubleFunction";
        constexpr static std::string_view sig = "Ljava/util/function/DoubleFunction;";

        constexpr static std::string_view apply_fn = "apply";
        constexpr static std::string_view apply_sig = "(D)Ljava/lang/Object;";

    public:
        static jobject native_invoke(JNIEnv* env, jobject fn, jmethodID m, jdouble val)
        {
            return env->CallObjectMethod(fn, m, val);
        }
    };

    template <typename Arg>
    struct JavaToIntFunctionType : JavaFunctionBase<JavaToIntFunctionType<Arg>, int32_t, Arg>
    {
        using native_type = std::function<int32_t(Arg)>;

        constexpr static std::string_view class_name = "java.util.function.ToIntFunction";
        constexpr static std::string_view sig = "Ljava/util/function/ToIntFunction;";

        constexpr static std::string_view apply_fn = "applyAsInt";
        constexpr static std::string_view apply_sig = "(Ljava/lang/Object;)I";

    public:
        static jint native_invoke(JNIEnv* env, jobject fn, jmethodID m, jobject val)
        {
            return env->CallIntMethod(fn, m, val);
        }
    };

    template <typename Arg>
    struct JavaToLongFunctionType : JavaFunctionBase<JavaToLongFunctionType<Arg>, int64_t, Arg>
    {
        using native_type = std::function<int64_t(Arg)>;

        constexpr static std::string_view class_name = "java.util.function.ToLongFunction";
        constexpr static std::string_view sig = "Ljava/util/function/ToLongFunction;";

        constexpr static std::string_view apply_fn = "applyAsLong";
        constexpr static std::string_view apply_sig = "(Ljava/lang/Object;)J";

    public:
        static jlong native_invoke(JNIEnv* env, jobject fn, jmethodID m, jobject val)
        {
            return env->CallLongMethod(fn, m, val);
        }
    };

    template <typename Arg>
    struct JavaToDoubleFunctionType : JavaFunctionBase<JavaToDoubleFunctionType<Arg>, double, Arg>
    {
        using native_type = std::function<double(Arg)>;

        constexpr static std::string_view class_name = "java.util.function.ToDoubleFunction";
        constexpr static std::string_view sig = "Ljava/util/function/ToDoubleFunction;";

        constexpr static std::string_view apply_fn = "applyAsDouble";
        constexpr static std::string_view apply_sig = "(Ljava/lang/Object;)D";

    public:
        static jdouble native_invoke(JNIEnv* env, jobject fn, jmethodID m, jobject val)
        {
            return env->CallDoubleMethod(fn, m, val);
        }
    };
}
