package hu.info.hunyadi.javabind;

import java.util.function.DoubleFunction;

/**
 * Represents an object that wraps a native callback function.
 */
public final class NativeDoubleFunction<R> implements AutoCloseable, DoubleFunction<R> {
    protected NativeDoubleFunction() {}

    private long nativePointer = 0;

    public native R apply(double value);

    public native void close();

    protected void finalize() {
        close();
    }
}
