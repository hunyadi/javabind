package hu.info.hunyadi.test;

import hu.info.hunyadi.javabind.NativeObject;

public class Sample extends NativeObject {
    /** Native objects need a static factory function. */
    public static native Sample create();

    /** Native objects implement AutoCloseable to dispose of resources. */
    public native void close();

    public native void returns_void();

    public native boolean returns_bool();

    public native int returns_int();

    public native int value();

    public native void add(int value);
}
