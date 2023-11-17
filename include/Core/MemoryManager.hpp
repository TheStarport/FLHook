#pragma once

class SaveGameDetour
{
        std::wstring path;

        static bool GetUserDataPathDetour(char* retPtr);
        std::wstring GetSaveDataPath() const;

    protected:
        void InitHook();
        void DestroyHook();
};

class MemoryManager final : public SaveGameDetour, public Singleton<MemoryManager>
{
    public:
        MemoryManager() = default;
        void AddHooks();
        void RemoveHooks();
};
