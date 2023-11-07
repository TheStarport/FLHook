// ReSharper disable CppParameterMayBeConst
#pragma once

struct SehException final : std::exception
{
        SehException(unsigned code, EXCEPTION_POINTERS* ep) : code(code), record(*ep->ExceptionRecord), context(*ep->ContextRecord) {}

        SehException() = default;

        unsigned code;
        EXCEPTION_RECORD record;
        CONTEXT context;

        static void Translator(unsigned code, EXCEPTION_POINTERS* ep) { throw SehException(code, ep); }

        [[nodiscard]]
        const char* what() const override
        {
            return "SEH Exception should not be handled using what()";
        }
};
