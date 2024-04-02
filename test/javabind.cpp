/**
 * javabind: effective C++ and Java interoperability
 * @see https://github.com/hunyadi/javabind
 *
 * Copyright (c) 2024 Levente Hunyadi
 *
 * This work is licensed under the terms of the MIT license.
 * For a copy, see <https://opensource.org/licenses/MIT>.
 */

#include <javabind/javabind.hpp>

template <typename L>
std::ostream& write_bracketed_list(std::ostream& os, const L& vec, char left, char right)
{
    os << left;
    if (!vec.empty()) {
        auto&& it = vec.begin();
        os << *it;

        for (++it; it != vec.end(); ++it) {
            os << ", " << *it;
        }
    }
    os << right;
    return os;
}

template <typename L>
std::ostream& write_list(std::ostream& os, const L& vec)
{
    return write_bracketed_list(os, vec, '[', ']');
}

template <typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& list)
{
    return write_list(os, list);
}

struct Rectangle
{
    Rectangle() = default;
    Rectangle(double width, double height)
        : width(width)
        , height(height)
    {}

    double width = 0.0;
    double height = 0.0;
};

struct Sample
{
    static void returns_void()
    {
        JAVA_OUTPUT << "returns_void()" << std::endl;
    }

    static bool returns_bool()
    {
        JAVA_OUTPUT << "returns_bool()" << std::endl;
        return true;
    }

    static int32_t returns_int()
    {
        JAVA_OUTPUT << "returns_int()" << std::endl;
        return 82;
    }

    int32_t value() const
    {
        return _value;
    }

    void operator+=(int32_t val)
    {
        _value += val;
    }

private:
    int32_t _value = 0;
};

static void returns_void()
{
    JAVA_OUTPUT << "returns_void()" << std::endl;
}

struct StaticSample
{
    static bool returns_bool()
    {
        JAVA_OUTPUT << "returns_bool()" << std::endl;
        return true;
    }

    static int32_t returns_int()
    {
        JAVA_OUTPUT << "returns_int()" << std::endl;
        return 82;
    }

    static std::string returns_string()
    {
        JAVA_OUTPUT << "returns_string()" << std::endl;
        return "a sample string";
    }

    static bool pass_bool(bool value)
    {
        JAVA_OUTPUT << "pass_bool(" << value << ")" << std::endl;
        return value;
    }

    static int8_t pass_byte(int8_t value)
    {
        JAVA_OUTPUT << "pass_byte(" << value << ")" << std::endl;
        return value;
    }

    static int16_t pass_short(int16_t value)
    {
        JAVA_OUTPUT << "pass_short(" << value << ")" << std::endl;
        return value;
    }

    static int32_t pass_int(int32_t value)
    {
        JAVA_OUTPUT << "pass_int(" << value << ")" << std::endl;
        return value;
    }

    static int64_t pass_long(int64_t value)
    {
        JAVA_OUTPUT << "pass_long(" << value << ")" << std::endl;
        return value;
    }

    static float pass_float(float value)
    {
        JAVA_OUTPUT << "pass_float(" << value << ")" << std::endl;
        return value;
    }

    static double pass_double(double value)
    {
        JAVA_OUTPUT << "pass_double(" << value << ")" << std::endl;
        return value;
    }

    static std::string pass_string(const std::string& value)
    {
        JAVA_OUTPUT << "pass_string(" << value << ")" << std::endl;
        return value;
    }

    static javabind::boxed<bool> pass_boxed_boolean(javabind::boxed<bool> value)
    {
        JAVA_OUTPUT << "pass_boxed_boolean(" << value << ")" << std::endl;
        return value;
    }

    static javabind::boxed<int32_t> pass_boxed_integer(javabind::boxed<int32_t> value)
    {
        JAVA_OUTPUT << "pass_boxed_integer(" << value << ")" << std::endl;
        return value;
    }

    static std::vector<bool> pass_bool_array(const std::vector<bool>& values)
    {
        JAVA_OUTPUT << "pass_bool_array(" << values << ")" << std::endl;
        return std::vector<bool>(values.begin(), values.end());
    }

    static std::vector<int8_t> pass_byte_array(const std::vector<int8_t>& values)
    {
        JAVA_OUTPUT << "pass_byte_array(" << values << ")" << std::endl;
        return std::vector<int8_t>(values.begin(), values.end());
    }

    static std::vector<int16_t> pass_short_array(const std::vector<int16_t>& values)
    {
        JAVA_OUTPUT << "pass_short_array(" << values << ")" << std::endl;
        return std::vector<int16_t>(values.begin(), values.end());
    }

    static std::vector<int32_t> pass_int_array(const std::vector<int32_t>& values)
    {
        JAVA_OUTPUT << "pass_int_array(" << values << ")" << std::endl;
        return std::vector<int32_t>(values.begin(), values.end());
    }

    static std::vector<int64_t> pass_long_array(const std::vector<int64_t>& values)
    {
        JAVA_OUTPUT << "pass_long_array(" << values << ")" << std::endl;
        return std::vector<int64_t>(values.begin(), values.end());
    }

    static std::vector<float> pass_float_array(const std::vector<float>& values)
    {
        JAVA_OUTPUT << "pass_float_array(" << values << ")" << std::endl;
        return std::vector<float>(values.begin(), values.end());
    }

    static std::vector<double> pass_double_array(const std::vector<double>& values)
    {
        JAVA_OUTPUT << "pass_double_array(" << values << ")" << std::endl;
        return std::vector<double>(values.begin(), values.end());
    }

