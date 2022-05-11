#pragma once

#include <plugin.h>
#include <string>

class MailCommunicator final : public PluginCommunicator {
  public:
    inline static const char *pluginName = "Mail";
    explicit MailCommunicator(const std::string &plugin);
    void PluginCall(SendMail, std::wstring character, std::wstring msg);

    enum class MailEvent {
        MailSent = 0
    };

    struct MailSent {
        std::wstring character;
        std::wstring msg;
    };
};