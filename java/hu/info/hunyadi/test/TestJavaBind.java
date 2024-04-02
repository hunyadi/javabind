package hu.info.hunyadi.test;

import java.util.Arrays;
import hu.info.hunyadi.test.Rectangle;
import hu.info.hunyadi.test.Sample;
import hu.info.hunyadi.test.StaticSample;

public class TestJavaBind {
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
        assert StaticSample.pass_string("ok").equals("ok");
        System.out.println("PASS: class functions with simple types");

        assert StaticSample.pass_boxed_boolean(Boolean.valueOf(true)).equals(Boolean.valueOf(true));
        assert StaticSample.pass_boxed_integer(Integer.valueOf(23)).equals(Integer.valueOf(23));
        System.out.println("PASS: class functions with boxed types");

        boolean[] bool_array = new boolean[]{true, false, false};
        int[] int_array = new int[]{1, 2, 3};
        long[] long_array = new long[]{4, 5, 6, 7, 8, 9};
        float[] float_array = new float[]{1.0f, 0.5f, 0.25f};
        double[] double_array = new double[]{1.0, 0.5, 0.25};
        assert Arrays.equals(StaticSample.pass_bool_array(bool_array), bool_array);
        assert Arrays.equals(StaticSample.pass_int_array(int_array), int_array);
        assert Arrays.equals(StaticSample.pass_long_array(long_array), long_array);
        assert Arrays.equals(StaticSample.pass_float_array(float_array), float_array);
        assert Arrays.equals(StaticSample.pass_double_array(double_array), double_array);
        System.out.println("PASS: class functions with array types");

        assert StaticSample.pass_function("my string", s -> "'" + s + "'").equals("my string -> 'my string'");
        assert StaticSample.int_to_string_function(123, val -> String.valueOf(val)).equals("123");
        assert StaticSample.long_to_string_function(456789, val -> String.valueOf(val)).equals("456789");
        assert StaticSample.double_to_string_function(0.125, val -> String.valueOf(val)).equals("0.125");
        assert StaticSample.string_to_int_function("123", val -> Integer.parseInt(val)) == 123;
        assert StaticSample.string_to_long_function("456789", val -> Long.parseLong(val)) == 456789;
        assert StaticSample.string_to_double_function("0.125", val -> Double.parseDouble(val)) == 0.125;
        System.out.println("PASS: functional interface");

        assert StaticSample.pass_record(new Rectangle(1.0, 2.0)).equals(new Rectangle(2.0, 4.0));
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
            assert person.getResidence().getCity().equals("Budapest");
            person.setResidence(vienna);
            assert person.getResidence().getCity().equals("Wien");
        }
        System.out.println("PASS: getters and setters with record class");
    }
}
