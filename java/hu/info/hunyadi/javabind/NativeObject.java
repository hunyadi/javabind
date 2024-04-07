/**
 * javabind: effective C++ and Java interoperability
 * @see https://github.com/hunyadi/javabind
 *
 * Copyright (c) 2024 Levente Hunyadi
 *
 * This work is licensed under the terms of the MIT license.
 * For a copy, see <https://opensource.org/licenses/MIT>.
 */

package hu.info.hunyadi.javabind;

/**
 * Represents a class that is instantiated in native code.
 */
public abstract class NativeObject implements AutoCloseable {
    /**
     * Prevents this class from being constructed directly in Java.
     *
     * The native pointer member is assigned in native code via a factory function.
     */
    protected NativeObject() {
    }

    /**
     * Holds an opaque reference to an object that exists in the native code
     * execution context.
     */
    @SuppressWarnings("unused")
    private long nativePointer = 0;

    /**
     * Disposes of objects allocated in the native code execution context.
     */
    public abstract void close();
}
