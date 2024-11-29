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
#include "export.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>

namespace javabind
{
    template<typename ContentWriter>
    static void write_class(const std::filesystem::path& output_dir, const ClassDescription& class_desc, ContentWriter writer)
    {
        std::filesystem::path output_path = output_dir / class_desc.package_path;
        std::filesystem::path output_filename = output_path / (class_desc.name + ".java");
        std::filesystem::create_directories(output_path);
        std::ofstream os{ output_filename };

        if (!os) {
            std::cerr << "Failed to open file: " << output_filename << std::endl;
            return;
        }
        os << "package " << class_desc.package_name << ";\n";
        os << "\n";
        writer(os, class_desc.name);
        os.close();
    }

    void codegen(const std::filesystem::path& output_dir)
    {
        for (const auto& [enum_class_name, bindings] : javabind::EnumBindings::value) {
            write_class(
                output_dir,
                ClassDescription::from_full_name(enum_class_name),
                [&bindings](auto& stream, const auto& class_name) {
                    write_enum_class(stream, class_name, bindings);
                }
            );
        }
        for (const auto& [record_class_sig, bindings] : javabind::FieldBindings::value) {
            write_class(
                output_dir,
                ClassDescription::from_signature(record_class_sig),
                [&bindings](auto& stream, const auto& class_name) {
                    write_record_class(stream, class_name, bindings);
                }
            );
        }
        for (const auto& [native_class_name, bindings] : javabind::FunctionBindings::value) {
            write_class(
                output_dir,
                ClassDescription::from_full_name(native_class_name),
                [&bindings](auto& stream, const auto& class_name) {
                    write_native_class(stream, class_name, bindings);
                }
            );
        }
    }
}

#ifndef __has_attribute
#define __has_attribute(x) 0
#endif

#if (defined(__GNUC__) && ((__GNUC__ > 4) || (__GNUC__ == 4) && (__GNUC_MINOR__ > 2))) || __has_attribute(visibility)
#define JAVABIND_EXPORT __attribute__((visibility("default")))
#define JAVABIND_IMPORT __attribute__((visibility("default")))
#elif defined(_MSC_VER)
#define JAVABIND_EXPORT __declspec(dllexport)
#define JAVABIND_IMPORT __declspec(dllimport)
#else
#define JAVABIND_EXPORT
#define JAVABIND_IMPORT
#endif

#define JAVA_EXTENSION_EXPORT() \
    extern "C" { JAVABIND_EXPORT void java_bindings_emit_signatures(const char* output_dir); } \
    void java_bindings_emit_signatures(const char* output_dir) { java_bindings_initializer(); javabind::codegen(output_dir); }

#define JAVA_EXTENSION_IMPORT() \
    extern "C" { JAVABIND_IMPORT void java_bindings_emit_signatures(const char* output_dir); }
