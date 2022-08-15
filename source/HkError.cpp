#include "Global.hpp"

struct ErrorInfo
{
	HkError hkErr;
	std::wstring text;
};

const std::array<ErrorInfo, 29> hkErrors = {{
    {HKE_OK, L"Ok"},
    {PlayerNotLoggedIn, L"Player not logged in"},
    {CharDoesNotExist, L"Char does not exist"},
    {CouldNotDecodeCharFile, L"Could not decode charfile"},
    {CouldNotEncodeCharFile, L"Coult not encode charfile"},
    {InvalidBaseName, L"Invalid basename"},
    {NoTargetSelected, L"No target selected"},
    {TargetIsNotPlayer, L"Target is not a player"},
    {PlayerNotDocked, L"Player in space"},
    {UnknownError, L"Unknown error"},
    {InvalidClientId, L"Invalid client id"},
    {InvalidIdString, L"Invalid id std::string"},
    {InvalidSystem, L"Invalid system"},
    {PlayerNotInSpace, L"Player not in space"},
    {NoAdmin, L"Player is no admin"},
    {WrongXmlSyntax, L"Wrong XML syntax"},
    {InvalidGood, L"Invalid good"},
    {NoCharSelected, L"Player has no char selected"},
    {AlreadyExists, L"Charname already exists"},
    {CharacterNameTooLong, L"Charname is too long"},
    {CharacterNameTooShort, L"Charname is too short"},
    {AmbiguousShortcut, L"Ambiguous shortcut"},
    {NoMatchingPlayer, L"No matching player"},
    {InvalidShortcutString, L"Invalid shortcut std::string"},
    {MpNewCharacterFileNotFoundOrInvalid, L"mpnewcharacter.fl not found or invalid"},
    {InvalidRepGroup, L"Invalid reputation group"},
    {PluginCannotBeLoaded, L"Plugin cannot be unloaded"},
    {PluginNotFound, L"Plugin not found"},
}};

std::wstring HkErrGetText(HkError hkErr)
{
	if (const auto err = std::find_if(hkErrors.begin(), hkErrors.end(), [hkErr](const ErrorInfo& err) {
		return err.hkErr == hkErr;
	}); err != hkErrors.end())
	{
		return err->text;
	}

	return L"No error text available";
}
