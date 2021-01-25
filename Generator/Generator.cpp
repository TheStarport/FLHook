#pragma region Windows header shenanigans
/// These defines are universally accepted as helping to trim down windows.h size
#define WIN32_LEAN_AND_MEAN
#define STRICT

/// These defines all come from windows.h itself and were set to minimize size as much as possible
/// If new usages of the Windows APIs arise, some of these may have to be commented out
#define NOGDICAPMASKS // - CC_*, LC_*, PC_*, CP_*, TC_*, RC_
//#define NOVIRTUALKEYCODES // - VK_*
//#define NOWINMESSAGES // - WM_*, EM_*, LB_*, CB_*
//#define NOWINSTYLES // - WS_*, CS_*, ES_*, LBS_*, SBS_*, CBS_*
//#define NOSYSMETRICS // - SM_*
#define NOMENUS // - MF_*
#define NOICONS // - IDI_*
//#define NOKEYSTATES // - MK_*
#define NOSYSCOMMANDS // - SC_*
#define NORASTEROPS // - Binary and Tertiary raster ops
#define NOSHOWWINDOW // - SW_*
#define OEMRESOURCE // - OEM Resource values
#define NOATOM // - Atom Manager routines
//#define NOCLIPBOARD // - Clipboard routines
//#define NOCOLOR // - Screen colors
//#define NOCTLMGR // - Control and Dialog routines
#define NODRAWTEXT // - DrawText() and DT_*
//#define NOGDI // - All GDI defines and routines
#define NOKERNEL // - All KERNEL defines and routines
//#define NOUSER // - All USER defines and routines
#define NONLS // - All NLS defines and routines
//#define NOMB // - MB_* and MessageBox()
#define NOMEMMGR // - GMEM_*, LMEM_*, GHND, LHND, associated routines
#define NOMETAFILE // - typedef METAFILEPICT
#define NOMINMAX // - Macros min(a, b) and max(a, b)
//#define NOMSG // - typedef MSG and associated routines
#define NOOPENFILE // - OpenFile(), OemToAnsi, AnsiToOem, and OF_*
#define NOSCROLL // - SB_* and scrolling routines
#define NOSERVICE // - All Service Controller routines, SERVICE_ equates, etc.
#define NOSOUND // - Sound driver routines
#define NOTEXTMETRIC // - typedef TEXTMETRIC and associated routines
//#define NOWH // - SetWindowsHook and WH_*
//#define NOWINOFFSETS // - GWL_*, GCL_*, associated routines
#define NOCOMM // - COMM driver routines
#define NOKANJI // - Kanji support stuff.
#define NOHELP // - Help engine interface.
#define NOPROFILER // - Profiler interface.
#define NODEFERWINDOWPOS // - DeferWindowPos routines
#define NOMCX // - Modem Configuration Extensions

#include <windows.h>
#undef DELETE
#pragma endregion

#include <filesystem>
#include <fstream>
#include <ranges>

#include <cxxopts.hpp>

#include <cppast/cpp_entity_kind.hpp>        // for the cpp_entity_kind definition
#include <cppast/cpp_forward_declarable.hpp> // for is_definition()
#include <cppast/cpp_member_function.hpp>
#include <cppast/cpp_namespace.hpp>          // for cpp_namespace
#include <cppast/cpp_type_alias.hpp>
#include <cppast/libclang_parser.hpp> // for libclang_parser, libclang_compile_config, cpp_entity,...
#include <cppast/visitor.hpp>         // for visit()

std::string ReplaceBackslashes(std::string s) {
    std::replace(s.begin(), s.end(), '\\', '/');
    return s;
}

class WinLogger final : public cppast::diagnostic_logger {
public:
    using diagnostic_logger::diagnostic_logger;

private:
    mutable char buf_[8192] { };
    bool do_log(const char *source, const cppast::diagnostic &d) const override {
        auto loc = d.location.to_string();
        if (loc.empty())
            sprintf_s(buf_, "[%s] [%s] %s\n", source, to_string(d.severity), d.message.c_str());
        else
            sprintf_s(buf_, "[%s] [%s] %s %s\n", source, to_string(d.severity),
                         d.location.to_string().c_str(), d.message.c_str());

        OutputDebugStringA(buf_);
        return true;
    }
};

