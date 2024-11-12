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
#include "object.hpp"
#include "signature.hpp"
#include <vector>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>

namespace javabind
{
    template<typename T>
    using boxed_t = std::conditional_t<std::is_arithmetic_v<T>, boxed<T>, T>;

    /**
     * Provides an opaque view to a Java list.
     *
     * Calls to this C++ wrapper object translate into JNI calls, lazily unpacking elements.
     */
    template <typename T>
    struct list_view
    {
        list_view(JNIEnv* env, jobject javaList)
            : env(env)
            , javaList(javaList)
        {
            LocalClassRef listClass(env, javaList);
            sizeFunc = listClass.getMethod("size", FunctionTraits<int32_t()>::sig);
            getFunc = listClass.getMethod("get", FunctionTraits<object(int32_t)>::sig);
        }

        std::size_t size() const
        {
            return env->CallIntMethod(javaList, sizeFunc.ref());
        }

        T get(std::size_t i) const
        {
            LocalObjectRef listElement(env, env->CallObjectMethod(javaList, getFunc.ref(), static_cast<jint>(i)));
            return arg_type_t<T>::native_value(env, listElement.ref());
        }

    private:
        JNIEnv* env;
        jobject javaList;
        Method sizeFunc;
        Method getFunc;
    };

    template <typename T>
    struct set_view_iterator
    {
        set_view_iterator(JNIEnv* env, LocalObjectRef&& setIterator)
            : env(env)
            , setIterator(std::move(setIterator))
        {
            LocalClassRef iteratorClass(env, "java/util/Iterator");
            hasNextFunc = iteratorClass.getMethod("hasNext", FunctionTraits<bool()>::sig);
            nextFunc = iteratorClass.getMethod("next", FunctionTraits<object()>::sig);
        }

        bool has_next() const
        {
            return static_cast<bool>(env->CallBooleanMethod(setIterator.ref(), hasNextFunc.ref()));
        }

        T get_next() const
        {
            LocalObjectRef element(env, env->CallObjectMethod(setIterator.ref(), nextFunc.ref()));
            using java_elem_type = arg_type_t<T>;
            return java_elem_type::native_value(env, static_cast<typename java_elem_type::java_type>(element.ref()));
        }

    private:
        JNIEnv* env;
        LocalObjectRef setIterator;
        Method hasNextFunc;
        Method nextFunc;
    };

    /**
     * Provides an opaque view to a Java set.
     *
     * Calls to this C++ wrapper object translate into JNI calls, lazily unpacking elements.
     */
    template <typename T>
    struct set_view
    {
        set_view(JNIEnv* env, jobject javaSet)
            : env(env)
            , javaSet(javaSet)
        {}

        set_view_iterator<T> iterator() const
        {
            LocalClassRef setClass(env, javaSet);
            Method iteratorFunc = setClass.getMethod("iterator", "()Ljava/util/Iterator;");
            return set_view_iterator<T>(env, LocalObjectRef(env, env->CallObjectMethod(javaSet, iteratorFunc.ref())));
        }

    private:
        JNIEnv* env;
        jobject javaSet;
    };

    template <typename K, typename V>
    struct map_entry
    {
        map_entry(K&& key, V&& value)
            : key(std::forward<K>(key))
            , value(std::forward<V>(value))
        {}

        K key;
        V value;
    };

    template <typename K, typename V>
    struct map_view_iterator
    {
        map_view_iterator(JNIEnv* env, LocalObjectRef&& mapIterator)
            : env(env)
            , mapIterator(std::move(mapIterator))
        {
            LocalClassRef iteratorClass(env, "java/util/Iterator");
            hasNextFunc = iteratorClass.getMethod("hasNext", FunctionTraits<bool()>::sig);
            nextFunc = iteratorClass.getMethod("next", FunctionTraits<object()>::sig);

            LocalClassRef entryClass(env, "java/util/Map$Entry");
            getKeyFunc = entryClass.getMethod("getKey", FunctionTraits<object()>::sig);
            getValueFunc = entryClass.getMethod("getValue", FunctionTraits<object()>::sig);
        }

        bool has_next() const
        {
            return static_cast<bool>(env->CallBooleanMethod(mapIterator.ref(), hasNextFunc.ref()));
        }

        map_entry<K, V> get_next() const
        {
            LocalObjectRef entry(env, env->CallObjectMethod(mapIterator.ref(), nextFunc.ref()));
            LocalObjectRef javaKey(env, env->CallObjectMethod(entry.ref(), getKeyFunc.ref()));
            LocalObjectRef javaValue(env, env->CallObjectMethod(entry.ref(), getValueFunc.ref()));

            using java_key_type = arg_type_t<K>;
            using java_value_type = arg_type_t<V>;

            auto nativeKey = java_key_type::native_value(env, static_cast<typename java_key_type::java_type>(javaKey.ref()));
            auto nativeValue = java_value_type::native_value(env, static_cast<typename java_value_type::java_type>(javaValue.ref()));

            return map_entry<K, V>(std::move(nativeKey), std::move(nativeValue));
        }

    private:
        JNIEnv* env;
        LocalObjectRef mapIterator;
        Method hasNextFunc;
        Method nextFunc;
        Method getKeyFunc;
        Method getValueFunc;
    };

