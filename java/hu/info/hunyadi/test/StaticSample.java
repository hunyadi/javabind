package hu.info.hunyadi.test;

import java.util.function.Function;
import java.util.function.IntFunction;
import java.util.function.LongFunction;
import java.util.function.DoubleFunction;
import java.util.function.ToIntFunction;
import java.util.function.ToLongFunction;
import java.util.function.ToDoubleFunction;

public class StaticSample {
    public static native void returns_void();
    public static native boolean returns_bool();
    public static native int returns_int();
    public static native String returns_string();

    public static native boolean pass_bool(boolean value);
    public static native byte pass_byte(byte value);
    public static native short pass_short(short value);
    public static native int pass_int(int value);
    public static native long pass_long(long value);
    public static native float pass_float(float value);
    public static native double pass_double(double value);
    public static native String pass_string(String value);

    public static native Boolean pass_boxed_boolean(Boolean value);
    public static native Integer pass_boxed_integer(Integer value);

    public static native boolean[] pass_bool_array(boolean[] values);
    public static native byte[] pass_byte_array(byte[] values);
    public static native short[] pass_short_array(short[] values);
    public static native int[] pass_int_array(int[] values);
    public static native long[] pass_long_array(long[] values);
    public static native float[] pass_float_array(float[] values);
    public static native double[] pass_double_array(double[] values);

    public static native String pass_function(String s, Function<String, String> fn);
    public static native String int_to_string_function(int value, IntFunction<String> fn);
    public static native String long_to_string_function(long value, LongFunction<String> fn);
    public static native String double_to_string_function(double value, DoubleFunction<String> fn);
    public static native int string_to_int_function(String value, ToIntFunction<String> fn);
    public static native long string_to_long_function(String value, ToLongFunction<String> fn);
    public static native double string_to_double_function(String value, ToDoubleFunction<String> fn);

    public static native Rectangle pass_record(Rectangle rect);
}