class Parser {
    std::ofstream hookSrc_;
    std::ofstream hookHeader_;
    std::ofstream sdkHeader_;
    std::stringstream hookedCallEnum_;
    std::map<std::string, std::stringstream> addresses_;
    
    cppast::libclang_compile_config config_;
        
    // the entity index is used to resolve cross references in the AST
    cppast::cpp_entity_index idx_;

    // the logger is used to print diagnostics
    WinLogger logger_;

    std::vector<std::unique_ptr<cppast::cpp_file>> files_;

    enum class TypeInformation {
        UNKNOWN,
        IS_FLOAT,
        IS_INTEGRAL,
        IS_UNSIGNED,
        IS_STRING,
        IS_POINTER
    };

    struct ParsedArg {
        const std::string type;
        const std::string name;
        TypeInformation info;
    };

    const cppast::cpp_type& WalkTypedefs(const cppast::cpp_type& baseType) const {
        if(baseType.kind() != cppast::cpp_type_kind::user_defined_t)
            return baseType;

        const auto& userType = dynamic_cast<const cppast::cpp_user_defined_type&>(baseType);
        const auto& typeEntity = userType.entity().get(idx_)[0u].get();
        if(typeEntity.kind() == cppast::cpp_entity_kind::type_alias_t) {
            const auto& typeAlias = dynamic_cast<const cppast::cpp_type_alias&>(typeEntity);
            const auto& underlyingType = typeAlias.underlying_type();
            return WalkTypedefs(underlyingType);
        }

        return baseType;
    }

    TypeInformation DetermineTypeInformation(const cppast::cpp_type& in_type) const {
        const auto& type = WalkTypedefs(in_type);
        if(type.kind() == cppast::cpp_type_kind::pointer_t) {
            const auto& ptr = dynamic_cast<const cppast::cpp_pointer_type&>(type);
            if(ptr.pointee().kind() == cppast::cpp_type_kind::builtin_t) {
                const auto& builtin = dynamic_cast<const cppast::cpp_builtin_type&>(ptr.pointee());
                if(builtin.builtin_type_kind() == cppast::cpp_char)
                    return TypeInformation::IS_STRING;
            }
            return TypeInformation::IS_POINTER;
        }

        if(type.kind() == cppast::cpp_type_kind::builtin_t) {
            const auto& builtin = dynamic_cast<const cppast::cpp_builtin_type&>(type);
            
            switch (builtin.builtin_type_kind()) {
            using namespace cppast;
            case cpp_bool:
            case cpp_schar:
            case cpp_short:
            case cpp_int:
            case cpp_long:
            case cpp_longlong:
                return TypeInformation::IS_INTEGRAL;
            case cpp_uchar:
            case cpp_ushort:
            case cpp_uint:
            case cpp_ulong:
            case cpp_ulonglong:
                return TypeInformation::IS_UNSIGNED;
            case cpp_float:
            case cpp_double:
                return TypeInformation::IS_FLOAT;
            default:
                return TypeInformation::UNKNOWN;
            }
        }

        return TypeInformation::UNKNOWN;
    }

    template<std::ranges::range T>
    std::vector<ParsedArg> ParseArgs(const T& params) {
        size_t genIdx = 0;
        std::vector<ParsedArg> args;
        args.reserve(std::distance(std::ranges::begin(params), std::ranges::end(params)));
        for(const auto& arg : params) {
            std::string argName = arg.name().empty() ? ("_genArg" + std::to_string(++genIdx)) : arg.name();
            args.push_back({ cppast::to_string(arg.type()), argName, DetermineTypeInformation(arg.type()) });
        }

        return args;
    }

