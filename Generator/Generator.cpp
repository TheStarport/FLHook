// Copyright (C) 2017-2019 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.
// 
#include <windows.h>
#undef min
#undef max

#include <iostream>
#include <filesystem>
#include <fstream>
#include <ranges>

#include <cxxopts.hpp>

#include <cppast/code_generator.hpp>         // for generate_code()
#include <cppast/cpp_entity_kind.hpp>        // for the cpp_entity_kind definition
#include <cppast/cpp_forward_declarable.hpp> // for is_definition()
#include <cppast/cpp_member_function.hpp>
#include <cppast/cpp_namespace.hpp>          // for cpp_namespace
#include <cppast/cpp_type_alias.hpp>
#include <cppast/libclang_parser.hpp> // for libclang_parser, libclang_compile_config, cpp_entity,...
#include <cppast/visitor.hpp>         // for visit()

std::string replace_backslashes(std::string s) {
    std::replace(s.begin(), s.end(), '\\', '/');
    return s;
}

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

class parser {
    std::ofstream hook_src_, hook_header_, sdk_header_;
    std::stringstream hooked_call_enum_;
    
    cppast::libclang_compile_config config_;
        
    // the entity index is used to resolve cross references in the AST
    cppast::cpp_entity_index idx_;

    // the logger is used to print diagnostics
    win_logger logger_;

    std::vector<std::unique_ptr<cppast::cpp_file>> files_;

    enum class type_information {
        unknown,
        is_float,
        is_integral,
        is_unsigned,
        is_string,
        is_pointer
    };

    struct parsed_arg {
        const std::string type;
        const std::string name;
        type_information info;
    };

    const cppast::cpp_type& walk_typedefs(const cppast::cpp_type& base_type) const {
        if(base_type.kind() != cppast::cpp_type_kind::user_defined_t)
            return base_type;

        const auto& user_type = dynamic_cast<const cppast::cpp_user_defined_type&>(base_type);
        const auto& type_entity = user_type.entity().get(idx_)[0u].get();
        if(type_entity.kind() == cppast::cpp_entity_kind::type_alias_t) {
            const auto& type_alias = dynamic_cast<const cppast::cpp_type_alias&>(type_entity);
            const auto& underlying_type = type_alias.underlying_type();
            return walk_typedefs(underlying_type);
        }

        return base_type;
    }

    type_information determine_type_information(const cppast::cpp_type& in_type) const {
        const auto& type = walk_typedefs(in_type);
        if(type.kind() == cppast::cpp_type_kind::pointer_t) {
            const auto& ptr = static_cast<const cppast::cpp_pointer_type&>(type);
            if(ptr.pointee().kind() == cppast::cpp_type_kind::builtin_t) {
                const auto& builtin = static_cast<const cppast::cpp_builtin_type&>(ptr.pointee());
                if(builtin.builtin_type_kind() == cppast::cpp_char)
                    return type_information::is_string;
            }
            return type_information::is_pointer;
        }

        if(type.kind() == cppast::cpp_type_kind::builtin_t) {
            const auto& builtin = static_cast<const cppast::cpp_builtin_type&>(type);
            
            switch (builtin.builtin_type_kind()) {
            using namespace cppast;
            case cpp_bool:
            case cpp_schar:
            case cpp_short:
            case cpp_int:
            case cpp_long:
            case cpp_longlong:
                return type_information::is_integral;
            case cpp_uchar:
            case cpp_ushort:
            case cpp_uint:
            case cpp_ulong:
            case cpp_ulonglong:
                return type_information::is_unsigned;
            case cpp_float:
            case cpp_double:
                return type_information::is_float;
            default:
                return type_information::unknown;
            }
        }

        return type_information::unknown;
    }

    template<std::ranges::range T>
    std::vector<parsed_arg> parse_args(const T& params) {
        size_t gen_idx = 0;
        std::vector<parsed_arg> args;
        args.reserve(std::distance(std::ranges::begin(params), std::ranges::end(params)));
        for(const auto& arg : params) {
            std::string arg_name = arg.name().empty() ? ("_genArg" + std::to_string(++gen_idx)) : arg.name();
            args.push_back({ cppast::to_string(arg.type()), arg_name, determine_type_information(arg.type()) });
        }

        return args;
    }

