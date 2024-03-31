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

struct Sample
{
    Sample() = default;

    static void returns_void()
    {
        JAVA_OUTPUT << "returns_void()" << std::endl;
    }

    static bool returns_bool()
    {
        JAVA_OUTPUT << "returns_bool()" << std::endl;
        return true;
    }

    static int returns_int()
    {
        JAVA_OUTPUT << "returns_int()" << std::endl;
        return 82;
    }

    int value() const
    {
        return _value;
    }

    void add(int val)
    {
        _value += val;
    }

private:
    int _value = 0;
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

    static int returns_int()
    {
        JAVA_OUTPUT << "returns_int()" << std::endl;
        return 82;
    }

    static int64_t returns_long()
    {
        JAVA_OUTPUT << "returns_long()" << std::endl;
        return 1982102320240331;
    }

    static float returns_float()
    {
        JAVA_OUTPUT << "returns_float()" << std::endl;
        return std::numeric_limits<float>::max();
    }

    static double returns_double()
    {
        JAVA_OUTPUT << "returns_double()" << std::endl;
        return std::numeric_limits<double>::max();
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

    static int pass_int(int value)
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

    static javabind::boxed<int> pass_boxed(javabind::boxed<int> value)
    {
        JAVA_OUTPUT << "pass_boxed(" << value << ")" << std::endl;
        return value;
    }

    static std::vector<bool> pass_bool_array(const std::vector<bool>& values)
    {
        auto result = std::vector<bool>(values.begin(), values.end());
        JAVA_OUTPUT << "pass_bool_array(" << result << ")" << std::endl;
        return result;
    }

    static std::vector<int> pass_int_array(const std::vector<int>& values)
    {
        auto result = std::vector<int>(values.begin(), values.end());
        JAVA_OUTPUT << "pass_int_array(" << result << ")" << std::endl;
        return result;
    }

    static std::vector<int64_t> pass_long_array(const std::vector<int64_t>& values)
    {
        auto result = std::vector<int64_t>(values.begin(), values.end());
        JAVA_OUTPUT << "pass_long_array(" << result << ")" << std::endl;
        return result;
    }

    static std::vector<float> pass_float_array(const std::vector<float>& values)
    {
        auto result = std::vector<float>(values.begin(), values.end());
        JAVA_OUTPUT << "pass_float_array(" << result << ")" << std::endl;
        return result;
    }

    static std::vector<double> pass_double_array(const std::vector<double>& values)
    {
        auto result = std::vector<double>(values.begin(), values.end());
        JAVA_OUTPUT << "pass_double_array(" << result << ")" << std::endl;
        return result;
    }

    static std::string pass_function(const std::string& str, const std::function<std::string(std::string)>& fn)
    {
        JAVA_OUTPUT << "pass_function(" << str << ")" << std::endl;
        return str + " -> " + fn(str);
    }
};

DECLARE_NATIVE_CLASS(Sample, "hu.info.hunyadi.test.Sample");
DECLARE_STATIC_CLASS(StaticSample, "hu.info.hunyadi.test.StaticSample");

JAVA_EXTENSION_MODULE()
{
    using namespace javabind;

    native_class<Sample>()
        .constructor<Sample()>("create")
        .function<Sample::returns_void>("returns_void")
        .function<Sample::returns_bool>("returns_bool")
        .function<Sample::returns_int>("returns_int")
        .function<&Sample::value>("value")
        .function<&Sample::add>("add")
        ;

    static_class<StaticSample>()
        // fundamental types and simple well-known types as return values
        .function<returns_void>("returns_void")
        .function<StaticSample::returns_bool>("returns_bool")
        .function<StaticSample::returns_int>("returns_int")
        .function<StaticSample::returns_long>("returns_long")
        .function<StaticSample::returns_float>("returns_float")
        .function<StaticSample::returns_double>("returns_double")
        .function<StaticSample::returns_string>("returns_string")

        // fundamental types and simple well-known types as arguments
        .function<StaticSample::pass_bool>("pass_bool")
        .function<StaticSample::pass_int>("pass_int")
        .function<StaticSample::pass_long>("pass_long")
        .function<StaticSample::pass_float>("pass_float")
        .function<StaticSample::pass_double>("pass_double")
        .function<StaticSample::pass_string>("pass_string")
        .function<StaticSample::pass_boxed>("pass_boxed")

        // arrays as arguments and return values
        .function<StaticSample::pass_bool_array>("pass_bool_array")
        .function<StaticSample::pass_int_array>("pass_int_array")
        .function<StaticSample::pass_long_array>("pass_long_array")
        .function<StaticSample::pass_float_array>("pass_float_array")
        .function<StaticSample::pass_double_array>("pass_double_array")

        // functional interface
        .function<StaticSample::pass_function>("pass_function")
        ;
}