    static void AppendArgsList(const std::vector<ParsedArg>& args, bool inclType, std::ostream& output) {
        if(args.empty())
            return;

        auto append = [&](const auto& a, bool trailingComma = true) {
            if(inclType)
                output << a.type << " ";
            output << a.name;
            if(trailingComma) output << ", ";
        };

        std::ranges::for_each(args | std::ranges::views::take(args.size() - 1), append);
        append(args.back(), false);
    }

    static std::string MakeContextFuncName(const std::string& context, const std::string& funcName) {
        std::string enumContext = context;
        std::replace(enumContext.begin(), enumContext.end(), ':', '_');

        return enumContext + funcName;
    }

    static std::string MakeEnum(const std::string& context, const std::string& funcName) {
        std::string enumContext = context;
        std::replace(enumContext.begin(), enumContext.end(), ':', '_');

        for(auto p = enumContext.find("Hk"); p != std::string::npos; p = enumContext.find("Hk")) {
            enumContext = enumContext.substr(0, p) + enumContext.substr(p + 2);
        }

        return enumContext + funcName;
    }

    void InsertDebugLog(const std::string& funcName, const std::vector<ParsedArg>& args) {
        hookSrc_ << "\tAddDebugLog(\"" << funcName << "(";
        if(!args.empty())
            hookSrc_ << "\\n";
        for(auto& a : args) {
            hookSrc_ << "\\t" << a.type << " " << a.name << " = %";
            switch(a.info) {
            case TypeInformation::IS_INTEGRAL:
                hookSrc_ << "d";
                break;
            case TypeInformation::IS_UNSIGNED:
                hookSrc_ << "u";
                break;
            case TypeInformation::IS_FLOAT:
                hookSrc_ << "f";
                break;
            case TypeInformation::IS_POINTER:
                hookSrc_ << "p";
                break;
            case TypeInformation::IS_STRING:
            default:
                hookSrc_ << "s";
                break;
            }

            hookSrc_ << "\\n";
        }
        if(!args.empty()) {
            hookSrc_ << ")\"," << std::endl << "\t\t\t";
            AppendArgsList(args, false, hookSrc_);
        } else
            hookSrc_ << ")\"";
        hookSrc_ << ");" << std::endl << std::endl;
    }

    void InsertFunctionSignature(const std::string &context, bool serverCall, const cppast::cpp_member_function &func, const std::string &funcName, const std::vector<ParsedArg>& args) {
        if(serverCall && !context.empty())
            hookSrc_ << "namespace " << context.substr(0, context.length() - 2) << " {" << std::endl;

        hookSrc_ << to_string(func.return_type()) << " ";
        if(serverCall)
            hookSrc_ << "__stdcall ";
        hookSrc_ << funcName << "(";
        AppendArgsList(args, true, hookSrc_);
        hookSrc_ << ") {" << std::endl;
    }

    template<bool Before>
    void InsertCallPlugins(const cppast::cpp_member_function &func, const std::string& fullEnumVal, bool voidReturn, const std::vector<ParsedArg>& args) {
        if(Before) {
            const char* returnSig = voidReturn ? "\tauto skip" : "\tauto [retVal, skip]";
            hookSrc_ << returnSig << " = CallPluginsBefore<" << to_string(func.return_type()) << ">";
        } else
            hookSrc_ << std::endl << "\tCallPluginsAfter";

        hookSrc_ << "(" << fullEnumVal;
        if(!args.empty()) {
            hookSrc_ << ",\n\t\t\t";
            AppendArgsList(args, false, hookSrc_);
        }
        hookSrc_ << ");" << std::endl;

        if(Before)
            hookSrc_ << std::endl;
        else if(!voidReturn)
                hookSrc_ << std::endl << "\treturn retVal;" << std::endl;
    }
    
    template<bool Before>
    void InsertSemaphore(const type_safe::optional_ref<const cppast::cpp_attribute>& semaphore) {
        if(semaphore) {
            auto semaphoreName = semaphore.value().arguments().value().front().spelling;
            hookSrc_ << "\t" << semaphoreName << " = " << (Before ? "true" : "false") << ";" << std::endl;
        }
    }

