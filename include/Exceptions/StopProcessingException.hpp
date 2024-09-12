#pragma once

class DLL StopProcessingException final : std::exception
{
    public:
        explicit StopProcessingException() {}
        ~StopProcessingException() noexcept override = default;

        [[nodiscard]]
        const char* what() const noexcept override
        {
            return "The execution of the current scope was requested to be stopped.";
        }
};
