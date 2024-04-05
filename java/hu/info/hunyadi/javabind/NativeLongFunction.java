package hu.info.hunyadi.javabind;

import java.util.function.LongFunction;

/**
 * Represents an object that wraps a native callback function.
 */
public final class NativeLongFunction<R> implements AutoCloseable, LongFunction<R> {
    protected NativeLongFunction() {}

    private long nativePointer = 0;

    public native R apply(long value);

    public native void close();

    protected void finalize() {
        close();
    }
}