    template<bool client>
    void InsertFunctionCall(const cppast::cpp_member_function &func, const std::vector<ParsedArg>& args, bool noPlugins, bool voidReturn, bool hasCatch) {
        
        if(!voidReturn && noPlugins)
            hookSrc_ << "\t" << to_string(func.return_type()) << " retVal;" << std::endl;
        
        const char* indent = "\t";
        if(!noPlugins) {
            hookSrc_ << "\tif(!skip) {" << std::endl;
            indent = "\t\t";
        }

        hookSrc_ << indent << "CALL_" << (client ? "CLIENT" : "SERVER") << "_PREAMBLE {" << std::endl;

        if(!voidReturn)
            hookSrc_ << indent << "\tretVal = ";
        else
            hookSrc_ << indent << "\t";

        if(!client)
            hookSrc_ << "Server.";

        hookSrc_ << func.name() << "(";
        AppendArgsList(args, false, hookSrc_);
        hookSrc_ << ");" << std::endl;

        hookSrc_ << indent << "} CALL_" << (client ? "CLIENT" : "SERVER") << "_POSTAMBLE";
        if(!client) {
            hookSrc_ << "(";
            if(hasCatch) {
                hookSrc_ << func.name();
                hookSrc_ << "__Catch(";
                AppendArgsList(args, false, hookSrc_);
                hookSrc_ << ")";
            } else
                hookSrc_ << "true";
            hookSrc_ << ")";
        }
        hookSrc_ << ";" << std::endl;
        
        if(!noPlugins)
            hookSrc_ << "\t}" << std::endl;
        else if(!voidReturn)
            hookSrc_ << std::endl;
    }

    void GenerateFunctionHook(const cppast::cpp_entity& entity, const std::string& context, bool clientCall, bool serverCall) {

        const auto& func = dynamic_cast<const cppast::cpp_member_function&>(entity);
        std::string funcName = serverCall ? func.name() : context + func.name();
        std::string enumVal = MakeEnum(context, func.name());
        std::string fullEnumVal = "HookedCall::" + enumVal;

        bool voidReturn = func.return_type().kind() == cppast::cpp_type_kind::builtin_t && dynamic_cast<const cppast::cpp_builtin_type&>(func.return_type()).builtin_type_kind() == cppast::cpp_void;

        bool disabled = has_attribute(entity, "Disable").has_value();
        bool noPlugins = has_attribute(entity, "NoPlugins").has_value();
        bool noPluginsAfter = has_attribute(entity, "NoPluginsAfter").has_value();
        bool noLog = has_attribute(entity, "NoLog").has_value();
        auto callInner = has_attribute(entity, "CallInner");
        bool callInnerAfter = has_attribute(entity, "CallInnerAfter").has_value();
        bool dcCheck = has_attribute(entity, "DisconnectCheck").has_value();
        bool hasCatch = has_attribute(entity, "CallCatch").has_value();
        auto semaphore = has_attribute(entity, "Semaphore");
        auto args = ParseArgs(func.parameters());

        InsertFunctionSignature(context, serverCall, func, funcName, args);

        if(disabled) {
            hookSrc_ << std::endl << "}" << std::endl;
            if(serverCall && !context.empty())
                hookSrc_ << "}" << std::endl;
            hookSrc_ << std::endl;
            return;
        }

        if(!noPlugins)
            hookedCallEnum_ << "\t" << enumVal << "," << std::endl;

        if(!noLog)
            InsertDebugLog(funcName, args);

        if(!noPlugins)
            InsertCallPlugins<true>(func, fullEnumVal, voidReturn, args);

        if(dcCheck)
            hookSrc_ << "\tCHECK_FOR_DISCONNECT;" << std::endl << std::endl;

        if(callInner) {
            hookSrc_ << "\t";
            const auto& callInnerArgs = callInner.value().arguments();
            bool canBreak = callInnerArgs && callInnerArgs.value().front().spelling == "true";
            if(canBreak)
                hookSrc_ << "bool innerCheck = ";
            if(serverCall && !context.empty())
                hookSrc_ << func.name();
            else
                hookSrc_ << MakeContextFuncName(context, func.name());
            hookSrc_ << "__Inner(";
            AppendArgsList(args, false, hookSrc_);
            hookSrc_ << ");" << std::endl;

            if(canBreak) {
                hookSrc_ << "\tif(!innerCheck) return";
                if(!voidReturn)
                    hookSrc_ << " " << to_string(func.return_type()) << "()";
                hookSrc_ << ";";
            }

            hookSrc_ << std::endl;
        }

        InsertSemaphore<true>(semaphore);

        if(clientCall)
            InsertFunctionCall<true>(func, args, noPlugins, voidReturn, hasCatch);
        else if(serverCall)
            InsertFunctionCall<false>(func, args, noPlugins, voidReturn, hasCatch);
        
        InsertSemaphore<false>(semaphore);

        if(callInnerAfter) {
            hookSrc_ << "\t";
            if(serverCall && !context.empty())
                hookSrc_ << func.name();
            else
                hookSrc_ << MakeContextFuncName(context, func.name());
            hookSrc_ << "__InnerAfter(";
            AppendArgsList(args, false, hookSrc_);
            hookSrc_ << ");" << std::endl << std::endl;
        }

        if(!noPlugins && !noPluginsAfter)
            InsertCallPlugins<false>(func, fullEnumVal, voidReturn, args);
        else if(!voidReturn)
            hookSrc_ << "\treturn retVal;" << std::endl;

        hookSrc_ << "}" << std::endl;
        
        if(serverCall && !context.empty())
            hookSrc_ << "}" << std::endl;

        hookSrc_ << std::endl;
    }

