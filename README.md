# javabind: C++ and Java interoperability

javabind is a lightweight C++17 header-only library that exposes C++ types to Java and vice versa, primarily in order to create Java bindings for existing C++ code. The objective of this library is to provide an easy-to-use interoperability interface that feels natural from both C++ and Java. javabind uses C++ compile-time introspection to generate Java Native Interface (JNI) stubs that can be accessed from Java with regular function invocation. The stubs incur minimal or no overhead compared to hand-written JNI code.

This project has been inspired by a similar binding interface between JavaScript and C++ in [emscripten](https://emscripten.org), between Python and C++ in [PyBind11](https://pybind11.readthedocs.io/en/stable/) and [Boost.Python](https://www.boost.org/doc/libs/1_81_0/libs/python/doc/html/index.html), and between Kotlin and C++ in [ktbind](https://github.com/hunyadi/ktbind). Unlike [JNA](https://github.com/java-native-access/jna), which one can utilize by means of an intermediary C interface, javabind offers a direct interface between C++ and Java.

## Core features

The following C++ features can be mapped to Java:

* Fundamental types and custom data structures as function arguments or return values
* Instance methods and static methods
* Functions with template parameters
* Overloaded functions
* Operators
* Instance attributes and static attributes
* Arbitrary exception types
* STL containers

Furthermore, the following Java features are seamlessly exposed to C++:

* Primitive types and boxed primitive types
* Arrays of primitive types passed with zero-copy semantics
* Collection types
* Functional interfaces and lambda expressions
* Arbitrary exception types

## Getting started

javabind is a header-only library, including the interoperability header in your C++ project allows you to create bindings to Java:

```cpp
#include <javabind/javabind.hpp>
```

Consider the following C++ class as an example:

```cpp
class Person
{
    std::string name;
    Residence residence;
public:
    Person() = default;
    Person(const std::string& n) : name(n) {}
    Person(const std::string& n, const Residence& r) : name(n), residence(r) {}
    Residence get_residence() const { return residence; }
    void set_residence(const Residence& r) { residence = r; }
};
```

Let's suppose we want to expose this classes to Java. All Java bindings should be registered in the extension module block. We use `native_class` and its builder functions `constructor` and `function` to expose the member functions of the class `Person`:

```cpp
DECLARE_NATIVE_CLASS(Person, "hu.info.hunyadi.test.Person");

JAVA_EXTENSION_MODULE() {
    using namespace javabind;
    native_class<Person>()
        .constructor<Person(std::string)>("create")
        .constructor<Person(std::string, Residence)>("create")
        .function<&Person::get_residence>("getResidence")
        .function<&Person::set_residence>("setResidence")
        ;
    print_registered_bindings();
}
```

`DECLARE_NATIVE_CLASS` assigns a fully-qualified Java class name to the C++ class, which is required to look up objects at run time.

The type parameter of the template function `constructor` is a function signature to help choose between multiple available constructors. Use the same style you would with `std::function<R(Args...)>`.

The non-type template parameter of `function` is a function pointer, either a member function pointer (as shown above) or a free function pointer. If multiple functions have the same name (making the pointer reference ambiguous), a static cast to the right signature might be necessary.

`print_registered_bindings` is a utility function that lets you print the Java class definition that corresponds to the registered C++ class definitions. `print_registered_bindings` prints to Java `System.out` when you load the compiled shared library (`*.so` on macOS and Linux, or `*.dll` on Windows) with Java's `System.loadLibrary()`. You would normally use it in the development phase.

Next, we need corresponding native bindings in Java:

```java
package hu.info.hunyadi.test;

import hu.info.hunyadi.javabind.NativeObject;

public class Person extends NativeObject {
    public static native Person create(String name);
    public static native Person create(String name, Residence residence);
    public native void close();
    public native Residence getResidence();
    public native void setResidence(Residence residence);
}
```

The example above highlights many interesting characteristics.

First, `Person` derives from `NativeObject`. Internally, `NativeObject` stores a raw pointer as a Java `long`. This raw pointer is completely opaque to Java, and refers to an object in the C++ memory space. When methods of `Person` are called, the raw pointer is de-referenced, and the call is routed to the C++ mapping of `Person`, as registered by `native_class` in the extension module block.

Second, there are two factory methods to create instances of `Person`. Factory methods are necessary because the raw pointer has to be populated in C++, and cannot be set in Java. The `constructor` call in `native_class` routes the Java factory method to template-generated C++ code, which creates the object, calls the appropriate C++ constructor, sets the raw pointer, and returns a reference to the object. Meanwhile, `NativeObject` has a protected constructor to prevent instantiating `Person` objects directly in Java.

Third, `Person` implements the `close` method inherited from the `AutoCloseable` interface (via `NativeObject`). C++ objects have constructors and destructors but Java (JVM) has garbage collection. In order to ensure that objects are properly reclaimed when they are no longer needed, `native_class` in C++ binds a template-generated de-allocator to `close`, and calling the `close` method triggers the C++ destructor. You may have `close()` called automatically at the end of a `try` block in Java.

Finally, getter and setter methods involve a Java record class called `Residence`. `Residence` is defined in Java as follows:

```java
public record Residence(String country, String city) {
    String getCountry() {
        return country;
    }

    String getCity() {
        return city;
    }
}
```

In order to make fields of the record class accessible from C++, we need to define a corresponding C++ class and declare bindings:

```cpp
struct Residence
{
    std::string country;
    std::string city;
};

DECLARE_RECORD_CLASS(Residence, "hu.info.hunyadi.test.Residence");

JAVA_EXTENSION_MODULE()
{
    using namespace javabind;
    // ...
    record_class<Residence>()
        .field<&Residence::country>("country")
        .field<&Residence::city>("city")
        ;
}
```

The above declaration makes `Residence` a type that we can pass and return in function calls. When `Residence` objects are received, data in Java is copied into the `struct` defined in C++. When `Residence` objects are returned, data in C++ is copied into the Java record class.

## Signatures

C++ function signatures that are invoked from Java can take arguments by value or by const reference. C++ functions return simple or composite types by value.

## Type mapping

javabind recognizes several widely-used types and marshals them automatically between C++ and Java without explicit user-defined type specification:

| C++ type | Java consumed type | Java produced type |
| -------- | ------------------ | ------------------ |
| `void` | n/a | `void` |
| `bool` | `boolean` | `boolean` |
| `int8_t` | `byte` | `byte` |
| `char16_t` | `char` | `char` |
| `int16_t` | `short` | `short` |
| `int32_t` | `int` | `int` |
| `int64_t` | `long` | `long` |
| `float` | `float` | `float` |
| `double` | `double` | `double` |
| `std::string` (UTF-8) | `String` | `String` |
| `std::string_view` (UTF-8) | `String` | `String` |
| `std::u16string_view` (UTF-16) | `String` | `String` |
| `boxed<bool>` | `Boolean` | `Boolean` |
| `boxed<int8_t>` | `Byte` | `Byte` |
| `boxed<char16_t>` | `Character` | `Character` |
| `boxed<int16_t>` | `Short` | `Short` |
| `boxed<int32_t>` | `Integer` | `Integer` |
| `boxed<int64_t>` | `Long` | `Long` |
| `boxed<float>` | `Float` | `Float` |
| `boxed<double>` | `Double` | `Double` |
| `std::basic_string_view<T>` if `T` is an arithmetic type | `T[]` | n/a |
| `std::vector<T>` if `T` is an arithmetic type | `T[]` | `T[]` |
| `std::vector<T>` if `T` is not an arithmetic type | `java.util.List<T>` | `java.util.ArrayList<T>` |
| `std::set<E>` | `java.util.Set<E>` | `java.util.TreeSet<E>` |
| `std::unordered_set<E>` | `java.util.Set<E>` | `java.util.HashSet<E>` |
| `std::map<K,V>` | `java.util.Map<K,V>` | `java.util.TreeMap<K,V>` |
| `std::unordered_map<K,V>` | `java.util.Map<K,V>` | `java.util.HashMap<K,V>` |
| `std::optional<T>` | `T` | `T` |
| `std::function<R(T)>` | `Function<T,R>` | `NativeFunction<T,R>` implements `Function<T,R>` |
| `std::function<R(int32_t)>` | `IntFunction<R>` | `NativeIntFunction<R>` implements `IntFunction<R>` |
| `std::function<R(int64_t)>` | `LongFunction<R>` | `NativeLongFunction<R>` implements `LongFunction<R>` |
| `std::function<R(double)>` | `DoubleFunction<R>` | `NativeDoubleFunction<R>` implements `DoubleFunction<R>` |
| `std::function<int32_t(T)>` | `ToIntFunction<T>` | `NativeToIntFunction<T>` implements `ToIntFunction<T>` |
| `std::function<int64_t(T)>` | `ToLongFunction<T>` | `NativeToLongFunction<T>` implements `ToLongFunction<T>` |
| `std::function<double(T)>` | `ToDoubleFunction<T>` | `NativeToDoubleFunction<T>` implements `ToDoubleFunction<T>` |
| `std::function<bool(T)>` | `Predicate<T>` | `NativePredicate<T>` implements `Predicate<T>` |
| `std::function<bool(int32_t)>` | `IntPredicate` | `NativeIntPredicate` implements `IntPredicate` |
| `std::function<bool(int64_t)>` | `LongPredicate` | `NativeLongPredicate` implements `LongPredicate` |
| `std::function<bool(double)>` | `DoublePredicate` | `NativeDoublePredicate` implements `DoublePredicate` |
| `std::function<void(T)>` | `Consumer<T>` | `NativeConsumer<T>` implements `Consumer<T>` |
| `std::function<void(int32_t)>` | `IntConsumer` | `NativeIntConsumer` implements `IntConsumer` |
| `std::function<void(int64_t)>` | `LongConsumer` | `NativeLongConsumer` implements `LongConsumer` |
| `std::function<void(double)>` | `DoubleConsumer` | `NativeDoubleConsumer` implements `DoubleConsumer` |

`boxed` is a lightweight C++ wrapper defined by the library to match Java boxed types such as `java.lang.Integer`. `boxed` has no C++ run-time overhead, it is only used for disambiguation.

Collection types are copied between C++ and Java.

Optionals are converted to a null-value in Java when they don't have a value in C++. Null-values are converted to an empty optional in C++.

C++ types `basic_string_view<T>` translate to JNI calls `GetPrimitiveArrayCritical` and `ReleasePrimitiveArrayCritical` to get a direct pointer to the memory managed by the Java Virtual Machine (JVM). This imposes [significant restrictions](https://docs.oracle.com/javase/8/docs/technotes/guides/jni/spec/functions.html#GetPrimitiveArrayCritical_ReleasePrimitiveArrayCritical):

> After calling `GetPrimitiveArrayCritical`, the native code should not run for an extended period of time before it calls `ReleasePrimitiveArrayCritical`. We must treat the code inside this pair of functions as running in a "critical region." Inside a critical region, native code must not call other JNI functions, or any system call that may cause the current thread to block and wait for another Java thread. (For example, the current thread must not call read on a stream being written by another Java thread.)

The C++ type `u16string_view` translates to JNI calls `GetStringCritical` and `ReleaseStringCritical`, which entail similar restrictions as `GetPrimitiveArrayCritical` and `ReleasePrimitiveArrayCritical`.

## Exceptions

Exceptions thrown in C++ automatically trigger a Java exception when crossing the language boundary. The interoperability layer catches all exceptions that inherit from `std::exception`, and throws a `java.lang.Exception` before passing control back to the JVM.

Exceptions originating from Java are automatically wrapped in a C++ type called `JavaException`, which derives from `std::exception`. The function `what()` in `JavaException` retrieves the Java exception message. C++ code can catch `JavaException` and take appropriate action, which causes the exception to be cleared in Java.

## Functional interface

javabind can expose C++ function objects (`std::function<R(T)>`) to Java with wrappers that implement functional interfaces such as `Function<T,R>` or `Predicate<T>`. Each wrapper such as `NativeFunction<T,R>` or `NativePredicate<T>` extends the abstract base class `NativeCallback`, which is responsible for encapsulating a raw pointer. This raw pointer points at a memory location in the C++ domain, allocated with the operator `new`, and de-allocated with `delete` once the Java wrapper is garbage collected. Invocation is done in a way similar to regular native class methods but the call is bound not to an object instance (as with `NativeObject`) but to a function object.

Because function objects as C++ return values are depending on class definitions in Java, auxiliary classes such as `NativeFunction<T,R>` or `NativePredicate<T>` must be available on the class path at binding registration time to be accessible for `FindClass`. All of these are defined in the namespace `hu.info.hunyadi.javabind`.

Auxiliary classes use `java.lang.ref.Cleaner` to ensure associated native resources are reclaimed when the Java object becomes phantom reachable.

## Binding registration

The macro `JAVA_EXTENSION_MODULE` expands into a pair of function definitions:

```c
JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved) { ... }
JNIEXPORT void JNI_OnUnload(JavaVM* vm, void* reserved) { ... }
```

These definitions, in turn, iterate over the function and field bindings registered with `native_class`, `static_class` and `record_class`.

Each function binding generates a function pointer at compile time, which are passed to the JNI function `RegisterNatives`. Each of these function pointers points at a static member function of a template class, where the template parameters capture the type information extracted from the function signature. When the function is invoked through the pointer, the function makes the appropriate type conversions to cast Java types into C++ types and back. For example, the C++ function signature

```cpp
bool func(const std::string& str, const std::vector<int32_t>& vec, double d);
```

causes the function adapter template to be instantiated with parameter types `std::string`, `std::vector<int32_t>` and `double` and return type `bool`. When Java calls the pointer through JNI, the adapter transforms the types `std::string` and `std::vector<int32_t>`. (`double` and `bool` need no transformation.) For each transformed type, a temporary object is created, all of which are then used in invoking the original function `func`.

Internally, field bindings utilize JNI accessor functions like `GetObjectField` and `SetObjectField` to extract and populate Java objects. Like with function bindings, javabind uses C++ type information to make the appropriate JNI function call. For instance, setting a field with type `double` entails a call to `GetDoubleField` (from Java to C++) or `SetDoubleField` (from C++ to Java). If the type is a composite type, such as a `std::vector<T>`, then a Java object is constructed recursively, and then set with `SetObjectField`. For example,

* Setting a field of type `std::vector<boxed<int32_t>>` first creates a `java.util.ArrayList` with JNI's `NewObject`, then sets elements with the `add` method (invoked using JNI's `CallBooleanMethod`), performing boxing for the primitive type `int` with `valueOf`, and finally uses `SetObjectField` with the newly created `java.util.ArrayList` instance.
* Setting an `std::vector<std::string>` field involves creating a `java.util.ArrayList` with JNI's `NewObject`, and a call to JNI's `NewStringUTF` for each string element. The strings are then added to the `java.util.ArrayList` instance with `add`, and finally to the field with `SetObjectField`.
