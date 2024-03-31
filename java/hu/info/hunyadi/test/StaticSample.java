package hu.info.hunyadi.test;

import java.util.function.Function;

public class StaticSample {
    public static native void returns_void();

    public static native boolean returns_bool();
    public static native int returns_int();
    public static native long returns_long();
    public static native float returns_float();
    public static native double returns_double();
    public static native String returns_string();

    public static native boolean pass_bool(boolean value);
    public static native int pass_int(int value);
    public static native long pass_long(long value);
    public static native float pass_float(float value);
    public static native double pass_double(double value);
    public static native String pass_string(String value);

    public static native Integer pass_boxed(Integer value);

    public static native boolean[] pass_bool_array(boolean[] values);
    public static native int[] pass_int_array(int[] values);
    public static native long[] pass_long_array(long[] values);
    public static native float[] pass_float_array(float[] values);
    public static native double[] pass_double_array(double[] values);

    public static native String pass_function(String s, Function<String, String> fn);
}