    std::stringstream& GetAddressStream(const std::string& context) {
        auto it = addresses_.find(context);
        if(it == addresses_.end()) {
            it = addresses_.insert(std::pair<std::string, std::stringstream>(context, std::stringstream(""))).first;
            it->second << "HookEntry " << MakeContextFuncName(context, "Entries") << "[] = {" << std::endl;
        }

        return it->second;
    }

    void GenerateHooks(const cppast::cpp_entity& e, const std::string& context, bool clientCall, bool serverCall) {
        int32_t funcIndex = 0;
        cppast::visit(e, [&](const cppast::cpp_entity& entity, cppast::visitor_info) {
            if(entity.kind() == cppast::cpp_entity_kind::member_function_t) {
                bool noHook = has_attribute(entity.attributes(), "NoHook").has_value();

                if(!noHook && !entity.name().starts_with("operator")) {
                    auto name = context + (serverCall ? "Hk" : "") + e.name();
                    GenerateFunctionHook(entity, name + "::", clientCall, serverCall);

                    if(serverCall) {
                        int32_t address = funcIndex * 4;
                        if(auto addrAttribute = has_attribute(entity.attributes(), "Address"); addrAttribute)
                            address = std::stoi(addrAttribute.value().arguments().value().as_string(), 0, 0);
                        GetAddressStream(name) << "\t{ FARPROC(" << name << "::" << entity.name() << "), 0x" << std::hex << std::uppercase << std::setw(3) << std::setfill('0') << address << ", nullptr }," << std::endl;
                    }
                }

                funcIndex++;
            }
        });
    }

public:
    explicit Parser(const std::filesystem::path& slnDir) : logger_(true) {
        config_.set_flags(cppast::cpp_standard::cpp_latest, cppast::compile_flag::ms_extensions | cppast::compile_flag::ms_compatibility);
        config_.define_macro("ST6_ALLOCATION_DEFINED", "1");
        config_.define_macro("_X86_", "1");
        config_.define_macro("IMPORT", " ");
        config_.define_macro("EXPORT", " ");
        config_.add_include_dir(ReplaceBackslashes((slnDir / R"(..\FLHookSDK\include)").lexically_normal().string()));
        
        std::filesystem::path genHookSrc = (slnDir / "../source/__generated.cpp").lexically_normal();
        std::filesystem::path genSdkHeader = (slnDir / "../FLHookSDK/include/__generated.h").lexically_normal();

        hookSrc_.open(genHookSrc.c_str());
        hookSrc_ << "//\n// WARNING: THIS IS AN AUTO-GENERATED FILE, DO NOT EDIT!\n//" << std::endl << std::endl;
        hookSrc_ << "#include <Generated.inl>" << std::endl << std::endl;

        sdkHeader_.open(genSdkHeader.c_str());
        sdkHeader_ << "#pragma once\n\n//\n// WARNING: THIS IS AN AUTO-GENERATED FILE, DO NOT EDIT!\n//" << std::endl << std::endl;

        hookedCallEnum_ << "enum class HookedCall {" << std::endl;
    }
    Parser(const Parser&) = delete;
    Parser(Parser&&) = delete;