    static void append_args_list(const std::vector<parsed_arg>& args, bool incl_type, std::ostream& output) {
        if(args.empty())
            return;

        auto append = [&](const auto& a, bool trail_comma = true) {
            if(incl_type)
                output << a.type << " ";
            output << a.name;
            if(trail_comma) output << ", ";
        };

        std::ranges::for_each(args | std::ranges::views::take(args.size() - 1), append);
        append(args.back(), false);
    }

    static std::string make_context_func_name(const std::string& context, const std::string& func_name) {
        std::string enum_context = context;
        std::replace(enum_context.begin(), enum_context.end(), ':', '_');

        return enum_context + func_name;
    }

    static std::string make_enum(const std::string& context, const std::string& func_name) {
        std::string enum_context = context;
        std::replace(enum_context.begin(), enum_context.end(), ':', '_');

        for(auto p = enum_context.find("Hk"); p != std::string::npos; p = enum_context.find("Hk")) {
            enum_context = enum_context.substr(0, p) + enum_context.substr(p + 2);
        }

        return enum_context + func_name;
    }

    void generate_function_hook(const cppast::cpp_entity& entity, const std::string& context, bool client_call, bool server_call) {

        // Never hook operator methods
        if(entity.name().starts_with("operator"))
            return;

        const auto& func = static_cast<const cppast::cpp_function&>(entity);
        std::string func_name = server_call ? func.name() : context + func.name();
        std::string enum_val = make_enum(context, func.name());
        std::string full_enum_val = "HookedCall::" + enum_val;

        bool void_return = func.return_type().kind() == cppast::cpp_type_kind::builtin_t && static_cast<const cppast::cpp_builtin_type&>(func.return_type()).builtin_type_kind() == cppast::cpp_void;
        
        bool no_plugins = has_attribute(entity, "NoPlugins").has_value();
        bool no_log = has_attribute(entity, "NoLog").has_value();
        auto call_inner = has_attribute(entity, "CallInner");
        bool call_inner_after = has_attribute(entity, "CallInnerAfter").has_value();
        bool dc_check = has_attribute(entity, "DisconnectCheck").has_value();
        auto semaphore = has_attribute(entity, "Semaphore");

        if(!no_plugins)
            hooked_call_enum_ << "\t" << enum_val << "," << std::endl;

        auto args = parse_args(func.parameters());

        if(server_call && !context.empty())
            hook_src_ << "namespace " << context.substr(0, context.length() - 2) << " {" << std::endl;

        hook_src_ << to_string(func.return_type()) << " ";
        if(server_call)
            hook_src_ << "__stdcall ";
        hook_src_ << func_name << "(";
        append_args_list(args, true, hook_src_);
        hook_src_ << ") {" << std::endl;

        if(!no_log) {
            hook_src_ << "\tAddDebugLog(\"" << func_name << "(";
            if(!args.empty())
                hook_src_ << "\\n";
            for(auto& a : args) {
                hook_src_ << "\\t" << a.type << " " << a.name << " = %";
                switch(a.info) {
                case type_information::is_integral:
                    hook_src_ << "d";
                    break;
                case type_information::is_unsigned:
                    hook_src_ << "u";
                    break;
                case type_information::is_float:
                    hook_src_ << "f";
                    break;
                case type_information::is_pointer:
                    hook_src_ << "p";
                    break;
                case type_information::is_string:
                default:
                    hook_src_ << "s";
                    break;
                }

                hook_src_ << "\\n";
            }
            if(!args.empty()) {
                hook_src_ << ")\"," << std::endl << "\t\t\t";
                append_args_list(args, false, hook_src_);
            } else
                hook_src_ << ")\"";
            hook_src_ << ");" << std::endl << std::endl;
        }

        if(!no_plugins) {
            const char* retval = void_return ? "\tauto skip" : "\tauto [retVal, skip]";
            hook_src_ << retval << " = CallPluginsBefore<" << to_string(func.return_type()) << ">(" << full_enum_val;
            if(!args.empty()) {
                hook_src_ << ",\n\t\t\t";
                append_args_list(args, false, hook_src_);
            }
            hook_src_ << ");" << std::endl << std::endl;
        }

        if(dc_check)
            hook_src_ << "\tCHECK_FOR_DISCONNECT;" << std::endl << std::endl;

        if(call_inner) {
            hook_src_ << "\t";
            auto call_inner_args = call_inner.value().arguments();
            bool can_break = call_inner_args && call_inner_args.value().front().spelling == "true";
            if(can_break)
                hook_src_ << "bool innerCheck = ";
            hook_src_ << make_context_func_name(context, func.name()) << "__Inner(";
            append_args_list(args, false, hook_src_);
            hook_src_ << ");" << std::endl;

            if(can_break) {
                hook_src_ << "\tif(!innerCheck) return";
                if(!void_return)
                    hook_src_ << " " << to_string(func.return_type()) << "()";
                hook_src_ << ";";
            }

            hook_src_ << std::endl;
        }

        if(semaphore) {
            auto semaphore_name = semaphore.value().arguments().value().front().spelling;

            hook_src_ << "\t" << semaphore_name << " = true;" << std::endl;
        }

        hook_src_ << "\t";
        if(!no_plugins) {
            hook_src_ << "if(!skip) ";
            if(!void_return)
                hook_src_ << "retVal = ";
        } else if(!void_return)
            hook_src_ << "auto retVal = ";

        if(client_call) {
            hook_src_ << "CALL_CLIENT_METHOD(" << func.name() << "(";
            append_args_list(args, false, hook_src_);
            hook_src_ << "));" << std::endl;
        } else if(server_call) {
            hook_src_ << "EXECUTE_SERVER_CALL(Server." << func.name() << "(";
            append_args_list(args, false, hook_src_);
            hook_src_ << "));" << std::endl;
        }

        if(semaphore) {
            auto semaphore_name = semaphore.value().arguments().value().front().spelling;

            hook_src_ << "\t" << semaphore_name << " = false;" << std::endl;
        }

        if(call_inner_after) {
            hook_src_ << "\t" << make_context_func_name(context, func.name()) << "__InnerAfter(";
            append_args_list(args, false, hook_src_);
            hook_src_ << ");" << std::endl << std::endl;
        }

        if(!no_plugins) {
            hook_src_ << std::endl << "\tCallPluginsAfter(" << full_enum_val;
            if(!args.empty()) {
                hook_src_ << ",\n\t\t\t";
                append_args_list(args, false, hook_src_);
            }
            hook_src_ << ");" << std::endl;
            
            if(!void_return)
                hook_src_ << std::endl << "\treturn retVal;" << std::endl;
        } else if(!void_return)
            hook_src_ << "\treturn retVal;" << std::endl;

        hook_src_ << "}" << std::endl;
        
        if(server_call && !context.empty())
            hook_src_ << "}" << std::endl;

        hook_src_ << std::endl;
    }

