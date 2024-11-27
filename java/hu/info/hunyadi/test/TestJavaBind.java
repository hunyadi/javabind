package hu.info.hunyadi.test;

import java.util.Arrays;
import java.util.function.Function;
import java.util.List;
import java.util.Set;
import java.util.Map;

import java.time.Duration;
import java.time.Instant;

public class TestJavaBind {
    public static String string_transform(String source) {
        return source.replace(' ', '_');
    }

    public static void main(String[] args) {
        System.out.println("LOAD: Java host application");
        System.loadLibrary("javabind_native");
        System.out.println("LOAD: native library");

        StaticSample.returns_void();
        assert StaticSample.returns_bool();
        assert StaticSample.returns_int() == 82;
        assert StaticSample.returns_string().equals("a sample string");
        assert StaticSample.pass_bool(true) == true;
        assert StaticSample.pass_bool(false) == false;
        assert StaticSample.pass_char('A') == 'A';
        assert StaticSample.pass_short(Short.MIN_VALUE) == Short.MIN_VALUE;
        assert StaticSample.pass_short(Short.MIN_VALUE) == Short.MIN_VALUE;
        assert StaticSample.pass_int(Integer.MIN_VALUE) == Integer.MIN_VALUE;
        assert StaticSample.pass_int(Integer.MAX_VALUE) == Integer.MAX_VALUE;
        assert StaticSample.pass_long(Long.MIN_VALUE) == Long.MIN_VALUE;
        assert StaticSample.pass_long(Long.MAX_VALUE) == Long.MAX_VALUE;
        assert StaticSample.pass_float(2.5f) == 2.5f;
        assert StaticSample.pass_float(Float.MAX_VALUE) == Float.MAX_VALUE;
        assert StaticSample.pass_double(1.125) == 1.125;
        assert StaticSample.pass_double(Double.MAX_VALUE) == Double.MAX_VALUE;
        assert StaticSample.pass_foo_bar(FooBar.Foo) == FooBar.Foo;
        assert StaticSample.pass_foo_bar(FooBar.Bar) == FooBar.Bar;
        assert StaticSample.pass_nanoseconds(Duration.ofNanos(1000)).equals(Duration.ofNanos(1000));
        assert StaticSample.pass_microseconds(Duration.ofNanos(1000000)).equals(Duration.ofNanos(1000000));
        assert StaticSample.pass_milliseconds(Duration.ofMillis(1000)).equals(Duration.ofMillis(1000));
        assert StaticSample.pass_seconds(Duration.ofSeconds(1000)).equals(Duration.ofSeconds(1000));
        assert StaticSample.pass_minutes(Duration.ofMinutes(1000)).equals(Duration.ofMinutes(1000));
        assert StaticSample.pass_hours(Duration.ofHours(1000)).equals(Duration.ofHours(1000));
        assert StaticSample.pass_time_point(Instant.ofEpochSecond(1000, 15)).equals(Instant.ofEpochSecond(1000, 15));
        assert StaticSample.pass_string("ok").equals("ok");
        assert StaticSample.pass_utf8_string("árvíztűrő tükörfúrógép").equals("árvíztűrő tükörfúrógép");
        StaticSample.pass_utf16_string("árvíztűrő tükörfúrógép");
        System.out.println("PASS: class functions with simple types");

        short max_unsigned_byte = 255;
        assert StaticSample.pass_unsigned_byte(max_unsigned_byte) == max_unsigned_byte;
        int max_unsigned_short = 65535;
        assert StaticSample.pass_unsigned_short(max_unsigned_short) == max_unsigned_short;
        long max_unsigned_int = 4294967295l;
        assert StaticSample.pass_unsigned_int(max_unsigned_int) == max_unsigned_int;
        System.out.println("PASS: class functions with unsigned integer types");

        assert StaticSample.pass_boxed_boolean(Boolean.valueOf(true)).equals(Boolean.valueOf(true));
        assert StaticSample.pass_boxed_integer(Integer.valueOf(23)).equals(Integer.valueOf(23));
        assert StaticSample.pass_boxed_long(Long.MAX_VALUE).equals(Long.MAX_VALUE);
        assert StaticSample.pass_boxed_double(-Double.MAX_VALUE).equals(-Double.MAX_VALUE);
        System.out.println("PASS: class functions with boxed types");

        boolean[] bool_array = new boolean[] { true, false, false };
        char[] char_array = new char[] { 'A', 'l', 'm', 'a' };
        short[] short_array = new short[] { (short) 2, (short) 3 };
        int[] int_array = new int[] { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };
        long[] long_array = new long[] { 4, 5, 6, 7, 8, 9 };
        float[] float_array = new float[] { 1.0f, 0.5f, 0.25f };
        double[] double_array = new double[] { 1.0, 0.5, 0.25 };
        assert Arrays.equals(StaticSample.pass_bool_array(bool_array), bool_array);
        assert Arrays.equals(StaticSample.pass_char_array(char_array), char_array);
        assert Arrays.equals(StaticSample.pass_short_array(short_array), short_array);
        assert Arrays.equals(StaticSample.pass_int_array(int_array), int_array);
        assert Arrays.equals(StaticSample.pass_long_array(long_array), long_array);
        assert Arrays.equals(StaticSample.pass_float_array(float_array), float_array);
        assert Arrays.equals(StaticSample.pass_double_array(double_array), double_array);
        assert Arrays.equals(StaticSample.pass_bool_array_view(bool_array), bool_array);
        assert Arrays.equals(StaticSample.pass_short_array_view(short_array), short_array);
        assert Arrays.equals(StaticSample.pass_int_array_view(int_array), int_array);
        assert Arrays.equals(StaticSample.pass_long_array_view(long_array), long_array);
        assert Arrays.equals(StaticSample.pass_float_array_view(float_array), float_array);
        assert Arrays.equals(StaticSample.pass_double_array_view(double_array), double_array);
        System.out.println("PASS: class functions with array types");

        assert StaticSample.pass_function("my string", s -> "'" + s + "'").equals("my string -> 'my string'");
        Function<String, String> replace = StaticSample.returns_function(" ", "_");
        assert replace.apply("my string").equals("my_string");
        assert replace.apply("lorem ipsum dolor sit amet").equals("lorem_ipsum_dolor_sit_amet");

        StaticSample.apply_int_consumer(23, val -> System.out.println(val));
        StaticSample.apply_long_consumer(1989l, val -> System.out.println(val));
        StaticSample.apply_double_consumer(3.14159265359, val -> System.out.println(val));
        StaticSample.apply_string_consumer("start to finish", val -> System.out.println(val));

        assert StaticSample.apply_int_predicate(23, val -> val > 0);
        assert StaticSample.apply_long_predicate(1989l, val -> val > 0l);
        assert StaticSample.apply_double_predicate(3.14159265359, val -> val > 0.0);
        assert StaticSample.apply_string_predicate("start to finish", val -> val.startsWith("start"));

        assert StaticSample.apply_int_to_string_function(123, val -> String.valueOf(val)).equals("123");
        assert StaticSample.apply_long_to_string_function(456789, val -> String.valueOf(val)).equals("456789");
        assert StaticSample.apply_double_to_string_function(0.125, val -> String.valueOf(val)).equals("0.125");
        assert StaticSample.apply_string_to_int_function("123", val -> Integer.parseInt(val)) == 123;
        assert StaticSample.apply_string_to_long_function("456789", val -> Long.parseLong(val)) == 456789;
        assert StaticSample.apply_string_to_double_function("0.125", val -> Double.parseDouble(val)) == 0.125;

        assert StaticSample.get_int_to_string_function().apply(123).equals("123");
        assert StaticSample.get_long_to_string_function().apply(456789l).equals("456789");
        assert StaticSample.get_double_to_string_function().apply(0.125).equals("0.125");
        assert StaticSample.get_string_to_int_function().applyAsInt("123") == 123;
        assert StaticSample.get_string_to_long_function().applyAsLong("456789") == 456789l;
        assert StaticSample.get_string_to_double_function().applyAsDouble("0.125") == 0.125;

        StaticSample.get_string_consumer().accept("alma");
        StaticSample.get_int_consumer().accept(23);
        StaticSample.get_long_consumer().accept(1989l);
        StaticSample.get_double_consumer().accept(3.14);
        StaticSample.get_person_ref_consumer().accept(Person.create("Bert"));
        StaticSample.get_person_const_ref_consumer().accept(Person.create("Ernie"));

        try {
            StaticSample.get_string_to_int_function().applyAsInt("abcd");
            assert false;
        } catch (Exception e) {
        }
        System.out.println("PASS: functional interface");

        assert StaticSample.pass_record(new Rectangle(1.0, 2.0)).equals(new Rectangle(2.0, 4.0));
        PrimitiveRecord source = new PrimitiveRecord((byte) 1, '@', (short) 2, 3, 4l, 5.0f, 6.0);
        PrimitiveRecord target = new PrimitiveRecord((byte) 2, '@', (short) 4, 6, 8l, 10.0f, 12.0);
        assert StaticSample.transform_record(source).equals(target);
        System.out.println("PASS: record class");

        try (Sample obj = Sample.create()) {
            obj.returns_void();
            assert obj.returns_bool();
            assert obj.returns_int() == 82;

            assert obj.value() == 0;
            obj.add(10);
            assert obj.value() == 10;
            obj.add(13);
            assert obj.value() == 23;
        }
        System.out.println("PASS: class constructor and member functions");

        Residence budapest = new Residence("Hungary", "Budapest");
        Residence vienna = new Residence("Austria", "Wien");
        try (Person person = Person.create("Alma", budapest)) {
            assert person.getName().equals("Alma");
            person.setName("Dalma");
            assert person.getName().equals("Dalma");

            assert person.getResidence().getCity().equals("Budapest");
            person.setResidence(vienna);
            assert person.getResidence().getCity().equals("Wien");

            person.setChildren(List.of(Person.create("Bela"), Person.create("Cecil")));
            assert person.getChildren().size() == 2;
            assert person.getChildren().get(0).getName().equals("Bela");
            assert person.getChildren().get(1).getName().equals("Cecil");
        }
        System.out.println("PASS: getters and setters with record class");

        assert StaticSample.pass_list(List.of(new Rectangle(1.0, 2.0), new Rectangle(3.0, 4.0)))
                .equals(List.of(new Rectangle(1.0, 2.0), new Rectangle(3.0, 4.0)));
        assert StaticSample.pass_ordered_set(Set.of("one", "two", "three")).equals(Set.of("one", "two", "three"));
        assert StaticSample.pass_unordered_set(Set.of("one", "two", "three")).equals(Set.of("one", "two", "three"));
        assert StaticSample.pass_ordered_set_with_int_key(Set.of(1, 2, 3)).equals(Set.of(1, 2, 3));

        Map<String, Rectangle> rectangles = Map.ofEntries(
                Map.entry("a", new Rectangle(1.0, 2.0)),
                Map.entry("b", new Rectangle(3.0, 4.0)),
                Map.entry("c", new Rectangle(5.0, 6.0)));
        assert StaticSample.pass_ordered_map(rectangles).equals(rectangles);
        assert StaticSample.pass_unordered_map(rectangles).equals(rectangles);
        assert StaticSample.pass_ordered_map_with_int_key(Map.of(1, "one", 2, "two", 3, "three"))
                .equals(Map.of(1, "one", 2, "two", 3, "three"));
        assert StaticSample.pass_ordered_map_with_int_value(Map.of("one", 1, "two", 2, "three", 3))
                .equals(Map.of("one", 1, "two", 2, "three", 3));
        System.out.println("PASS: collections");

        assert StaticSample.pass_optional_rectangle(null) == null;
        assert StaticSample.pass_optional_rectangle(new Rectangle(1.0, 2.0)).equals(new Rectangle(1.0, 2.0));
        assert StaticSample.pass_optional_int(null) == null;
        assert StaticSample.pass_optional_int(23).equals(23);
        assert StaticSample.pass_optional_string(null) == null;
        assert StaticSample.pass_optional_string("ok").equals("ok");
        System.out.println("PASS: optional");

    }
}
