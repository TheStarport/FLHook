#pragma once

class DLL UnsupportedException final : std::exception
{
    public:
        explicit UnsupportedException() {}
        ~UnsupportedException() noexcept override = default;
        [[nodiscard]]
        const char* what() const noexcept override
        {
            return "The action performed is not supported or valid.";
        }
};
