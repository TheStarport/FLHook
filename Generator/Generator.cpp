// Copyright (C) 2017-2019 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.
// 
#include <windows.h>
#undef min
#undef max

#include <iostream>
#include <filesystem>

#include <cxxopts.hpp>

#include <cppast/code_generator.hpp>         // for generate_code()
#include <cppast/cpp_entity_kind.hpp>        // for the cpp_entity_kind definition
#include <cppast/cpp_forward_declarable.hpp> // for is_definition()
#include <cppast/cpp_namespace.hpp>          // for cpp_namespace
#include <cppast/libclang_parser.hpp> // for libclang_parser, libclang_compile_config, cpp_entity,...
#include <cppast/visitor.hpp>         // for visit()

class win_logger : public cppast::diagnostic_logger {
public:
    using diagnostic_logger::diagnostic_logger;

private:
    mutable char buf[8192];
    bool do_log(const char *source, const cppast::diagnostic &d) const override {
        auto loc = d.location.to_string();
        if (loc.empty())
            sprintf_s(buf, "[%s] [%s] %s\n", source, to_string(d.severity), d.message.c_str());
        else
            sprintf_s(buf, "[%s] [%s] %s %s\n", source, to_string(d.severity),
                         d.location.to_string().c_str(), d.message.c_str());

        OutputDebugStringA(buf);
        return true;
    }
};

// parse a file
std::unique_ptr<cppast::cpp_file> parse_file(const cppast::libclang_compile_config& config,
                                             const cppast::diagnostic_logger&       logger,
                                             std::string filename, bool fatal_error)
{
    // the entity index is used to resolve cross references in the AST
    // we don't need that, so it will not be needed afterwards
    cppast::cpp_entity_index idx;
    // the parser is used to parse the entity
    // there can be multiple parser implementations
    cppast::libclang_parser parser(type_safe::ref(logger));
    // parse the file
    auto file = parser.parse(idx, filename, config);
    if (fatal_error && parser.error())
        return nullptr;
    return file;
}

std::string replace_backslashes(std::string s) {
    std::replace(s.begin(), s.end(), '\\', '/');
    return s;
}

void generate_hooks(const cppast::cpp_entity& e) {
    
}

void locate_hooks(const cppast::cpp_entity& base) {
    cppast::visit(base, [](const cppast::cpp_entity& entity, cppast::visitor_info) {
        if(entity.kind() == cppast::cpp_entity_kind::class_t || entity.kind() == cppast::cpp_entity_kind::function_t) {
            if(const auto& att = cppast::has_attribute(entity.attributes(), "Hook"))
                generate_hooks(entity);
        } else if(entity.kind() == cppast::cpp_entity_kind::namespace_t)
            locate_hooks(entity);
    });
}

int main(int argc, char* argv[])
{
    if(argc != 2)
        return 1;

    std::filesystem::path sln_dir = argv[1];

    cppast::libclang_compile_config config;
    config.set_flags(cppast::cpp_standard::cpp_latest, cppast::compile_flag::ms_extensions | cppast::compile_flag::ms_compatibility);
    config.define_macro("ST6_ALLOCATION_DEFINED", "1");
    config.add_include_dir(replace_backslashes((sln_dir / "..\\FLHookSDK\\include").lexically_normal().string()));

    // the logger is used to print diagnostics
    win_logger logger;

    auto file = parse_file(config, logger, replace_backslashes((sln_dir / "..\\FLHookSDK\\include\\FLCoreRemoteClient.h").lexically_normal().string()), false);
    locate_hooks(*file.get());
}