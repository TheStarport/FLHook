#include <windows.h>
#include <FLHook.h>
#include <algorithm>
#include <list>
#include <map>
#include <math.h>
#include <plugin.h>
#include <plugin_comms.h>
#include <stdio.h>
#include <string>
#include <time.h>

#include "Main.h"

namespace Pub {
namespace Player {
enum MissionMessageType {
    MissionMessageType_Failure, // mission failure
    MissionMessageType_Type1,   // objective
    MissionMessageType_Type2,   // objective
    MissionMessageType_Type3,   // mission success
};
}
} // namespace Pub

#define POPUPDIALOG_BUTTONS_LEFT_YES 1
#define POPUPDIALOG_BUTTONS_CENTER_NO 2
#define POPUPDIALOG_BUTTONS_RIGHT_LATER 4
#define POPUPDIALOG_BUTTONS_CENTER_OK 8

namespace PlayerCommands {
void BaseHelp(uint client, const std::wstring &args) {
    PlayerBase *base = GetPlayerBaseForClient(client);
    if (!base) {
        PrintUserCmdText(client, L"ERR Not in player base");
        return;
    }

    std::wstring help =
        L"<RDL><PUSH/>"
        L"<TRA bold=\"true\"/><TEXT>/base login [password]</TEXT><TRA "
        L"bold=\"false\"/><PARA/>"
        L"<TEXT>Login as base administrator. The following commands are only "
        L"available if you are logged in as a base "
        L"administrator.</TEXT><PARA/><PARA/>"

        L"<TRA bold=\"true\"/><TEXT>/base addpwd [password], /base rmpwd "
        L"[password], /base lstpwd</TEXT><TRA bold=\"false\"/><PARA/>"
        L"<TEXT>Add, remove and list administrator passwords for the "
        L"base.</TEXT><PARA/><PARA/>"

        L"<TRA bold=\"true\"/><TEXT>/base addtag [tag], /base rmtag [tag], "
        L"/base "
        L"lsttag</TEXT><TRA bold=\"false\"/><PARA/>"
        L"<TEXT>Add, remove and list ally tags for the "
        L"base.</TEXT><PARA/><PARA/>"

        L"<TRA bold=\"true\"/><TEXT>/base setmasterpwd [old password] [new "
        L"password]</TEXT><TRA bold=\"false\"/><PARA/>"
        L"<TEXT>Set the master password for the base.</TEXT><PARA/><PARA/>"

        L"<TRA bold=\"true\"/><TEXT>/base rep [clear]</TEXT><TRA "
        L"bold=\"false\"/><PARA/>"
        L"<TEXT>Set or clear the faction that this base is affiliated with. "
        L"When "
        L"setting the affiliation, the affiliation will be that of the player "
        L"executing the command.</TEXT><PARA/><PARA/>"

        L"<TRA bold=\"true\"/><TEXT>/bank withdraw [credits], /bank deposit "
        L"[credits], /bank status</TEXT><TRA bold=\"false\"/><PARA/>"
        L"<TEXT>Withdraw, deposit or check the status of the credits held by "
        L"the "
        L"base's bank.</TEXT><PARA/><PARA/>"

        L"<TRA bold=\"true\"/><TEXT>/shop price [item] [price] [min stock] "
        L"[max "
        L"stock]</TEXT><TRA bold=\"false\"/><PARA/>"
        L"<TEXT>Set the [price] of [item]. If the current stock is less than "
        L"[min stock]"
        L" then the item cannot be bought by docked ships. If the current "
        L"stock "
        L"is more or equal"
        L" to [max stock] then the item cannot be sold to the base by docked "
        L"ships.</TEXT><PARA/><PARA/>"
        L"<TEXT>To prohibit selling to the base of an item by docked ships "
        L"under "
        L"all conditions, set [max stock] to 0."
        L"To prohibit buying from the base of an item by docked ships under "
        L"all "
        L"conditions, set [min stock] to 0.</TEXT><PARA/><PARA/>"

        L"<TRA bold=\"true\"/><TEXT>/shop remove [item]</TEXT><TRA "
        L"bold=\"false\"/><PARA/>"
        L"<TEXT>Remove the item from the stock list. It cannot be sold to the "
        L"base by docked ships unless they are base "
        L"administrators.</TEXT><PARA/><PARA/>"

        L"<TRA bold=\"true\"/><TEXT>/shop [page]</TEXT><TRA "
        L"bold=\"false\"/><PARA/>"
        L"<TEXT>Show the shop stock list for [page]. There are a maximum of 40 "
        L"items shown per page.</TEXT><PARA/><PARA/>"

        L"<TRA bold=\"true\"/><TEXT>/base defensemode</TEXT><TRA "
        L"bold=\"false\"/><PARA/>"
        L"<TEXT>Control the defense mode for the base.</TEXT><PARA/><PARA/>"

        L"<TRA bold=\"true\"/><TEXT>/base info</TEXT><TRA "
        L"bold=\"false\"/><PARA/>"
        L"<TEXT>Set the base's infocard description.</TEXT><PARA/><PARA/>"

        L"<TRA bold=\"true\"/><TEXT>/base facmod</TEXT><TRA "
        L"bold=\"false\"/><PARA/>"
        L"<TEXT>Control factory modules.</TEXT><PARA/><PARA/>"

        L"<TRA bold=\"true\"/><TEXT>/base defmod</TEXT><TRA "
        L"bold=\"false\"/><PARA/>"
        L"<TEXT>Control defense modules.</TEXT><PARA/><PARA/>"

        L"<TRA bold=\"true\"/><TEXT>/base shieldmod</TEXT><TRA "
        L"bold=\"false\"/><PARA/>"
        L"<TEXT>Control shield modules.</TEXT><PARA/><PARA/>"

        L"<TRA bold=\"true\"/><TEXT>/base buildmod</TEXT><TRA "
        L"bold=\"false\"/><PARA/>"
        L"<TEXT>Control the construction and destruction of base modules and "
        L"upgrades.</TEXT><PARA/><PARA/>"

        L"<POP/></RDL>";

    HkChangeIDSString(client, 500000, L"Base Help");
    HkChangeIDSString(client, 500001, help);

    FmtStr caption(0, 0);
    caption.begin_mad_lib(500000);
    caption.end_mad_lib();

    FmtStr message(0, 0);
    message.begin_mad_lib(500001);
    message.end_mad_lib();

    pub::Player::PopUpDialog(client, caption, message,
                             POPUPDIALOG_BUTTONS_CENTER_OK);
}

void BaseLogin(uint client, const std::wstring &args) {
    PlayerBase *base = GetPlayerBaseForClient(client);
    if (!base) {
        PrintUserCmdText(client, L"ERR Not in player base");
        return;
    }

    std::wstring password = GetParam(args, ' ', 2);
    if (!password.length()) {
        PrintUserCmdText(client, L"ERR No password");
        return;
    }

    if (find(base->passwords.begin(), base->passwords.end(), password) ==
        base->passwords.end()) {
        PrintUserCmdText(client, L"ERR Access denied");
        return;
    }

    clients[client].admin = true;
    SendMarketGoodSync(base, client);
    PrintUserCmdText(client, L"OK Access granted");
    PrintUserCmdText(client, L"Welcome administrator, all base command and "
                             L"control functions are available.");
    return;
}

void BaseAddPwd(uint client, const std::wstring &args) {
    PlayerBase *base = GetPlayerBaseForClient(client);
    if (!base) {
        PrintUserCmdText(client, L"ERR Not in player base");
        return;
    }

    if (!clients[client].admin) {
        PrintUserCmdText(client, L"ERR Access denied");
        return;
    }

    std::wstring password = GetParam(args, ' ', 2);
    if (!password.length()) {
        PrintUserCmdText(client, L"ERR No password");
        return;
    }

    if (find(base->passwords.begin(), base->passwords.end(), password) !=
        base->passwords.end()) {
        PrintUserCmdText(client, L"ERR Password already exists");
        return;
    }

    base->passwords.push_back(password);
    base->Save();
    PrintUserCmdText(client, L"OK");
}

void BaseRmPwd(uint client, const std::wstring &args) {
    PlayerBase *base = GetPlayerBaseForClient(client);
    if (!base) {
        PrintUserCmdText(client, L"ERR Not in player base");
        return;
    }

    if (!clients[client].admin) {
        PrintUserCmdText(client, L"ERR Access denied");
        return;
    }

    std::wstring password = GetParam(args, ' ', 2);
    if (!password.length()) {
        PrintUserCmdText(client, L"ERR No password");
    }

    if (find(base->passwords.begin(), base->passwords.end(), password) ==
        base->passwords.end()) {
        PrintUserCmdText(client, L"ERR Password does not exist");
        return;
    }

    base->passwords.remove(password);
    base->Save();
    PrintUserCmdText(client, L"OK");
}

void BaseSetMasterPwd(uint client, const std::wstring &args) {
    PlayerBase *base = GetPlayerBaseForClient(client);
    if (!base) {
        PrintUserCmdText(client, L"ERR Not in player base");
        return;
    }

    if (!clients[client].admin) {
        PrintUserCmdText(client, L"ERR Access denied");
        return;
    }

    std::wstring old_password = GetParam(args, ' ', 2);
    if (!old_password.length()) {
        PrintUserCmdText(client, L"ERR No old password");
        PrintUserCmdText(client,
                         L"/base setmasterpwd <old_password> <new_password>");
        return;
    }

    std::wstring new_password = GetParam(args, ' ', 3);
    if (!new_password.length()) {
        PrintUserCmdText(client, L"ERR No new password");
        PrintUserCmdText(client,
                         L"/base setmasterpwd <old_password> <new_password>");
        return;
    }

    if (find(base->passwords.begin(), base->passwords.end(), new_password) !=
        base->passwords.end()) {
        PrintUserCmdText(client, L"ERR Password already exists");
        return;
    }

    if (base->passwords.size()) {
        if (base->passwords.front() != old_password) {
            PrintUserCmdText(client, L"ERR Incorrect master password");
            PrintUserCmdText(
                client, L"/base setmasterpwd <old_password> <new_password>");
            return;
        }
    }

    base->passwords.remove(old_password);
    base->passwords.push_front(new_password);
    base->Save();
    PrintUserCmdText(client, L"OK New master password %s",
                     new_password.c_str());
}

void BaseLstPwd(uint client, const std::wstring &cmd) {
    PlayerBase *base = GetPlayerBaseForClient(client);
    if (!base) {
        PrintUserCmdText(client, L"ERR Not in player base");
        return;
    }

    if (!clients[client].admin) {
        PrintUserCmdText(client, L"ERR Access denied");
        return;
    }

    // Do not display the first password.
    bool first = true;
    for (auto &i : base->passwords) {
        if (first)
            first = false;
        else
            PrintUserCmdText(client, L"%s", i.c_str());
    }
    PrintUserCmdText(client, L"OK");
}

void BaseAddAllyTag(uint client, const std::wstring &args) {
    PlayerBase *base = GetPlayerBaseForClient(client);
    if (!base) {
        PrintUserCmdText(client, L"ERR Not in player base");
        return;
    }

    if (!clients[client].admin) {
        PrintUserCmdText(client, L"ERR Access denied");
        return;
    }

    std::wstring tag = GetParam(args, ' ', 2);
    if (!tag.length()) {
        PrintUserCmdText(client, L"ERR No tag");
        return;
    }

    if (find(base->ally_tags.begin(), base->ally_tags.end(), tag) !=
        base->ally_tags.end()) {
        PrintUserCmdText(client, L"ERR Tag already exists");
        return;
    }

    base->ally_tags.push_back(tag);
    base->Save();
    PrintUserCmdText(client, L"OK");
}

void BaseRmAllyTag(uint client, const std::wstring &args) {
    PlayerBase *base = GetPlayerBaseForClient(client);
    if (!base) {
        PrintUserCmdText(client, L"ERR Not in player base");
        return;
    }

    if (!clients[client].admin) {
        PrintUserCmdText(client, L"ERR Access denied");
        return;
    }

    std::wstring tag = GetParam(args, ' ', 2);
    if (!tag.length()) {
        PrintUserCmdText(client, L"ERR No tag");
    }

    if (find(base->ally_tags.begin(), base->ally_tags.end(), tag) ==
        base->ally_tags.end()) {
        PrintUserCmdText(client, L"ERR Tag does not exist");
        return;
    }

    base->ally_tags.remove(tag);
    base->Save();
    PrintUserCmdText(client, L"OK");
}

void BaseLstAllyTag(uint client, const std::wstring &cmd) {
    PlayerBase *base = GetPlayerBaseForClient(client);
    if (!base) {
        PrintUserCmdText(client, L"ERR Not in player base");
        return;
    }

    if (!clients[client].admin) {
        PrintUserCmdText(client, L"ERR Access denied");
        return;
    }

    for (auto &i : base->ally_tags)
        PrintUserCmdText(client, L"%s", i.c_str());
    PrintUserCmdText(client, L"OK");
}

void BaseRep(uint client, const std::wstring &args) {
    PlayerBase *base = GetPlayerBaseForClient(client);
    if (!base) {
        PrintUserCmdText(client, L"ERR Not in player base");
        return;
    }

    if (!clients[client].admin) {
        PrintUserCmdText(client, L"ERR Access denied");
        return;
    }

    std::wstring arg = GetParam(args, ' ', 2);
    if (arg == L"clear") {
        base->affiliation = 0;
        base->Save();
        PrintUserCmdText(client, L"OK Affiliation cleared");
        return;
    }

    int rep;
    pub::Player::GetRep(client, rep);

    uint affiliation;
    Reputation::Vibe::Verify(rep);
    Reputation::Vibe::GetAffiliation(rep, affiliation, false);
    if (affiliation == -1) {
        PrintUserCmdText(client, L"OK Player has no affiliation");
        return;
    }

    base->affiliation = affiliation;
    base->Save();
    PrintUserCmdText(
        client, L"OK Affiliation set to %s",
        HkGetWStringFromIDS(Reputation::get_name(affiliation)).c_str());
}

void BaseInfo(uint client, const std::wstring &args) {
    PlayerBase *base = GetPlayerBaseForClient(client);
    if (!base) {
        PrintUserCmdText(client, L"ERR Not in player base");
        return;
    }

    if (!clients[client].admin) {
        PrintUserCmdText(client, L"ERR Access denied");
        return;
    }

    uint iPara = ToInt(GetParam(args, ' ', 2));
    const std::wstring &cmd = GetParam(args, ' ', 3);
    const std::wstring &msg = GetParamToEnd(args, ' ', 4);

    if (iPara > 0 && iPara <= MAX_PARAGRAPHS && cmd == L"a") {
        int length = base->infocard_para[iPara].length() + msg.length();
        if (length > MAX_CHARACTERS) {
            PrintUserCmdText(client, L"ERR Too many characters. Limit is %d",
                             MAX_CHARACTERS);
            return;
        }

        base->infocard_para[iPara] += XMLText(msg);
        PrintUserCmdText(client, L"OK %d/%d characters used", length,
                         MAX_CHARACTERS);

        // Update the infocard text.
        base->infocard.clear();
        for (int i = 1; i <= MAX_PARAGRAPHS; i++) {
            std::wstring wscXML = base->infocard_para[i];
            if (wscXML.length())
                base->infocard += L"<TEXT>" + wscXML + L"</TEXT><PARA/><PARA/>";
        }

        base->Save();
    } else if (iPara > 0 && iPara <= MAX_PARAGRAPHS && cmd == L"d") {
        base->infocard_para[iPara] = L"";
        PrintUserCmdText(client, L"OK");

        // Update the infocard text.
        base->infocard.clear();
        for (int i = 1; i <= MAX_PARAGRAPHS; i++) {
            std::wstring wscXML = base->infocard_para[i];
            if (wscXML.length())
                base->infocard += L"<TEXT>" + wscXML + L"</TEXT><PARA/><PARA/>";
        }

        base->Save();
    } else {
        PrintUserCmdText(client, L"ERR Invalid parameters");
        PrintUserCmdText(client, L"/base info <paragraph> <command> <text>");
        PrintUserCmdText(
            client, L"|  <paragraph> The paragraph number in the range 1-%d",
            MAX_PARAGRAPHS);
        PrintUserCmdText(client, L"|  <command> The command to perform on the "
                                 L"paragraph, 'a' for append, 'd' for delete");
    }
}

void BaseDefenseMode(uint client, const std::wstring &args) {
    PlayerBase *base = GetPlayerBaseForClient(client);
    if (!base) {
        PrintUserCmdText(client, L"ERR Not in player base");
        return;
    }

    if (!clients[client].admin) {
        PrintUserCmdText(client, L"ERR Access denied");
        return;
    }

    std::wstring wscMode = GetParam(args, ' ', 2);
    if (wscMode == L"0") {
        base->defense_mode = 0;
    } else if (wscMode == L"1") {
        base->defense_mode = 1;
    } else if (wscMode == L"2") {
        base->defense_mode = 2;
    } else {
        PrintUserCmdText(client, L"/base defensemode <mode>");
        PrintUserCmdText(client,
                         L"|  <mode> = 0 - neutral to non-allied ships, "
                         L"docking restricted to allied ships");
        PrintUserCmdText(client,
                         L"|  <mode> = 1 - hostile to non-allied ships, "
                         L"docking restricted to allied ships");
        PrintUserCmdText(client, L"|  <mode> = 2 - neutral to non-allied "
                                 L"ships, docking unrestricted");
        PrintUserCmdText(client, L"defensemode = %u", base->defense_mode);
        return;
    }

    PrintUserCmdText(client, L"OK defensemode = %u", base->defense_mode);
    base->Save();
    base->SyncReputationForBase();
}

void BaseBuildMod(uint client, const std::wstring &args) {
    PlayerBase *base = GetPlayerBaseForClient(client);
    if (!base) {
        PrintUserCmdText(client, L"ERR Not in player base");
        return;
    }

    if (!clients[client].admin) {
        PrintUserCmdText(client, L"ERR Access denied");
        return;
    }

    const std::wstring &cmd = GetParam(args, ' ', 2);
    if (cmd == L"list") {
        PrintUserCmdText(client, L"Modules:");
        for (uint index = 1; index < base->modules.size(); index++) {
            if (base->modules[index]) {
                Module *mod = (Module *)base->modules[index];
                PrintUserCmdText(client, L"%u: %s", index,
                                 mod->GetInfo(false).c_str());
            } else {
                PrintUserCmdText(
                    client, L"%u: Empty - available for new module", index);
            }
        }
        PrintUserCmdText(client, L"OK");
    } else if (cmd == L"destroy") {
        uint index = ToInt(GetParam(args, ' ', 3));
        if (index < 1 || index >= base->modules.size() ||
            !base->modules[index]) {
            PrintUserCmdText(client, L"ERR Module not found");
            return;
        }

        delete base->modules[index];
        base->modules[index] = 0;
        base->Save();
        PrintUserCmdText(client, L"OK Module destroyed");
    } else if (cmd == L"construct") {
        uint index = ToInt(GetParam(args, ' ', 3));
        uint type = ToInt(GetParam(args, ' ', 4));
        if (index < 1 || index >= base->modules.size() ||
            base->modules[index]) {
            PrintUserCmdText(client, L"ERR Module index not valid");
            return;
        }

        if (type < Module::TYPE_CORE || type > Module::TYPE_LAST) {
            PrintUserCmdText(client, L"ERR Module type not available");
            return;
        }

        if (type == Module::TYPE_CORE) {
            if (base->base_level >= 4) {
                PrintUserCmdText(client, L"ERR Upgrade not available");
                return;
            }
        }

        base->modules[index] = new BuildModule(base, type);
        base->Save();
        PrintUserCmdText(client, L"OK Module construction started");
    } else {
        PrintUserCmdText(client, L"ERR Invalid parameters");
        PrintUserCmdText(client, L"/base buildmod [list|construct|destroy]");
        PrintUserCmdText(client, L"|  list - show modules and build status");
        PrintUserCmdText(client,
                         L"|  destroy <index> - destroy module at <index>");
        PrintUserCmdText(client,
                         L"|  construct <index> <type> - start building "
                         L"module <type> at <index>");
        PrintUserCmdText(client, L"|     <type> = 1 - core upgrade");
        PrintUserCmdText(client, L"|     <type> = 2 - shield generator");
        PrintUserCmdText(client, L"|     <type> = 3 - cargo storage");
        PrintUserCmdText(client,
                         L"|     <type> = 4 - defense platform array type 1");
        PrintUserCmdText(client, L"|     <type> = 5 - docking module factory");
        PrintUserCmdText(client,
                         L"|     <type> = 6 - jumpdrive manufacturing factory");
        PrintUserCmdText(
            client,
            L"|     <type> = 7 - hyperspace survey manufacturing factory");
        PrintUserCmdText(
            client,
            L"|     <type> = 8 - cloaking device manufacturing factory");
        PrintUserCmdText(client,
                         L"|     <type> = 9 - defense platform array type 2");
        PrintUserCmdText(client,
                         L"|     <type> = 10 - defense platform array type 3");
    }
}

void BaseFacMod(uint client, const std::wstring &args) {
    PlayerBase *base = GetPlayerBaseForClient(client);
    if (!base) {
        PrintUserCmdText(client, L"ERR Not in player base");
        return;
    }

    if (!clients[client].admin) {
        PrintUserCmdText(client, L"ERR Access denied");
        return;
    }

    const std::wstring &cmd = GetParam(args, ' ', 2);
    if (cmd == L"list") {
        PrintUserCmdText(client, L"Factory Modules:");
        for (uint index = 1; index < base->modules.size(); index++) {
            if (base->modules[index] &&
                (base->modules[index]->type == Module::TYPE_M_CLOAK ||
                 base->modules[index]->type ==
                     Module::TYPE_M_HYPERSPACE_SCANNER ||
                 base->modules[index]->type == Module::TYPE_M_JUMPDRIVES ||
                 base->modules[index]->type == Module::TYPE_M_DOCKING)) {
                FactoryModule *mod = (FactoryModule *)base->modules[index];
                PrintUserCmdText(client, L"%u: %s", index,
                                 mod->GetInfo(false).c_str());
            }
        }
        PrintUserCmdText(client, L"OK");
    } else if (cmd == L"clear") {
        uint index = ToInt(GetParam(args, ' ', 3));
        if (index < 1 || index >= base->modules.size() ||
            !base->modules[index]) {
            PrintUserCmdText(client, L"ERR Module index not valid");
            return;
        }

        if (!base->modules[index] ||
            (base->modules[index]->type != Module::TYPE_M_CLOAK &&
             base->modules[index]->type != Module::TYPE_M_HYPERSPACE_SCANNER &&
             base->modules[index]->type != Module::TYPE_M_JUMPDRIVES &&
             base->modules[index]->type != Module::TYPE_M_DOCKING)) {
            PrintUserCmdText(client, L"ERR Not factory module");
            return;
        }

        FactoryModule *mod = (FactoryModule *)base->modules[index];
        if (mod->ClearQueue())
            PrintUserCmdText(client, L"OK Build queue cleared");
        else
            PrintUserCmdText(client, L"ERR Build queue clear failed");
        base->Save();
    } else if (cmd == L"add") {
        uint index = ToInt(GetParam(args, ' ', 3));
        uint type = ToInt(GetParam(args, ' ', 4));
        if (index < 1 || index >= base->modules.size() ||
            !base->modules[index]) {
            PrintUserCmdText(client, L"ERR Module index not valid");
            return;
        }

        if (!base->modules[index] ||
            (base->modules[index]->type != Module::TYPE_M_CLOAK &&
             base->modules[index]->type != Module::TYPE_M_HYPERSPACE_SCANNER &&
             base->modules[index]->type != Module::TYPE_M_JUMPDRIVES &&
             base->modules[index]->type != Module::TYPE_M_DOCKING)) {
            PrintUserCmdText(client, L"ERR Not factory module");
            return;
        }

        FactoryModule *mod = (FactoryModule *)base->modules[index];
        if (mod->AddToQueue(type))
            PrintUserCmdText(client, L"OK Item added to build queue");
        else
            PrintUserCmdText(client, L"ERR Item add to build queue failed");
        base->Save();
    } else {
        PrintUserCmdText(client, L"ERR Invalid parameters");
        PrintUserCmdText(client, L"/base facmod [list|clear|add]");
        PrintUserCmdText(client,
                         L"|  list - show factory modules and build status");
        PrintUserCmdText(client, L"|  clear <index> - clear build queue for "
                                 L"factory module at <index>");
        PrintUserCmdText(client, L"|  add <index> <type> - add item <type> to "
                                 L"build queue for factory module at <index>");
        PrintUserCmdText(client, L"|     For Docking Module Factory:");
        PrintUserCmdText(client, L"|     <type> = 1 - docking module type 1");
        PrintUserCmdText(client, L"|     For Hyperspace Jumpdrive Factory");
        PrintUserCmdText(client, L"|     <type> = 2 - Jump Drive Series II");
        PrintUserCmdText(client, L"|     <type> = 3 - Jump Drive Series III");
        PrintUserCmdText(client, L"|     <type> = 4 - Jump Drive Series IV");
        PrintUserCmdText(client, L"|     For Hyperspace Survey Factory");
        PrintUserCmdText(client,
                         L"|     <type> = 5 - Hyperspace Survey Module Mk1");
        PrintUserCmdText(client,
                         L"|     <type> = 6 - Hyperspace Survey Module Mk2");
        PrintUserCmdText(client,
                         L"|     <type> = 7 - Hyperspace Survey Module Mk3");
        PrintUserCmdText(client, L"|     For Cloaking Device Factory");
        PrintUserCmdText(client, L"|     <type> = 8 - Cloaking Device (small)");
        PrintUserCmdText(client,
                         L"|     <type> = 9 - Cloaking Device (medium)");
        PrintUserCmdText(client,
                         L"|     <type> = 10 - Cloaking Device (large)");
    }
}

void BaseDefMod(uint client, const std::wstring &args) {
    PlayerBase *base = GetPlayerBaseForClient(client);
    if (!base) {
        PrintUserCmdText(client, L"ERR Not in player base");
        return;
    }

    if (!clients[client].admin) {
        PrintUserCmdText(client, L"ERR Access denied");
        return;
    }

    const std::wstring &cmd = GetParam(args, ' ', 2);
    if (cmd == L"list") {
        PrintUserCmdText(client, L"Defense Modules:");
        for (uint index = 0; index < base->modules.size(); index++) {
            if (base->modules[index]) {
                if (base->modules[index]->type == Module::TYPE_DEFENSE_1 ||
                    base->modules[index]->type == Module::TYPE_DEFENSE_2 ||
                    base->modules[index]->type == Module::TYPE_DEFENSE_3) {
                    DefenseModule *mod = (DefenseModule *)base->modules[index];
                    PrintUserCmdText(client,
                                     L"Module %u: Position %0.0f %0.0f %0.0f "
                                     L"Orient %0.0f %0.0f %0.0f",
                                     index, mod->pos.x, mod->pos.y, mod->pos.z,
                                     mod->rot.z, mod->rot.y, mod->rot.z);
                }
            }
        }
        PrintUserCmdText(client, L"OK");
    } else if (cmd == L"set") {
        uint index = ToInt(GetParam(args, ' ', 3));
        float x = (float)ToInt(GetParam(args, ' ', 4));
        float y = (float)ToInt(GetParam(args, ' ', 5));
        float z = (float)ToInt(GetParam(args, ' ', 6));
        float rx = (float)ToInt(GetParam(args, ' ', 7));
        float ry = (float)ToInt(GetParam(args, ' ', 8));
        float rz = (float)ToInt(GetParam(args, ' ', 9));
        if (index < base->modules.size() && base->modules[index]) {
            if (base->modules[index]->type == Module::TYPE_DEFENSE_1 ||
                base->modules[index]->type == Module::TYPE_DEFENSE_2 ||
                base->modules[index]->type == Module::TYPE_DEFENSE_3) {
                DefenseModule *mod = (DefenseModule *)base->modules[index];

                // Distance from base is limited to 5km
                Vector new_pos = {x, y, z};
                if (HkDistance3D(new_pos, base->position) > 5000) {
                    PrintUserCmdText(client, L"ERR Out of range");
                    return;
                }

                mod->pos = new_pos;
                mod->rot.x = rx;
                mod->rot.y = ry;
                mod->rot.z = rz;

                PrintUserCmdText(
                    client,
                    L"OK Module %u: Position %0.0f %0.0f %0.0f Orient "
                    L"%0.0f %0.0f %0.0f",
                    index, mod->pos.x, mod->pos.y, mod->pos.z, mod->rot.z,
                    mod->rot.y, mod->rot.z);
                base->Save();
                mod->Reset();
            } else {
                PrintUserCmdText(client, L"ERR Module not found");
            }
        } else {
            PrintUserCmdText(client, L"ERR Module not found");
        }
    } else {
        PrintUserCmdText(client, L"ERR Invalid parameters");
        PrintUserCmdText(client, L"/base defmod [list|set]");
        PrintUserCmdText(client, L"|  list - show position and orientations of "
                                 L"this bases weapons platform");
        PrintUserCmdText(
            client,
            L"|  set - <index> <x> <y> <z> <rx> <ry> <rz> - set the "
            L"position and orientation of the <index> weapons platform, "
            L"where x,y,z is the position and rx,ry,rz is the orientation");
    }
}

void BaseShieldMod(uint client, const std::wstring &args) {
    PlayerBase *base = GetPlayerBaseForClient(client);
    if (!base) {
        PrintUserCmdText(client, L"ERR Not in player base");
        return;
    }

    if (!clients[client].admin) {
        PrintUserCmdText(client, L"ERR Access denied");
        return;
    }

    const std::wstring &cmd = GetParam(args, ' ', 2);
    if (cmd == L"on") {
        base->shield_active_time = 3600 * 24;
    } else if (cmd == L"off") {
        base->shield_active_time = 0;
    } else {
        PrintUserCmdText(client, L"ERR Invalid parameters");
        PrintUserCmdText(client, L"/base shieldmod [on|off]");
        PrintUserCmdText(client, L"|  on - turn the shield on");
        PrintUserCmdText(client, L"|  off - turn the shield off");
    }

    // Force the timer for the shield module(s) to run and read their
    // status.
    for (uint index = 0; index < base->modules.size(); index++) {
        if (base->modules[index] &&
            base->modules[index]->type == Module::TYPE_SHIELDGEN) {
            ShieldModule *mod = (ShieldModule *)base->modules[index];
            mod->Timer(0);
            PrintUserCmdText(client, L"|  * %s", mod->GetInfo(false).c_str());
        }
    }
    PrintUserCmdText(client, L"OK");
}

void Bank(uint client, const std::wstring &args) {
    PlayerBase *base = GetPlayerBaseForClient(client);
    if (!clients[client].admin) {
        PrintUserCmdText(client, L"ERR Access denied");
        return;
    }

    const std::wstring &cmd = GetParam(args, ' ', 1);
    int money = ToInt(GetParam(args, ' ', 2));

    std::wstring charname =
        (const wchar_t *)Players.GetActiveCharacterName(client);

    if (cmd == L"withdraw") {
        float fValue;
        pub::Player::GetAssetValue(client, fValue);

        int iCurrMoney;
        pub::Player::InspectCash(client, iCurrMoney);

        if (fValue + money > 2100000000 || iCurrMoney + money > 2100000000) {
            PrintUserCmdText(client, L"ERR Ship asset value will be exceeded");
            return;
        }

        if (money > base->money || money < 0) {
            PrintUserCmdText(client, L"ERR Not enough or invalid credits");
            return;
        }

        pub::Player::AdjustCash(client, money);
        base->money -= money;
        base->Save();

        AddLog(Normal,L"NOTICE: Bank withdraw new_balance=%I64d money=%d base=%s "
               "charname=%s (%s)",
               base->money, money, wstos(base->basename).c_str(),
               wstos(charname).c_str(),
               wstos(HkGetAccountID(HkGetAccountByCharname(charname))).c_str());

        PrintUserCmdText(client, L"OK %u credits withdrawn", money);
    } else if (cmd == L"deposit") {
        int iCurrMoney;
        pub::Player::InspectCash(client, iCurrMoney);

        if (money > iCurrMoney || money < 0) {
            PrintUserCmdText(client, L"ERR Not enough or invalid credits");
            return;
        }

        pub::Player::AdjustCash(client, 0 - money);
        base->money += money;
        base->Save();

        AddLog(Normal,L"NOTICE: Bank deposit money=%d new_balance=%I64d base=%s "
               "charname=%s (%s)",
               money, base->money, wstos(base->basename).c_str(),
               wstos(charname).c_str(),
               wstos(HkGetAccountID(HkGetAccountByCharname(charname))).c_str());

        PrintUserCmdText(client, L"OK %u credits deposited", money);
    } else if (cmd == L"status") {
        PrintUserCmdText(client, L"OK current balance %I64d credits",
                         base->money);
    } else {
        PrintUserCmdText(client, L"ERR Invalid parameters");
        PrintUserCmdText(client, L"/bank [deposit|withdraw|status] [credits]");
    }
}

static void ShowShopStatus(uint client, PlayerBase *base, int page) {
    int pages = (base->market_items.size() / 40) + 1;
    if (page > pages)
        page = pages;
    else if (page < 1)
        page = 1;

    wchar_t buf[1000];
    _snwprintf_s(buf, sizeof(buf), L"Shop Management : Page %d/%d", page,
                 pages);
    std::wstring title = buf;

    int start_item = ((page - 1) * 40) + 1;
    int end_item = page * 40;

    std::wstring status = L"<RDL><PUSH/>";
    status += L"<TEXT>Available commands:</TEXT><PARA/>";
    status += L"<TEXT>  /shop price [item] [price] [min stock] [max "
              L"stock]</TEXT><PARA/>";
    status += L"<TEXT>  /shop remove [item]</TEXT><PARA/>";
    status += L"<TEXT>  /shop [page]</TEXT><PARA/><PARA/>";

    status += L"<TEXT>Stock:</TEXT><PARA/>";
    int item = 1;

    for (std::map<UINT, MARKET_ITEM>::iterator i = base->market_items.begin();
         i != base->market_items.end(); ++i, item++) {
        if (item < start_item)
            continue;
        if (item > end_item)
            break;

        const GoodInfo *gi = GoodList::find_by_id(i->first);
        if (!gi)
            continue;

        wchar_t buf[1000];
        _snwprintf_s(buf, _TRUNCATE,
                     L"<TEXT>  %02u:  %u x %s %0.0f credits stock: %u min %u "
                     L"max</TEXT><PARA/>",
                     uint(item), i->second.quantity,
                     HtmlEncode(HkGetWStringFromIDS(gi->iIDSName)).c_str(),
                     i->second.price, i->second.min_stock, i->second.max_stock);
        status += buf;
    }
    status += L"<POP/></RDL>";

    HkChangeIDSString(client, 500000, title);
    HkChangeIDSString(client, 500001, status);

    FmtStr caption(0, 0);
    caption.begin_mad_lib(500000);
    caption.end_mad_lib();

    FmtStr message(0, 0);
    message.begin_mad_lib(500001);
    message.end_mad_lib();

    pub::Player::PopUpDialog(client, caption, message,
                             POPUPDIALOG_BUTTONS_CENTER_OK);
}

void Shop(uint client, const std::wstring &args) {
    // Check that this player is in a player controlled base
    PlayerBase *base = GetPlayerBaseForClient(client);
    if (!base) {
        PrintUserCmdText(client, L"ERR Not in player base");
        return;
    }

    if (!clients[client].admin) {
        PrintUserCmdText(client, L"ERROR: Access denied");
        return;
    }

    const std::wstring &cmd = GetParam(args, ' ', 1);
    if (cmd == L"price") {
        int item = ToInt(GetParam(args, ' ', 2));
        int money = ToInt(GetParam(args, ' ', 3));
        int min_stock = ToInt(GetParam(args, ' ', 4));
        int max_stock = ToInt(GetParam(args, ' ', 5));

        if (money < 1 || money > 1000000000) {
            PrintUserCmdText(client, L"ERR Price not valid");
            return;
        }

        int curr_item = 1;
        for (std::map<UINT, MARKET_ITEM>::iterator i = base->market_items.begin();
             i != base->market_items.end(); ++i, curr_item++) {
            if (curr_item == item) {
                i->second.price = (float)money;
                i->second.min_stock = min_stock;
                i->second.max_stock = max_stock;
                SendMarketGoodUpdated(base, i->first, i->second);
                base->Save();

                int page = ((curr_item + 39) / 40);
                ShowShopStatus(client, base, page);
                PrintUserCmdText(client, L"OK");
                return;
            }
        }
        PrintUserCmdText(client, L"ERR Commodity does not exist");
    } else if (cmd == L"remove") {
        int item = ToInt(GetParam(args, ' ', 2));

        int curr_item = 1;
        for (std::map<UINT, MARKET_ITEM>::iterator i = base->market_items.begin();
             i != base->market_items.end(); ++i, curr_item++) {
            if (curr_item == item) {
                i->second.price = 0;
                i->second.quantity = 0;
                i->second.min_stock = 0;
                i->second.max_stock = 0;
                SendMarketGoodUpdated(base, i->first, i->second);
                base->market_items.erase(i->first);
                base->Save();

                int page = ((curr_item + 39) / 40);
                ShowShopStatus(client, base, page);
                PrintUserCmdText(client, L"OK");
                return;
            }
        }
        PrintUserCmdText(client, L"ERR Commodity does not exist");
    } else {
        int page = ToInt(GetParam(args, ' ', 1));
        ShowShopStatus(client, base, page);
        PrintUserCmdText(client, L"OK");
    }
}

void BaseDeploy(uint client, const std::wstring &args) {
    // Abort processing if this is not a "heavy lifter"
    uint shiparch;
    pub::Player::GetShipID(client, shiparch);
    if (set_construction_shiparch != 0 &&
        shiparch != set_construction_shiparch) {
        PrintUserCmdText(client, L"ERR Need construction ship");
        return;
    }

    uint ship;
    pub::Player::GetShip(client, ship);
    if (!ship) {
        PrintUserCmdText(client, L"ERR Not in space");
        return;
    }

    // If the ship is moving, abort the processing.
    Vector dir1;
    Vector dir2;
    pub::SpaceObj::GetMotion(ship, dir1, dir2);
    if (dir1.x > 5 || dir1.y > 5 || dir1.z > 5) {
        PrintUserCmdText(client, L"ERR Ship is moving");
        return;
    }

    std::wstring password = GetParam(args, ' ', 2);
    if (!password.length()) {
        PrintUserCmdText(client, L"ERR No password");
        PrintUserCmdText(client, L"Usage: /base deploy <password> <name>");
        return;
    }
    std::wstring basename = GetParamToEnd(args, ' ', 3);
    if (!basename.length()) {
        PrintUserCmdText(client, L"ERR No base name");
        PrintUserCmdText(client, L"Usage: /base deploy <password> <name>");
        return;
    }

    // Check for conflicting base name
    if (GetPlayerBase(CreateID(
            PlayerBase::CreateBaseNickname(wstos(basename)).c_str()))) {
        PrintUserCmdText(client, L"ERR Base name already exists");
        return;
    }

    // Check that the ship has the requires commodities.
    int hold_size;
    std::list<CARGO_INFO> cargo;
    HkEnumCargo((const wchar_t *)Players.GetActiveCharacterName(client), cargo,
                hold_size);
    for (std::map<uint, uint>::iterator i = construction_items.begin();
         i != construction_items.end(); ++i) {
        bool material_available = false;
        uint good = i->first;
        uint quantity = i->second;
        for (std::list<CARGO_INFO>::iterator ci = cargo.begin(); ci != cargo.end();
             ++ci) {
            if (ci->iArchID == good && ci->iCount >= (int)quantity) {
                material_available = true;
                pub::Player::RemoveCargo(client, ci->iID, quantity);
            }
        }
        if (material_available == false) {
            PrintUserCmdText(
                client,
                L"ERR Construction failed due to insufficient raw material.");
            for (i = construction_items.begin(); i != construction_items.end();
                 ++i) {
                const GoodInfo *gi = GoodList::find_by_id(i->first);
                if (gi) {
                    PrintUserCmdText(client, L"|  %ux %s", i->second,
                                     HkGetWStringFromIDS(gi->iIDSName).c_str());
                }
            }
            return;
        }
    }

    std::wstring charname =
        (const wchar_t *)Players.GetActiveCharacterName(client);
    AddLog(Normal,L"NOTICE: Base created %s by %s (%s)", wstos(basename).c_str(),
           wstos(charname).c_str(),
           wstos(HkGetAccountID(HkGetAccountByCharname(charname))).c_str());

    PlayerBase *newbase = new PlayerBase(client, password, basename);
    player_bases[newbase->base] = newbase;
    newbase->Spawn();
    newbase->Save();

    PrintUserCmdText(client, L"OK: Base deployed");
    PrintUserCmdText(client, L"Default administration password is %s",
                     password.c_str());
}
} // namespace PlayerCommands
