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
#include "binding.hpp"
#include <iostream>

namespace javabind
{
    namespace detail
    {
        static inline constexpr std::string_view indent = "    ";
    }

    struct ClassDescription
    {
        static std::tuple<std::string_view, std::string_view> split_on_last_char(std::string_view str, char ch)
        {
            auto pos = str.rfind(ch);
            if (pos == std::string::npos) {
                return std::make_tuple("", str);
            }
            return std::make_tuple(str.substr(0, pos), str.substr(pos + 1));
        }

        static std::string replace(std::string_view str, char old_value, char new_value)
        {
            std::string result{ str };
            std::replace(result.begin(), result.end(), old_value, new_value);
            return result;
        }

        static ClassDescription from_full_name(std::string_view full_name)
        {
            auto [package_name, name] = split_on_last_char(full_name, '.');
            return ClassDescription{ name, package_name, replace(package_name, '.', '/') };
        }

        static ClassDescription from_full_path(std::string_view full_path)
        {
            auto [package_path, name] = split_on_last_char(full_path, '/');
            return ClassDescription{ name, replace(package_path, '/', '.'), package_path };
        }

        static ClassDescription from_signature(std::string_view signature)
        {
            return from_full_path(signature.substr(1, signature.size() - 2));
        }

        ClassDescription(std::string_view name, std::string_view package_name, std::string_view package_path)
            : name{ name }
            , package_name{ package_name }
            , package_path{ package_path }
        {
        }

        std::string name;
        std::string package_name;
        std::string package_path;
    };

    /** Generates Java native signatures for enum class types. */
    static void write_enum_class(std::ostream& os, std::string_view class_name, const javabind::EnumBinding& binding)
    {
        os << "public enum " << class_name << " {\n";

        for (std::size_t i = 0; i < binding.names().size(); ++i) {
            os << detail::indent << binding.names().at(i);
            if (i != binding.names().size() - 1) os << ",";
            os << "\n";
        }
        os << "}\n";
    }

    /** Generates Java native signatures for record class types. */
    static void write_record_class(std::ostream& os, std::string_view class_name, const std::vector<javabind::FieldBinding>& bindings)
    {
        os << "public record " << class_name << "(\n";

        for (std::size_t i = 0; i < bindings.size(); ++i) {
            os << detail::indent << bindings.at(i).type << " " << bindings.at(i).name;
            if (i != bindings.size() - 1) os << ",\n";
        }
        os << ") {\n";
        os << "}\n";
    }

    /** Generates Java native signatures for regular class types. */
    static void write_native_class(std::ostream& os, std::string_view class_name, const std::vector<javabind::FunctionBinding>& bindings)
    {
        auto is_close_method =
            [](const auto& binding)
            {
                return binding.name == "close" && binding.signature == FunctionTraits<void()>::sig;
            };
        auto extends_native_object = std::any_of(bindings.begin(), bindings.end(), is_close_method);

        if (extends_native_object) {
            os << "public class " << class_name << " extends hu.info.hunyadi.javabind.NativeObject {\n";
        } else {
            os << "public class " << class_name << " {\n";
        }

        for (auto&& binding : bindings) {
            if (!binding.is_member) {
                os << detail::indent << "public static native " << binding.return_display << " " << binding.name << "(" << binding.param_display << ");\n";
            }
        }
        for (auto&& binding : bindings) {
            if (binding.is_member) {
                os << detail::indent << "public native " << binding.return_display << " " << binding.name << "(" << binding.param_display << ");\n";
            }
        }
        os << "}\n";
    }
}