    static std::string pass_function(const std::string& str, const std::function<std::string(std::string)>& fn)
    {
        JAVA_OUTPUT << "pass_function(" << str << ")" << std::endl;
        return str + " -> " + fn(str);
    }

    static std::string int_to_string_function(int32_t val, const std::function<std::string(int32_t)>& fn)
    {
        JAVA_OUTPUT << "int_to_string_function(" << val << ")" << std::endl;
        return fn(val);
    }

    static std::string long_to_string_function(int64_t val, const std::function<std::string(int64_t)>& fn)
    {
        JAVA_OUTPUT << "long_to_string_function(" << val << ")" << std::endl;
        return fn(val);
    }

    static std::string double_to_string_function(double val, const std::function<std::string(double)>& fn)
    {
        JAVA_OUTPUT << "double_to_string_function(" << val << ")" << std::endl;
        return fn(val);
    }

    static int32_t string_to_int_function(const std::string& val, const std::function<int32_t(std::string)>& fn)
    {
        JAVA_OUTPUT << "string_to_int_function(" << val << ")" << std::endl;
        return fn(val);
    }

    static int64_t string_to_long_function(const std::string& val, const std::function<int64_t(std::string)>& fn)
    {
        JAVA_OUTPUT << "string_to_long_function(" << val << ")" << std::endl;
        return fn(val);
    }

    static double string_to_double_function(const std::string& val, const std::function<double(std::string)>& fn)
    {
        JAVA_OUTPUT << "string_to_double_function(" << val << ")" << std::endl;
        return fn(val);
    }

    static Rectangle pass_record(const Rectangle& rect)
    {
        JAVA_OUTPUT << "pass_record({" << rect.width << ", " << rect.height << "})" << std::endl;
        return Rectangle(2 * rect.width, 2 * rect.height);
    }
};

struct Residence
{
    std::string country;
    std::string city;
};

class Person
{
    std::string name;
    Residence residence;
public:
    Person() = default;
    Person(const std::string& n) : name(n) {}
    Person(const std::string& n, const Residence& r) : name(n), residence(r) {}
    Residence get_residence() const { return residence; }
    void set_residence(const Residence& r) { residence = r; }
};

DECLARE_NATIVE_CLASS(Sample, "hu.info.hunyadi.test.Sample");
DECLARE_RECORD_CLASS(Rectangle, "hu.info.hunyadi.test.Rectangle");
DECLARE_STATIC_CLASS(StaticSample, "hu.info.hunyadi.test.StaticSample");

DECLARE_NATIVE_CLASS(Person, "hu.info.hunyadi.test.Person");
DECLARE_RECORD_CLASS(Residence, "hu.info.hunyadi.test.Residence");

JAVA_EXTENSION_MODULE()
{
    using namespace javabind;

    record_class<Rectangle>()
        .field<&Rectangle::width>("width")
        .field<&Rectangle::height>("height")
        ;

    native_class<Sample>()
        .constructor<Sample()>("create")
        .function<Sample::returns_void>("returns_void")
        .function<Sample::returns_bool>("returns_bool")
        .function<Sample::returns_int>("returns_int")
        .function<&Sample::value>("value")
        .function < &Sample::operator+=>("add")
        ;

    static_class<StaticSample>()
        // fundamental types and simple well-known types as return values
        .function<returns_void>("returns_void")
        .function<StaticSample::returns_bool>("returns_bool")
        .function<StaticSample::returns_int>("returns_int")
        .function<StaticSample::returns_string>("returns_string")

        // fundamental types and simple well-known types as arguments
        .function<StaticSample::pass_bool>("pass_bool")
        .function<StaticSample::pass_byte>("pass_byte")
        .function<StaticSample::pass_short>("pass_short")
        .function<StaticSample::pass_int>("pass_int")
        .function<StaticSample::pass_long>("pass_long")
        .function<StaticSample::pass_float>("pass_float")
        .function<StaticSample::pass_double>("pass_double")
        .function<StaticSample::pass_string>("pass_string")

        // boxing and unboxing
        .function<StaticSample::pass_boxed_boolean>("pass_boxed_boolean")
        .function<StaticSample::pass_boxed_integer>("pass_boxed_integer")

        // arrays as arguments and return values
        .function<StaticSample::pass_bool_array>("pass_bool_array")
        .function<StaticSample::pass_byte_array>("pass_byte_array")
        .function<StaticSample::pass_short_array>("pass_short_array")
        .function<StaticSample::pass_int_array>("pass_int_array")
        .function<StaticSample::pass_long_array>("pass_long_array")
        .function<StaticSample::pass_float_array>("pass_float_array")
        .function<StaticSample::pass_double_array>("pass_double_array")

        // functional interface
        .function<StaticSample::pass_function>("pass_function")
        .function<StaticSample::int_to_string_function>("int_to_string_function")
        .function<StaticSample::long_to_string_function>("long_to_string_function")
        .function<StaticSample::double_to_string_function>("double_to_string_function")
        .function<StaticSample::string_to_int_function>("string_to_int_function")
        .function<StaticSample::string_to_long_function>("string_to_long_function")
        .function<StaticSample::string_to_double_function>("string_to_double_function")

        // record class
        .function<StaticSample::pass_record>("pass_record")
        ;

    native_class<Person>()
        .constructor<Person(std::string)>("create")
        .constructor<Person(std::string, Residence)>("create")
        .function<&Person::get_residence>("getResidence")
        .function<&Person::set_residence>("setResidence")
        ;

    record_class<Residence>()
        .field<&Residence::country>("country")
        .field<&Residence::city>("city")
        ;
}
