package hu.info.hunyadi.javabind;

import java.util.function.ToLongFunction;

/**
 * Represents an object that wraps a native callback function.
 */
public final class NativeToLongFunction<T> implements AutoCloseable, ToLongFunction<T> {
    protected NativeToLongFunction() {}

    private long nativePointer = 0;

    public native long applyAsLong(T value);

    public native void close();

    protected void finalize() {
        close();
    }
}
