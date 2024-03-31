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
#include <array>
#include <string_view>

namespace javabind
{
    /**
     * Builds a zero-terminated string literal from an std::array.
     * @tparam N The size of the std::array.
     * @tparam I An index sequence, typically constructed with std::make_index_sequence<N>.
     */
    template <std::size_t N, std::array<char, N> const& S, typename I>
    struct to_char_array;

    template <std::size_t N, std::array<char, N> const& S, std::size_t... I>
    struct to_char_array<N, S, std::index_sequence<I...>>
    {
        static constexpr const char value[]{ S[I]..., 0 };
    };

    /**
     * Returns the number of digits in n.
     */
    constexpr std::size_t num_digits(std::size_t n)
    {
        return n < 10 ? 1 : num_digits(n / 10) + 1;
    }

    /**
     * Converts an unsigned integer into sequence of decimal digits.
     */
    template <std::size_t N>
    struct integer_to_digits
    {
    private:
        constexpr static std::size_t len = num_digits(N);

        constexpr static auto impl()
        {
            std::array<char, len> arr{};
            std::size_t n = N;
            std::size_t i = len;
            while (i > 0) {
                --i;
                arr[i] = '0' + (n % 10);
                n /= 10;
            }
            return arr;
        }
        constexpr static auto arr = impl();

    public:
        constexpr static std::string_view value = std::string_view(
            to_char_array< arr.size(), arr, std::make_index_sequence<arr.size()> >::value,
            arr.size()
        );
    };

    /**
     * Replaces all occurrences of a character in a string with another character at compile time.
     * @tparam S The string in which replacements are made.
     * @tparam O The character to look for.
     * @tparam N The character to replace to.
     */
    template <std::string_view const& S, char O, char N>
    class replace
    {
        static constexpr auto impl() noexcept
        {
            std::array<char, S.size()> arr{};
            for (std::size_t i = 0; i < S.size(); ++i) {
                if (S[i] == O) {
                    arr[i] = N;
                }
                else {
                    arr[i] = S[i];
                }
            }
            return arr;
        }

        static constexpr auto arr = impl();

    public:
        static constexpr std::string_view value = std::string_view(
            to_char_array< arr.size(), arr, std::make_index_sequence<arr.size()> >::value,
            arr.size()
        );
    };

    template <std::string_view const& S, char O, char N>
    static constexpr auto replace_v = replace<S, O, N>::value;

    /**
     * Concatenates a list of strings at compile time.
     */
    template <std::string_view const&... Strs>
    class join
    {
        // join all strings into a single std::array of chars
        static constexpr auto impl() noexcept
        {
            constexpr std::size_t len = (Strs.size() + ... + 0);
            std::array<char, len> arr{};
            auto append = [i = 0, &arr](auto const& s) mutable {
                for (auto c : s) {
                    arr[i++] = c;
                }
                };
            (append(Strs), ...);
            return arr;
        }

        // give the joined string static storage
        static constexpr auto arr = impl();

    public:
        // convert to a string literal, then view as a std::string_view
        static constexpr std::string_view value = std::string_view(
            to_char_array< arr.size(), arr, std::make_index_sequence<arr.size()> >::value,
            arr.size()
        );
    };

    template <std::string_view const&... Strs>
    static constexpr auto join_v = join<Strs...>::value;

    /**
     * Concatenates a list of strings at compile time, inserting a separator between neighboring items.
     */
    template <std::string_view const& Sep, std::string_view const&... Items>
    struct join_sep;

    template <std::string_view const& Sep, std::string_view const& Head, std::string_view const&... Tail>
    struct join_sep<Sep, Head, Tail...>
    {
        constexpr static std::string_view value = join<Head, join<Sep, Tail>::value...>::value;
    };

    template <std::string_view const& Sep, std::string_view const& Item>
    struct join_sep<Sep, Item>
    {
        constexpr static std::string_view value = Item;
    };

    template <std::string_view const& Sep>
    struct join_sep<Sep>
    {
        constexpr static std::string_view value = "";
    };

    // helper to extract value
    template <std::string_view const&... items>
    static constexpr auto join_sep_v = join_sep<items...>::value;
}
