# IPC
Within the FLHook framework, there is a system that allows plugins to communicate with each other, calling functions via an exposed pointer or dispatching events to listening plugins.

## Start To Finish
To allow other plugins to call code from the executing plugin, you must create a new class that inherits from the PluginCommunicator class and then expose any desired functions calls. Within the constructor you must set assign the implementations of these functions. 

Example:
```cpp
// PluginA.h
class ACommunicator final : public PluginCommunicator
{
    public:
        inline static const char* pluginName = "PluginA";
        explicit ACommunicator(const std::string& plugin);

        // Plugin Calls
        void PluginCall(SendHelloWorld, std::wstring msg);
}

// PluginA.cpp

void __stdcall HelloWorld(std::wstring msg) 
{
    Console::ConInfo(msg);
}

ACommunicator::ACommunicator(std::string& plugin) : PluginCommunicator(plugin)
{
   this->SendHelloWorld = HelloWorld;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    return true;
}

extern "C" EXPORT void ExportPluginInfo(PluginInfo *pi) {
    pi->name(ACommunicator::pluginName);
    // ...

    // Register plugin for IPC
    communicator = new ACommunicator(ACommunicator::pluginName);
    PluginCommunicator::ExportPluginCommunicator(communicator);
}

// PluginB.cpp
void LoadSettings()
{
    if (communicator) {
        communicator->SendHelloWorld(L"HelloWorld");
    }
}

extern "C" EXPORT void ExportPluginInfo(PluginInfo *pi) {
    pi->name("PluginB");
    // ...
    pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings);

    // We import the definitions for ACommunicator
    communicator = static_cast<ACommunicator*>(PluginCommunicator::ImportPluginCommunicator(ACommunicator::pluginName));
}

```

In the above example, if PluginA is loaded when PluginB goes to load settings, "Hello World" will be logged to the console. But in this instance, anything can be sent from client ids to structs and large data types.

**GOTCHYA:** You need to make sure that you import the header file with the communicator information from the calling plugins. In the above example, PluginB.cpp would need to import "../pluginA/pluginA.h" in order to work properly.

## But what about two way communication?
Looking at the above example, you might be wondering about if plugins need to be alerted about a change. What if the rename plugin causes a character name to change, and there were numerous plugins that were tracking something based on a character name? Well for that reason, there is a Dispatch function built into the PluginCommunicator class.

In the below example, we will dispatch a structure to any number of plugins, that can do what they want with that data.

```cpp
// PluginA.h
class ACommunicator final : public PluiginCommunicator
{
    public:
        inline static const char* pluginName = "PluginA";
        explicit ACommunicator(const std::string& plugin);

        enum class Event {
            SettingsLoaded
        }

        struct SettingsLoadedData {
            std::wstring settingThatWasLoaded;
        }
}

// PluginA.cpp
ACommunicator::ACommunicator(std::string& plugin) : PluginCommunicator(plugin)
{
    // We don't need to put anything here in this example
    // The constructor is only needed for registering callable functions (like in the above example), not event dispatches.
}

void LoadSettings()
{
    ACommunicator::SettingsLoadedData data { L"Setting 1" }
    communicator->Dispatch(static_cast<ACommunicator::Event::SettingsLoaded>, &data);
}

extern "C" EXPORT void ExportPluginInfo(PluginInfo *pi) {
    pi->name(ACommunicator::pluginName);
    // ...
    pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings);

    // Register plugin for IPC
    communicator = new ACommunicator(ACommunicator::pluginName);
    PluginCommunicator::ExportPluginCommunicator(communicator);
}

// PluginB.cpp

void PluginASettingsLoaded(ACommunicator::SettingsLoadedData* data)
{
    Console::Info(data->settingThatWasLoaded);
}

void __stdcall AEventHandler(int id, void* dataPack) 
{
    switch (static_cast<ACommunicator::Event>(id))
    {
        case ACommunicator::Event::SettingsLoaded:
            PluginASettingsLoaded(static_cast<ACommunicator::SettingsLoadedData*>(dataPack));
            return;
    }
}

extern "C" EXPORT void ExportPluginInfo(PluginInfo *pi) {
    pi->name("pluginB");
    // ...

    // Notice that extra parameter on the end!
    communicator = static_cast<ACommunicator*>(PluginCommunicator::ImportPluginCommunicator(ACommunicator::pluginName), AEventHandler);
}
```

The above example will cause PluginB to log to the console "Setting 1", once LoadSettings is called within PluginA.