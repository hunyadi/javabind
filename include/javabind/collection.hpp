#pragma once
#include "argtype.hpp"
#include "object.hpp"
#include "signature.hpp"
#include <vector>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>

namespace javabind
{
    template <typename T>
    struct ClassTraits<std::vector<T>>
    {
        constexpr static std::string_view class_name = "java.util.List";
        constexpr static std::string_view class_patj = "java/util/List";
        constexpr static std::string_view java_name = GenericTraits<class_name, T>::java_name;
        constexpr static std::string_view concrete_class_path = "java/util/ArrayList";
    };

    template <typename T>
    struct ClassTraits<std::set<T>>
    {
        constexpr static std::string_view class_name = "java.util.Set";
        constexpr static std::string_view class_path = "java/util/Set";
        constexpr static std::string_view java_name = GenericTraits<class_name, T>::java_name;
        constexpr static std::string_view concrete_class_path = "java/util/TreeSet";
    };

    template <typename T>
    struct ClassTraits<std::unordered_set<T>>
    {
        constexpr static std::string_view class_name = "java.util.Set";
        constexpr static std::string_view class_path = "java/util/Set";
        constexpr static std::string_view java_name = GenericTraits<class_name, T>::java_name;
        constexpr static std::string_view concrete_class_path = "java/util/HashSet";
    };

    template <typename K, typename V>
    struct ClassTraits<std::map<K, V>>
    {
        constexpr static std::string_view class_name = "java.util.Map";
        constexpr static std::string_view class_path = "java/util/Map";
        constexpr static std::string_view java_name = GenericTraits<class_name, K, V>::java_name;
        constexpr static std::string_view concrete_class_path = "java/util/TreeMap";
    };

    template <typename K, typename V>
    struct ClassTraits<std::unordered_map<K, V>>
    {
        constexpr static std::string_view class_name = "java.util.Map";
        constexpr static std::string_view class_path = "java/util/Map";
        constexpr static std::string_view java_name = GenericTraits<class_name, K, V>::java_name;
        constexpr static std::string_view concrete_class_path = "java/util/HashMap";
    };

    /**
     * Converts a C++ collection with a forward iterator into a Java List.
     */
    template <typename T>
    struct JavaListType : AssignableJavaType<std::vector<T>>
    {
        constexpr static std::string_view class_name = ClassTraits<std::vector<T>>::class_name;
        constexpr static std::string_view java_name = ClassTraits<std::vector<T>>::java_name;

        using native_type = std::vector<T>;
        using java_type = jobject;

    public:
        static native_type native_value(JNIEnv* env, java_type javaList)
        {
            LocalClassRef listClass(env, javaList);

            Method sizeFunc = listClass.getMethod("size", FunctionTraits<int32_t()>::sig);
            jint len = env->CallIntMethod(javaList, sizeFunc.ref());

            Method getFunc = listClass.getMethod("get", FunctionTraits<object(int32_t)>::sig);

            native_type nativeList;
            for (jint i = 0; i < len; i++) {
                LocalObjectRef listElement(env, env->CallObjectMethod(javaList, getFunc.ref(), i));
                nativeList.push_back(ArgType<T>::type::native_value(env, listElement.ref()));
            }

            return nativeList;
        }

        static java_type java_value(JNIEnv* env, const native_type& nativeList)
        {
            LocalClassRef arrayListClass(env, ClassTraits<native_type>::concrete_class_path);
            Method initFunc = arrayListClass.getMethod("<init>", FunctionTraits<void(int)>::sig);
            jobject arrayList = env->NewObject(arrayListClass.ref(), initFunc.ref(), nativeList.size());
            Method addFunc = arrayListClass.getMethod("add", FunctionTraits<bool(object)>::sig);

            for (auto&& element : nativeList) {
                LocalObjectRef arrayListElement(env, ArgType<T>::type::java_value(env, element));
                env->CallBooleanMethod(arrayList, addFunc.ref(), arrayListElement.ref());
            }
            return arrayList;
        }
    };

    /**
     * Converts a native set (e.g. a [set] or [unordered_set]) into a Java Set.
     */
    template <typename T>
    struct JavaSetType : AssignableJavaType<T>
    {
        using native_type = T;
        using element_type = typename native_type::key_type;
        using java_type = jobject;

        constexpr static std::string_view class_name = ClassTraits<native_type>::class_name;
        constexpr static std::string_view class_path = ClassTraits<native_type>::class_path;
        constexpr static std::string_view java_name = ClassTraits<native_type>::java_name;

        static native_type native_value(JNIEnv* env, java_type javaSet)
        {
            LocalClassRef setClass(env, javaSet);
            Method iteratorFunc = setClass.getMethod("iterator", "()Ljava/util/Iterator;");

            LocalClassRef iteratorClass(env, "java/util/Iterator");
            Method hasNextFunc = iteratorClass.getMethod("hasNext", "()Z");
            Method nextFunc = iteratorClass.getMethod("next", "()Ljava/lang/Object;");

            LocalObjectRef setIterator(env, env->CallObjectMethod(javaSet, iteratorFunc.ref()));

            native_type nativeSet;
            bool hasNext = static_cast<bool>(env->CallBooleanMethod(setIterator.ref(), hasNextFunc.ref()));
            while (hasNext) {
                LocalObjectRef element(env, env->CallObjectMethod(setIterator.ref(), nextFunc.ref()));
                using java_elem_type = typename ArgType<element_type>::type;
                nativeSet.insert(java_elem_type::native_value(env, static_cast<java_elem_type::java_type>(element.ref())));
                hasNext = static_cast<bool>(env->CallBooleanMethod(setIterator.ref(), hasNextFunc.ref()));
            }
            return nativeSet;
        }

