#pragma once

enum class Error
{
    UnknownError = 0,
    InvalidEquipment = 1,
    InvalidRepGroup = 2,
    InvalidRepInstance = 3,
    NoAffiliation = 4,
    InvalidGroupId = 5,
    InvalidObject = 6,
    InvalidFuse = 7,
    InvalidGood = 8,
    InvalidSound = 9,
    InvalidBase = 10,
    InvalidSystem = 11,
    InvalidClientId = 12,
    InvalidXmlSyntax = 13,
    InvalidPersonality = 14,
    PlayerNotOnline = 15,
    PlayerInCharacterSelect = 16,
    PlayerNotDocked = 17,
    PlayerNotInSpace = 18,
    PlayerNotInGroup = 19,
    ObjectIsNotAPlayer = 20,
    ObjectIsNotAnNpc = 21,
    ObjectIsNotAShip = 22,
    ObjectIsNotASolar = 23,
    DatabaseError = 24,
    PacketError = 25,
    PluginNotFound = 26,
    PlayerNoTradeActive = 27,
    NoFlufClientHook = 28,
};

class DLL ErrorInfo
{
        const inline static std::unordered_map<Error, std::wstring_view> errors = {
            {
             { Error::UnknownError, L"Unknown error / Malformed error response. Contact developer." },
             { Error::InvalidEquipment, L"The specified equipment / ship not found." },
             { Error::InvalidRepGroup, L"The specified faction was not found." },
             { Error::InvalidRepInstance, L"Could not fetch reputation data." },
             { Error::NoAffiliation, L"Object has no affiliation." },
             { Error::InvalidGroupId, L"Invalid GroupId provided. This is a developer issue, contact support." },
             { Error::InvalidObject, L"The object specified does not exist." },
             { Error::InvalidFuse, L"Fuse specified was not found, could not be lit, or could not be extinguished." },
             { Error::InvalidGood, L"The good specified was not found. The item/ship might not have a good entry associated with it." },
             { Error::InvalidSound, L"The sound / music specified was not found." },
             { Error::InvalidBase, L"The specified system was not found." },
             { Error::InvalidSystem, L"The specified base was not found." },
             { Error::InvalidClientId, L"The specified ClientId was not found." },
             { Error::InvalidXmlSyntax, L"The provided XML syntax was invalid. This is likely a developer/admin/config issue." },
             { Error::InvalidPersonality, L"The specified personality was not found." },
             { Error::PlayerNotOnline, L"Specified player was not online." },
             { Error::PlayerInCharacterSelect, L"Specified player was not online." },
             { Error::PlayerNotDocked, L"Player not docked." },
             { Error::PlayerNotInSpace, L"Player not in space." },
             { Error::PlayerNotInGroup, L"Player not in group." },
             { Error::ObjectIsNotAPlayer, L"The specified object is not a player." },
             { Error::ObjectIsNotAnNpc, L"The specified object is not an NPC." },
             { Error::ObjectIsNotAShip, L"The specified object is not a ship." },
             { Error::ObjectIsNotASolar, L"The specified object is not a solar." },
             { Error::DatabaseError, L"An error occurred while reading/writing to the database. This is a developer/admin issue, contact support." },
             { Error::PacketError, L"An error occurred while reading/sending a packet. Try again later, report if error is recurring." },
             { Error::PluginNotFound, L"Plugin not found / not loaded. This is a developer issue, contact support." },
             { Error::PlayerNoTradeActive, L"Player has no trade window open." },
             { Error::NoFlufClientHook, L"Client has no FLUF clienthook active." },
             }
        };

    public:
        [[nodiscard]]
        inline static std::wstring_view GetText(Error err)
        {
            if (const auto errInfo = errors.find(err); errInfo != errors.end())
            {
                return errInfo->second;
            }

            return L"No error text available";
        };
};
