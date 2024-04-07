package hu.info.hunyadi.test;

import hu.info.hunyadi.javabind.NativeObject;

public class Person extends NativeObject {
    public static native Person create(String name);

    public static native Person create(String name, Residence residence);

    public native void close();

    public native Residence getResidence();

    public native void setResidence(Residence residence);
}
