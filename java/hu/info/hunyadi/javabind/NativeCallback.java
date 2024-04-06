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
 * Represents an object that wraps a native callback function.
 */
public abstract class NativeCallback implements AutoCloseable {
    /**
     * Prevents this class from being constructed directly in Java.
     *
     * The native pointer member is assigned in native code via a factory function.
     */
    protected NativeCallback() {}

    /**
     * Holds an opaque reference to an object that exists in the native code execution context.
     */
    private long nativePointer = 0;

    /**
     * Disposes of objects allocated in the native code execution context.
     */
    public abstract void close();

    protected void finalize() {
        close();
    }
}
