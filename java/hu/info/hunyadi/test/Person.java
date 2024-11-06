package hu.info.hunyadi.test;

import hu.info.hunyadi.javabind.NativeObject;

public class Person extends NativeObject {
    public static native Person create(String name);

    public static native Person create(String name, Residence residence);

    public native void close();

    public native String getName();

    public native void setName(String name);

    public native Residence getResidence();

    public native void setResidence(Residence residence);

    public native java.util.List<hu.info.hunyadi.test.Person> getChildren();

    public native void setChildren(java.util.List<Person> children);
}