        static java_type java_value(JNIEnv* env, const native_type& nativeSet)
        {
            LocalClassRef setClass(env, ClassTraits<native_type>::concrete_class_path);
            Method initFunc = setClass.getMethod("<init>", FunctionTraits<void()>::sig);
            Method addFunc = setClass.getMethod("add", FunctionTraits<bool(object)>::sig);
            jobject javaSet = env->NewObject(setClass.ref(), initFunc.ref());
            if (javaSet == nullptr) {
                throw JavaException(env);
            }

            for (auto&& item : nativeSet) {
                LocalObjectRef element(env, ArgType<element_type>::type::java_value(env, item));
                env->CallBooleanMethod(javaSet, addFunc.ref(), element.ref());
            }
            return javaSet;
        }
    };

    template <typename T>
    struct JavaOrderedSetType : JavaSetType<std::set<T>>
    {};

    template <typename T>
    struct JavaUnorderedSetType : JavaSetType<std::unordered_set<T>>
    {};

    /**
     * Converts a native dictionary (e.g. a [map] or [unordered_map]) into a Java Map.
     */
    template <typename T>
    struct JavaMapType : AssignableJavaType<T>
    {
        using native_type = T;
        using key_type = typename native_type::key_type;
        using value_type = typename native_type::mapped_type;
        using java_type = jobject;

        constexpr static std::string_view class_name = ClassTraits<native_type>::class_name;
        constexpr static std::string_view class_path = ClassTraits<native_type>::class_path;
        constexpr static std::string_view java_name = ClassTraits<native_type>::java_name;

        static native_type native_value(JNIEnv* env, java_type javaMap)
        {
            LocalClassRef mapClass(env, javaMap);
            Method entrySetFunc = mapClass.getMethod("entrySet", "()Ljava/util/Set;");

            LocalClassRef entrySetClass(env, "java/util/Set");
            Method iteratorFunc = entrySetClass.getMethod("iterator", "()Ljava/util/Iterator;");

            LocalClassRef iteratorClass(env, "java/util/Iterator");
            Method hasNextFunc = iteratorClass.getMethod("hasNext", "()Z");
            Method nextFunc = iteratorClass.getMethod("next", "()Ljava/lang/Object;");

            LocalClassRef entryClass(env, "java/util/Map$Entry");
            Method getKeyFunc = entryClass.getMethod("getKey", "()Ljava/lang/Object;");
            Method getValueFunc = entryClass.getMethod("getValue", "()Ljava/lang/Object;");

            LocalObjectRef entrySet(env, env->CallObjectMethod(javaMap, entrySetFunc.ref()));
            LocalObjectRef mapIterator(env, env->CallObjectMethod(entrySet.ref(), iteratorFunc.ref()));

            native_type nativeMap;
            bool hasNext = static_cast<bool>(env->CallBooleanMethod(mapIterator.ref(), hasNextFunc.ref()));
            while (hasNext) {
                LocalObjectRef entry(env, env->CallObjectMethod(mapIterator.ref(), nextFunc.ref()));
                LocalObjectRef javaKey(env, env->CallObjectMethod(entry.ref(), getKeyFunc.ref()));
                LocalObjectRef javaValue(env, env->CallObjectMethod(entry.ref(), getValueFunc.ref()));

                using java_key_type = typename ArgType<key_type>::type;
                using java_value_type = typename ArgType<value_type>::type;

                auto nativeKey = java_key_type::native_value(env, static_cast<typename java_key_type::java_type>(javaKey.ref()));
                auto nativeValue = java_value_type::native_value(env, static_cast<typename java_value_type::java_type>(javaValue.ref()));
                nativeMap[nativeKey] = std::move(nativeValue);

                hasNext = static_cast<bool>(env->CallBooleanMethod(mapIterator.ref(), hasNextFunc.ref()));
            }
            return nativeMap;
        }

        static java_type java_value(JNIEnv* env, const native_type& nativeMap)
        {
            LocalClassRef mapClass(env, ClassTraits<native_type>::concrete_class_path);
            Method initFunc = mapClass.getMethod("<init>", FunctionTraits<void()>::sig);
            Method putFunc = mapClass.getMethod("put", FunctionTraits<object(object, object)>::sig);
            jobject javaMap = env->NewObject(mapClass.ref(), initFunc.ref(), nativeMap.size());
            if (javaMap == nullptr) {
                throw JavaException(env);
            }

            for (auto&& item : nativeMap) {
                LocalObjectRef key(env, ArgType<key_type>::type::java_value(env, item.first));
                LocalObjectRef value(env, ArgType<value_type>::type::java_value(env, item.second));
                env->CallObjectMethod(javaMap, putFunc.ref(), key.ref(), value.ref());
            }
            return javaMap;
        }
    };

    template <typename K, typename V>
    struct JavaOrderedMapType : JavaMapType<std::map<K, V>>
    {};

    template <typename K, typename V>
    struct JavaUnorderedMapType : JavaMapType<std::unordered_map<K, V>>
    {};

    template <typename T> struct ArgType<std::vector<T>, std::enable_if_t<!std::is_arithmetic_v<T>>> { using type = JavaListType<T>; };
    template <typename T> struct ArgType<std::set<T>> { using type = JavaOrderedSetType<T>; };
    template <typename T> struct ArgType<std::unordered_set<T>> { using type = JavaUnorderedSetType<T>; };
    template <typename K, typename V> struct ArgType<std::map<K, V>> { using type = JavaOrderedMapType<K, V>; };
    template <typename K, typename V> struct ArgType<std::unordered_map<K, V>> { using type = JavaUnorderedMapType<K, V>; };
}
