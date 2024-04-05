package hu.info.hunyadi.javabind;

import java.util.function.Function;

/**
 * Represents an object that wraps a native callback function.
 */
public final class NativeFunction<T, R> implements AutoCloseable, Function<T, R> {
    /**
     * Prevents this class from being constructed directly in Java.
     *
     * The native pointer member is assigned in native code via a factory function.
     */
    protected NativeFunction() {}

    /**
     * Holds an opaque reference to an object that exists in the native code execution context.
     */
    private long nativePointer = 0;

    public native R apply(T t);

    /**
     * Disposes of objects allocated in the native code execution context.
     */
    public native void close();

    protected void finalize() {
        close();
    }
}
