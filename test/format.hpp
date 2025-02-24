/**
 * javabind: effective C++ and Java interoperability
 * @see https://github.com/hunyadi/javabind
 *
 * Copyright (c) 2024 Levente Hunyadi
 *
 * This work is licensed under the terms of the MIT license.
 * For a copy, see <https://opensource.org/licenses/MIT>.
 */

#pragma once
#include <chrono>
#include <optional>
#include <vector>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>
#include <ctime>
#include <iostream>

inline bool time_to_struct(const time_t* timer, struct tm* buf)
{
#if defined(_MSC_VER)
    return gmtime_s(buf, timer) == 0;
#else
    return gmtime_r(timer, buf) != nullptr;
#endif
}

inline std::string to_string(const std::chrono::system_clock::time_point& instant)
{
    auto duration_s = std::chrono::duration_cast<std::chrono::seconds>(instant.time_since_epoch());
    auto duration_ns = std::chrono::duration_cast<std::chrono::nanoseconds>((instant - duration_s).time_since_epoch());

    if (duration_ns.count() < 0) {
        // ensure nanoseconds part is always non-negative
        duration_s -= std::chrono::seconds(1);
        duration_ns += std::chrono::nanoseconds(1'000'000'000);
    }

    // use 400-year periodicity of Gregorian calendar
    unsigned int periods = 0;
    while (duration_s.count() < 0) {
        // ensure seconds part is always non-negative
        duration_s += std::chrono::hours(24 * 146'097);
        ++periods;
    }

    unsigned long long ns = duration_ns.count();
    std::time_t tv = static_cast<std::time_t>(duration_s.count());
    std::tm tp;
    if (!time_to_struct(&tv, &tp)) {
        return std::string("[ERROR]");
    }

    // 1984-01-01 01:02:03.123456789Z
    char buf[64];
    int n = std::snprintf(buf, sizeof(buf), "%.4d-%02u-%02u %02u:%02u:%02u.%09lluZ",
        tp.tm_year + 1900 - 400 * periods,
        tp.tm_mon + 1,
        tp.tm_mday,
        tp.tm_hour,
        tp.tm_min,
        tp.tm_sec,
        ns
    );
    return std::string(buf, buf + n);
}

inline std::ostream& operator<<(std::ostream& os, char16_t ch)
{
    char fill = os.fill();
    std::ios_base::fmtflags flags = os.flags();
    std::streamsize width = os.width();
    os.fill('0');
    os.width(2);
    os << std::hex << (static_cast<unsigned>(ch) >> 8) << (static_cast<unsigned>(ch) & 0xff);
    os.fill(fill);
    os.flags(flags);
    os.width(width);
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const std::chrono::nanoseconds& ns)
{
    return os << ns.count() << "ns";
}

inline std::ostream& operator<<(std::ostream& os, const std::chrono::microseconds& us)
{
    return os << us.count() << "us";
}

inline std::ostream& operator<<(std::ostream& os, const std::chrono::milliseconds& ms)
{
    return os << ms.count() << "ms";
}

inline std::ostream& operator<<(std::ostream& os, const std::chrono::seconds& s)
{
    return os << s.count() << "s";
}

inline std::ostream& operator<<(std::ostream& os, const std::chrono::minutes& m)
{
    return os << m.count() << "m";
}

inline std::ostream& operator<<(std::ostream& os, const std::chrono::hours& h)
{
    return os << h.count() << "h";
}

inline std::ostream& operator<<(std::ostream& os, const std::chrono::system_clock::time_point& instant)
{
    return os << to_string(instant);
}

template <typename T>
std::ostream& operator<<(std::ostream& os, const std::optional<T>& opt)
{
    if (opt.has_value()) {
        return os << "{" << opt.value() << "}";
    } else {
        return os << "nullopt";
    }
}

template <typename K, typename V>
std::ostream& operator<<(std::ostream& os, const std::pair<K, V>& pair)
{
    return os << pair.first << ": " << pair.second;
}

template <typename L>
std::ostream& write_collection(std::ostream& os, const L& list, char left, char right)
{
    os << left;
    if (!list.empty()) {
        auto&& it = list.begin();
        os << *it;

        for (++it; it != list.end(); ++it) {
            os << ", " << *it;
        }
    }
    os << right;
    return os;
}

template <typename L>
std::ostream& write_list(std::ostream& os, const L& list)
{
    return write_collection(os, list, '[', ']');
}

template <typename S>
std::ostream& write_set(std::ostream& os, const S& set)
{
    return write_collection(os, set, '{', '}');
}

template <typename... Args>
std::ostream& operator<<(std::ostream& os, const std::vector<Args...>& list)
{
    return write_list(os, list);
}

template <typename... Args>
std::ostream& operator<<(std::ostream& os, const std::set<Args...>& set)
{
    return write_set(os, set);
}

template <typename... Args>
std::ostream& operator<<(std::ostream& os, const std::unordered_set<Args...>& set)
{
    return write_set(os, set);
}

template <typename... Args>
std::ostream& operator<<(std::ostream& os, const std::map<Args...>& set)
{
    return write_set(os, set);
}

template <typename... Args>
std::ostream& operator<<(std::ostream& os, const std::unordered_map<Args...>& set)
{
    return write_set(os, set);
}