    void generate_hooks(const cppast::cpp_entity& e, const std::string& context, bool client_call, bool server_call) {
        int32_t func_index = 0;
        cppast::visit(e, [&](const cppast::cpp_entity& entity, cppast::visitor_info) {
            if(entity.kind() == cppast::cpp_entity_kind::member_function_t) {
                bool no_hook = cppast::has_attribute(entity.attributes(), "NoHook").has_value();
                if(!no_hook)
                    generate_function_hook(entity, context + (server_call ? "Hk" : "") + e.name() + "::", client_call, server_call);

                func_index++;
            }
        });
    }

public:
    explicit parser(const std::filesystem::path& sln_dir) : logger_(true) {
        config_.set_flags(cppast::cpp_standard::cpp_latest, cppast::compile_flag::ms_extensions | cppast::compile_flag::ms_compatibility);
        config_.define_macro("ST6_ALLOCATION_DEFINED", "1");
        config_.define_macro("_X86_", "1");
        config_.define_macro("IMPORT", " ");
        config_.define_macro("EXPORT", " ");
        config_.add_include_dir(replace_backslashes((sln_dir / R"(..\FLHookSDK\include)").lexically_normal().string()));
        
        std::filesystem::path gen_hook_src = (sln_dir / "../source/__generated.cpp").lexically_normal();
        std::filesystem::path gen_sdk_header = (sln_dir / "../FLHookSDK/include/__generated.h").lexically_normal();

        hook_src_.open(gen_hook_src.c_str());
        hook_src_ << "//\n// WARNING: THIS IS AN AUTO-GENERATED FILE, DO NOT EDIT!\n//" << std::endl << std::endl;
        hook_src_ << "#include <Hook.h>" << std::endl << std::endl;

        sdk_header_.open(gen_sdk_header.c_str());
        sdk_header_ << "#pragma once\n\n//\n// WARNING: THIS IS AN AUTO-GENERATED FILE, DO NOT EDIT!\n//" << std::endl << std::endl;

        hooked_call_enum_ << "enum class HookedCall {" << std::endl;
    }

