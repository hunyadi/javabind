package hu.info.hunyadi.javabind;

import java.util.function.ToIntFunction;

/**
 * Represents an object that wraps a native callback function.
 */
public final class NativeToIntFunction<T> implements AutoCloseable, ToIntFunction<T> {
    protected NativeToIntFunction() {}

    private long nativePointer = 0;

    public native int applyAsInt(T value);

    public native void close();

    protected void finalize() {
        close();
    }
}