    /**
     * Provides an opaque view to a Java map.
     *
     * Calls to this C++ wrapper object translate into JNI calls, lazily unpacking elements.
     */
    template <typename K, typename V>
    struct map_view
    {
        map_view(JNIEnv* env, jobject javaMap)
            : env(env)
            , javaMap(javaMap)
        {}

        map_view_iterator<K, V> iterator() const
        {
            LocalClassRef mapClass(env, javaMap);
            Method entrySetFunc = mapClass.getMethod("entrySet", "()Ljava/util/Set;");

            LocalClassRef entrySetClass(env, "java/util/Set");
            Method iteratorFunc = entrySetClass.getMethod("iterator", "()Ljava/util/Iterator;");

            LocalObjectRef entrySet(env, env->CallObjectMethod(javaMap, entrySetFunc.ref()));
            LocalObjectRef mapIterator(env, env->CallObjectMethod(entrySet.ref(), iteratorFunc.ref()));

            return map_view_iterator<K, V>(env, std::move(mapIterator));
        }

    private:
        JNIEnv* env;
        jobject javaMap;
    };

    template <typename T>
    struct ClassTraits<std::vector<T>>
    {
        constexpr static std::string_view class_name = "java.util.List";
        constexpr static std::string_view class_path = "java/util/List";
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
            list_view<T> view(env, javaList);
            native_type nativeList;
            std::size_t size = view.size();
            for (std::size_t i = 0; i < size; i++) {
                nativeList.push_back(view.get(i));
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
                LocalObjectRef arrayListElement(env, arg_type_t<T>::java_value(env, element));
                env->CallBooleanMethod(arrayList, addFunc.ref(), arrayListElement.ref());
            }
            return arrayList;
        }
    };

    /**
     * Converts a native set (e.g. a [set] or [unordered_set]) into a Java Set.
     */
    template <template<typename...> typename S, typename T>
    struct JavaSetType : AssignableJavaType<S<T>>
    {
        using native_type = S<T>;
        using native_boxed_type = S<boxed_t<T>>;
        using element_type = typename native_boxed_type::key_type;
        using java_type = jobject;

        constexpr static std::string_view class_name = ClassTraits<native_boxed_type>::class_name;
        constexpr static std::string_view class_path = ClassTraits<native_boxed_type>::class_path;
        constexpr static std::string_view java_name = ClassTraits<native_boxed_type>::java_name;

        static native_type native_value(JNIEnv* env, java_type javaSet)
        {
            set_view<element_type> view(env, javaSet);
            set_view_iterator<element_type> iterator = view.iterator();
            native_type nativeSet;
            while (iterator.has_next()) {
                nativeSet.insert(iterator.get_next());
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
                LocalObjectRef element(env, arg_type_t<element_type>::java_value(env, item));
                env->CallBooleanMethod(javaSet, addFunc.ref(), element.ref());
            }
            return javaSet;
        }
    };

    template <typename T>
    struct JavaOrderedSetType : JavaSetType<std::set, T>
    {};

    template <typename T>
    struct JavaUnorderedSetType : JavaSetType<std::unordered_set, T>
    {};

    /**
     * Converts a native dictionary (e.g. a [map] or [unordered_map]) into a Java Map.
     */
    template <template<typename...> typename M, typename K, typename V>
    struct JavaMapType : AssignableJavaType<M<K, V>>
    {
        using native_type = M<K, V>;
        using native_boxed_type = M<boxed_t<K>, boxed_t<V>>;
        using key_type = typename native_boxed_type::key_type;
        using value_type = typename native_boxed_type::mapped_type;
        using java_type = jobject;

        constexpr static std::string_view class_name = ClassTraits<native_boxed_type>::class_name;
        constexpr static std::string_view class_path = ClassTraits<native_boxed_type>::class_path;
        constexpr static std::string_view java_name = ClassTraits<native_boxed_type>::java_name;

        static native_type native_value(JNIEnv* env, java_type javaMap)
        {
            map_view<key_type, value_type> view(env, javaMap);
            map_view_iterator<key_type, value_type> iterator = view.iterator();
            native_type nativeMap;
            while (iterator.has_next()) {
                auto item = iterator.get_next();
                nativeMap[std::move(item.key)] = std::move(item.value);
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
                LocalObjectRef key(env, arg_type_t<key_type>::java_value(env, item.first));
                LocalObjectRef value(env, arg_type_t<value_type>::java_value(env, item.second));
                env->CallObjectMethod(javaMap, putFunc.ref(), key.ref(), value.ref());
            }
            return javaMap;
        }
    };

    template <typename K, typename V>
    struct JavaOrderedMapType : JavaMapType<std::map, K, V>
    {};

    template <typename K, typename V>
    struct JavaUnorderedMapType : JavaMapType<std::unordered_map, K, V>
    {};

    template <typename T> struct ArgType<std::vector<T>, std::enable_if_t<!std::is_arithmetic_v<T>>> { using type = JavaListType<T>; };
    template <typename T> struct ArgType<std::set<T>> { using type = JavaOrderedSetType<T>; };
    template <typename T> struct ArgType<std::unordered_set<T>> { using type = JavaUnorderedSetType<T>; };
    template <typename K, typename V> struct ArgType<std::map<K, V>> { using type = JavaOrderedMapType<K, V>; };
    template <typename K, typename V> struct ArgType<std::unordered_map<K, V>> { using type = JavaUnorderedMapType<K, V>; };
}