    cppast::cpp_file& parse_file(std::string filename, bool fatal_error)
    {
        // the parser is used to parse the entity
        // there can be multiple parser implementations
        cppast::libclang_parser parser(type_safe::ref(logger_));
        // parse the file
        auto file = parser.parse(idx_, filename, config_);
        if (fatal_error && parser.error())
            throw std::invalid_argument("Parser fatal error.");

        files_.push_back(std::move(file));

        return *files_.back();
    }

    void locate_hooks(const cppast::cpp_entity& base, const std::string& context) {
        cppast::visit(base, [&](const cppast::cpp_entity& entity, cppast::visitor_info vi) -> bool {
            if(&entity == &base)
                return cppast::visitor_result::continue_visit;
            if(vi.event == cppast::visitor_info::container_entity_exit)
                return cppast::visitor_result::continue_visit;

            OutputDebugStringA((entity.name() + "\n").c_str());

            switch(entity.kind()) {
            case cppast::cpp_entity_kind::class_t:
            case cppast::cpp_entity_kind::base_class_t:
                if(cppast::has_attribute(entity.attributes(), "Hook").has_value()) {
                    bool client_call = cppast::has_attribute(entity.attributes(), "ClientCall").has_value();
                    bool server_call = cppast::has_attribute(entity.attributes(), "ServerCall").has_value();
                    generate_hooks(entity, context, client_call, server_call);

                    return cppast::visitor_result::continue_visit_no_children;
                }
                break;
            case cppast::cpp_entity_kind::namespace_t:
                locate_hooks(entity, context + entity.name() + "::");
                break;
            default:
                break;
            }

            return cppast::visitor_result::continue_visit;
        });
    }

    ~parser() {
        hook_src_ << "};" << std::endl;
        hooked_call_enum_ << "\tCount" << std::endl << "};";

        sdk_header_ << hooked_call_enum_.str();
    }
};

int main(int argc, char* argv[])
{
    if(argc != 2)
        return 1;

    try {
        std::filesystem::path sln_dir = argv[1];

        parser parse(sln_dir);

        auto core_defs_path = replace_backslashes((sln_dir / R"(..\FLHookSDK\include\FLCoreDefs.h)").lexically_normal().string());
        parse.parse_file(core_defs_path, false);

        auto remote_client_path = replace_backslashes((sln_dir / R"(..\FLHookSDK\include\FLCoreRemoteClient.h)").lexically_normal().string());
        auto& remote_client = parse.parse_file(remote_client_path, false);
        parse.locate_hooks(remote_client, "");

        auto server_path = replace_backslashes((sln_dir / R"(..\FLHookSDK\include\FLCoreServer.h)").lexically_normal().string());
        auto& server = parse.parse_file(server_path, false);
        parse.locate_hooks(server, "");
    } catch(...) {
        return 1;
    }
}