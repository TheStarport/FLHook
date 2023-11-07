#pragma once

// TODO: Move this to its own CPP file and use the Detour class
class DebugTools : public Singleton<DebugTools>
{
        static std::map<std::string, uint> hashMap;

        std::allocator<BYTE> allocator;

        static uint CreateIdDetour(const char* str);

    public:
        DebugTools() = default;
        void Init() const;
};
