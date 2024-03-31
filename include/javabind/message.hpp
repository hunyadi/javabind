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
#include <sstream>
#include <string>

namespace javabind
{
    /**
     * Allows a friendly message to be built with the stream insertion operator.
     *
     * Example: throw std::runtime_error(msg() << "Error: " << code);
     */
    struct msg
    {
        template<typename T>
        msg& operator<<(const T& part) {
            str << part;
            return *this;
        }

        template<typename T>
        msg& operator<<(T&& part) {
            str << part;
            return *this;
        }

        operator std::string() const {
            return str.str();
        }

    private:
        std::ostringstream str;
    };
}
