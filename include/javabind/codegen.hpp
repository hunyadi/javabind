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
#include <filesystem>
#include <fstream>
#include <iostream>

namespace javabind
{
    static inline constexpr std::string_view indent = "    ";

    struct ClassDescription
    {
        static std::tuple<std::string_view, std::string_view> split_on_last_char(std::string_view str, char ch)
        {
            auto pos = str.rfind(ch);
            if (pos == std::string::npos) return std::make_tuple("", str);
            return std::make_tuple(str.substr(0, pos), str.substr(pos + 1));
        }

        static std::string replace(std::string_view str, char old_value, char new_value)
        {
            std::string result { str };
            std::replace(result.begin(), result.end(), old_value, new_value);
            return result;
        }

        static ClassDescription from_full_name(std::string_view full_name)
        {
            auto [package_name, name] = split_on_last_char(full_name, '.');
            return ClassDescription { name, package_name, replace(package_name, '.', '/') };
        }

        static ClassDescription from_full_path(std::string_view full_path)
        {
            auto [package_path, name] = split_on_last_char(full_path, '/');
            return ClassDescription { name, replace(package_path, '/', '.'), package_path };
        }

        static ClassDescription from_signature(std::string_view signature)
        {
            return from_full_path(signature.substr(1, signature.size() - 2));
        }

        ClassDescription(std::string_view name, std::string_view package_name, std::string_view package_path)
            : name { name }
            , package_name { package_name }
            , package_path { package_path }
        {
        }

        std::string name;
        std::string package_name;
        std::string package_path;
    };

    static void write_enum_class(std::ostream& os, std::string_view class_name, const javabind::EnumBinding& binding)
    {
        os << "public enum " << class_name << " {\n";

        for (std::size_t i = 0; i < binding.names().size(); ++i)
        {
            os << indent << binding.names().at(i);
            if (i != binding.names().size() - 1) os << ",";
            os << "\n";
        }
        os << "}\n";
    }

    static void write_record_class(std::ostream& os, std::string_view class_name, const std::vector<javabind::FieldBinding>& bindings)
    {
        os << "public record " << class_name << "(\n";

        for (std::size_t i = 0; i < bindings.size(); ++i)
        {
            os << indent << bindings.at(i).type << " " << bindings.at(i).name;
            if (i != bindings.size() - 1) os << ",\n";
        }
        os << ") {\n";
        os << "}\n";
    }

    static void write_native_class(std::ostream& os, std::string_view class_name, const std::vector<javabind::FunctionBinding>& bindings)
    {
        auto is_close_method = [](const auto& binding) { return binding.name == "close" && binding.signature == FunctionTraits<void()>::sig; };
        auto extends_native_object = std::any_of(bindings.begin(), bindings.end(), is_close_method);

        if (extends_native_object)
        {
            os << "import hu.info.hunyadi.javabind.NativeObject;\n";
            os << "\n";
            os << "public class " << class_name << " extends NativeObject {\n";
        }
        else
        {
            os << "public class " << class_name << " {\n";
        }

        for (auto&& binding : bindings)
        {
            if (!binding.is_member)
            {
                os << indent << "public static native " << binding.return_display << " " << binding.name << "(" << binding.param_display << ");\n";
            }
        }
        for (auto&& binding : bindings)
        {
            if (binding.is_member)
            {
                os << indent << "public native " << binding.return_display << " " << binding.name << "(" << binding.param_display << ");\n";
            }
        }
        os << "}\n";
    }

    template<typename ContentWriter>
    static void write_class(const std::filesystem::path& output_dir, const ClassDescription& class_desc, ContentWriter writer)
    {
        std::filesystem::path output_path = output_dir / class_desc.package_path;
        std::filesystem::path output_filename = output_path / (class_desc.name + ".java");
        std::filesystem::create_directories(output_path);
        std::ofstream os { output_filename };

        if (!os)
        {
            std::cerr << "Failed to open file: " << output_filename << std::endl;
            return;
        }
        os << "package " << class_desc.package_name << ";\n";
        os << "\n";
        writer(os, class_desc.name);
        os.close();
    }

    void codegen(const std::filesystem::path& output_path)
    {
        for (const auto& [enum_class_name, bindings] : javabind::EnumBindings::value)
        {
            ClassDescription class_desc = ClassDescription::from_full_name(enum_class_name);
            write_class(output_path, class_desc, [&bindings](auto& stream, const auto& class_name) {
                write_enum_class(stream, class_name, bindings);
            });
        }
        for (const auto& [record_class_sig, bindings] : javabind::FieldBindings::value)
        {
            ClassDescription class_desc = ClassDescription::from_signature(record_class_sig);
            write_class(output_path, class_desc, [&bindings](auto& stream, const auto& class_name) {
                write_record_class(stream, class_name, bindings);
            });
        }
        for (const auto& [native_class_name, bindings] : javabind::FunctionBindings::value)
        {
            ClassDescription class_desc = ClassDescription::from_full_name(native_class_name);
            write_class(output_path, class_desc, [&bindings](auto& stream, const auto& class_name) {
                write_native_class(stream, class_name, bindings);
            });
        }
    }
}