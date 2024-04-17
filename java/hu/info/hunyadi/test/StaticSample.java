package hu.info.hunyadi.test;

import java.util.List;
import java.util.Set;
import java.util.Map;
import java.util.function.Predicate;
import java.util.function.IntPredicate;
import java.util.function.LongPredicate;
import java.util.function.DoublePredicate;
import java.util.function.Function;
import java.util.function.IntFunction;
import java.util.function.LongFunction;
import java.util.function.DoubleFunction;
import java.util.function.ToIntFunction;
import java.util.function.ToLongFunction;
import java.util.function.ToDoubleFunction;
import java.util.function.Consumer;
import java.util.function.IntConsumer;
import java.util.function.LongConsumer;
import java.util.function.DoubleConsumer;

public class StaticSample {
    public static native void returns_void();

    public static native boolean returns_bool();

    public static native int returns_int();

    public static native String returns_string();

    public static native boolean pass_bool(boolean value);

    public static native byte pass_byte(byte value);

    public static native char pass_char(char value);

    public static native short pass_short(short value);

    public static native int pass_int(int value);

    public static native long pass_long(long value);

    public static native float pass_float(float value);

    public static native double pass_double(double value);

    public static native String pass_string(String value);

    public static native String pass_utf8_string(String value);

    public static native void pass_utf16_string(String value);

    public static native Boolean pass_boxed_boolean(Boolean value);

    public static native Integer pass_boxed_integer(Integer value);

    public static native Long pass_boxed_long(Long value);

    public static native Double pass_boxed_double(Double value);

    public static native boolean[] pass_bool_array(boolean[] values);

    public static native byte[] pass_byte_array(byte[] values);

    public static native char[] pass_char_array(char[] values);

    public static native short[] pass_short_array(short[] values);

    public static native int[] pass_int_array(int[] values);

    public static native long[] pass_long_array(long[] values);

    public static native float[] pass_float_array(float[] values);

    public static native double[] pass_double_array(double[] values);

    public static native boolean[] pass_bool_array_view(boolean[] values);

    public static native byte[] pass_byte_array_view(byte[] values);

    public static native short[] pass_short_array_view(short[] values);

    public static native int[] pass_int_array_view(int[] values);

    public static native long[] pass_long_array_view(long[] values);

    public static native float[] pass_float_array_view(float[] values);

    public static native double[] pass_double_array_view(double[] values);

    public static native String pass_function(String s, Function<String, String> fn);

    public static native Function<String, String> returns_function(String search, String replace);

    public static native void apply_int_consumer(int value, IntConsumer fn);

    public static native void apply_long_consumer(long value, LongConsumer fn);

    public static native void apply_double_consumer(double value, DoubleConsumer fn);

    public static native void apply_string_consumer(String value, Consumer<String> fn);

    public static native boolean apply_int_predicate(int value, IntPredicate fn);

    public static native boolean apply_long_predicate(long value, LongPredicate fn);

    public static native boolean apply_double_predicate(double value, DoublePredicate fn);

    public static native boolean apply_string_predicate(String value, Predicate<String> fn);

    public static native String apply_int_to_string_function(int value, IntFunction<String> fn);

    public static native String apply_long_to_string_function(long value, LongFunction<String> fn);

    public static native String apply_double_to_string_function(double value, DoubleFunction<String> fn);

    public static native int apply_string_to_int_function(String value, ToIntFunction<String> fn);

    public static native long apply_string_to_long_function(String value, ToLongFunction<String> fn);

    public static native double apply_string_to_double_function(String value, ToDoubleFunction<String> fn);

    public static native IntFunction<String> get_int_to_string_function();

    public static native LongFunction<String> get_long_to_string_function();

    public static native DoubleFunction<String> get_double_to_string_function();

    public static native ToIntFunction<String> get_string_to_int_function();

    public static native ToLongFunction<String> get_string_to_long_function();

    public static native ToDoubleFunction<String> get_string_to_double_function();

    public static native Consumer<String> get_string_consumer();

    public static native IntConsumer get_int_consumer();

    public static native LongConsumer get_long_consumer();

    public static native DoubleConsumer get_double_consumer();

    public static native Rectangle pass_record(Rectangle rect);

    public static native PrimitiveRecord transform_record(PrimitiveRecord rec);

    public static native List<Rectangle> pass_list(List<Rectangle> list);

    public static native Set<String> pass_ordered_set(Set<String> set);

    public static native Set<String> pass_unordered_set(Set<String> set);

    public static native Map<String, Rectangle> pass_ordered_map(Map<String, Rectangle> map);

    public static native Map<String, Rectangle> pass_unordered_map(Map<String, Rectangle> map);
}
