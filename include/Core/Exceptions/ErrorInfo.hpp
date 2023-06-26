#pragma once

class DLL ErrorInfo
{
        const inline static std::map<Error, std::wstring_view> Errors = {
            {
             { Error::PlayerNotLoggedIn, L"Player not logged in" },
             { Error::CharacterDoesNotExist, L"Char does not exist" },
             { Error::CouldNotDecodeCharFile, L"Could not decode charfile" },
             { Error::CouldNotEncodeCharFile, L"Could not encode charfile" },
             { Error::NoTargetSelected, L"No target selected" },
             { Error::TargetIsNotPlayer, L"Target is not a player" },
             { Error::PlayerNotDocked, L"Player in space" },
             { Error::UnknownError, L"unknown error" },
             { Error::InvalidClientId, L"Invalid client id" },
             { Error::InvalidIdString, L"Invalid id string" },
             { Error::InvalidSystem, L"Invalid system" },
             { Error::PlayerNotInSpace, L"Player not in space" },
             { Error::NotAdmin, L"Player is not an admin" },
             { Error::MissingRequiredRole, L"Player is missing a required role." },
             { Error::WrongXmlSyntax, L"Wrong XML syntax" },
             { Error::InvalidGood, L"Invalid good" },
             { Error::CharacterNotSelected, L"Player has no char selected" },
             { Error::AlreadyExists, L"Charname already exists" },
             { Error::CharacterNameTooLong, L"Charname is too long" },
             { Error::CharacterNameTooShort, L"Charname is too short" },
             { Error::AmbiguousShortcut, L"Ambiguous shortcut" },
             { Error::NoMatchingPlayer, L"No matching player" },
             { Error::InvalidShortcutString, L"Invalid shortcut string" },
             { Error::MpNewCharacterFileNotFoundOrInvalid, L"mpnewcharacter.fl not found or invalid" },
             { Error::InvalidRepGroup, L"Invalid reputation group" },
             { Error::PluginCannotBeLoaded, L"Plugin cannot be unloaded" },
             { Error::PluginNotFound, L"Plugin not found" },
             { Error::InvalidIdType, L"Invalid Id Type provided" },
             { Error::InvalidSpaceObjId, L"Invalid SpaceObj Id provided" },
             { Error::InvalidBase, L"Invalid base provided" },
             { Error::InvalidBaseName, L"Invalid Base name" },
             { Error::MalformedData, L"Data being processed was malformed to the point of failure." },
             }
        };

    public:
        [[nodiscard]]
        static std::wstring_view GetText(Error err);
};
