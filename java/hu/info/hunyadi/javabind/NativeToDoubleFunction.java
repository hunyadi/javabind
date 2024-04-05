package hu.info.hunyadi.javabind;

import java.util.function.ToDoubleFunction;

/**
 * Represents an object that wraps a native callback function.
 */
public final class NativeToDoubleFunction<T> implements AutoCloseable, ToDoubleFunction<T> {
    protected NativeToDoubleFunction() {}

    private long nativePointer = 0;

    public native double applyAsDouble(T value);

    public native void close();

    protected void finalize() {
        close();
    }
}
