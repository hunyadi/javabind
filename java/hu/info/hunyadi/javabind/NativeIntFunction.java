package hu.info.hunyadi.javabind;

import java.util.function.IntFunction;

/**
 * Represents an object that wraps a native callback function.
 */
public final class NativeIntFunction<R> implements AutoCloseable, IntFunction<R> {
    protected NativeIntFunction() {}

    private long nativePointer = 0;

    public native R apply(int value);

    public native void close();

    protected void finalize() {
        close();
    }
}