    cppast::cpp_file& ParseFile(std::string filename, bool fatalError)
    {
        // the parser is used to parse the entity
        // there can be multiple parser implementations
        cppast::libclang_parser parser(type_safe::ref(logger_));
        // parse the file
        auto file = parser.parse(idx_, filename, config_);
        if (fatalError && parser.error())
            throw std::invalid_argument("Parser fatal error.");

        files_.push_back(std::move(file));

        return *files_.back();
    }

    void LocateHooks(const cppast::cpp_entity& base, const std::string& context) {
        cppast::visit(base, [&](const cppast::cpp_entity& entity, cppast::visitor_info vi) -> bool {
            if(&entity == &base)
                return cppast::visitor_result::continue_visit;
            if(vi.event == cppast::visitor_info::container_entity_exit)
                return cppast::visitor_result::continue_visit;

            logger_.log("Generator/locate_hooks", { entity.name(), cppast::source_location::make_unknown(), cppast::severity::info });

            switch(entity.kind()) {
            case cppast::cpp_entity_kind::class_t:
            case cppast::cpp_entity_kind::base_class_t:
                if(has_attribute(entity.attributes(), "Hook").has_value()) {
                    bool clientCall = has_attribute(entity.attributes(), "ClientCall").has_value();
                    bool serverCall = has_attribute(entity.attributes(), "ServerCall").has_value();
                    GenerateHooks(entity, context, clientCall, serverCall);

                    return cppast::visitor_result::continue_visit_no_children;
                }
                break;
            case cppast::cpp_entity_kind::namespace_t:
                LocateHooks(entity, context + entity.name() + "::");
                break;
            default:
                break;
            }

            return cppast::visitor_result::continue_visit;
        });
    }

    ~Parser() {
        for(const auto& a : addresses_) {
            hookSrc_ << a.second.str() << "};" << std::endl << std::endl;
        }
        hookedCallEnum_ << "\tCount" << std::endl << "};";

        sdkHeader_ << hookedCallEnum_.str();
    }
};

int main(int argc, char* argv[])
{
    if(argc != 2)
        return 1;

    //try {
        std::filesystem::path slnDir = argv[1];

        Parser parse(slnDir);

        auto coreDefsPath = ReplaceBackslashes((slnDir / R"(..\FLHookSDK\include\FLCoreDefs.h)").lexically_normal().string());
        parse.ParseFile(coreDefsPath, false);

        auto remoteClientPath = ReplaceBackslashes((slnDir / R"(..\FLHookSDK\include\FLCoreRemoteClient.h)").lexically_normal().string());
        auto& remote_client = parse.ParseFile(remoteClientPath, false);
        parse.LocateHooks(remote_client, "");

        auto serverPath = ReplaceBackslashes((slnDir / R"(..\FLHookSDK\include\FLCoreServer.h)").lexically_normal().string());
        auto& server = parse.ParseFile(serverPath, false);
        parse.LocateHooks(server, "");
    //} catch(...) {
    //    return 1;
    //}
}
