#include "Global.hpp"

struct ErrorInfo
{
	HK_ERROR hkErr;
	std::wstring text;
};

const std::array<ErrorInfo, 29> hkErrors = {{
    {HKE_OK, L"Ok"},
    {HKE_PLAYER_NOT_LOGGED_IN, L"Player not logged in"},
    {HKE_CHAR_DOES_NOT_EXIST, L"Char does not exist"},
    {HKE_COULD_NOT_DECODE_CHARFILE, L"Could not decode charfile"},
    {HKE_COULD_NOT_ENCODE_CHARFILE, L"Coult not encode charfile"},
    {HKE_INVALID_BASENAME, L"Invalid basename"},
    {HKE_NO_TARGET_SELECTED, L"No target selected"},
    {HKE_TARGET_IS_NOT_PLAYER, L"Target is not a player"},
    {HKE_PLAYER_NOT_DOCKED, L"Player in space"},
    {HKE_UNKNOWN_ERROR, L"Unknown error"},
    {HKE_INVALID_CLIENT_ID, L"Invalid client id"},
    {HKE_INVALID_ID_STRING, L"Invalid id std::string"},
    {HKE_INVALID_SYSTEM, L"Invalid system"},
    {HKE_PLAYER_NOT_IN_SPACE, L"Player not in space"},
    {HKE_PLAYER_NO_ADMIN, L"Player is no admin"},
    {HKE_WRONG_XML_SYNTAX, L"Wrong XML syntax"},
    {HKE_INVALID_GOOD, L"Invalid good"},
    {HKE_NO_CHAR_SELECTED, L"Player has no char selected"},
    {HKE_CHARNAME_ALREADY_EXISTS, L"Charname already exists"},
    {HKE_CHARNAME_TOO_LONG, L"Charname is too long"},
    {HKE_CHARNAME_TOO_SHORT, L"Charname is too short"},
    {HKE_AMBIGUOUS_SHORTCUT, L"Ambiguous shortcut"},
    {HKE_NO_MATCHING_PLAYER, L"No matching player"},
    {HKE_INVALID_SHORTCUT_STRING, L"Invalid shortcut std::string"},
    {HKE_MPNEWCHARACTERFILE_NOT_FOUND_OR_INVALID, L"mpnewcharacter.fl not found or invalid"},
    {HKE_INVALID_REP_GROUP, L"Invalid reputation group"},
    {HKE_PLUGIN_UNLOADABLE, L"Plugin cannot be unloaded"},
    {HKE_PLUGIN_NOT_FOUND, L"Plugin not found"},
}};

std::wstring HkErrGetText(HK_ERROR hkErr)
{
	if (const auto err = std::find_if(hkErrors.begin(), hkErrors.end(), [hkErr](const ErrorInfo& err) {
		return err.hkErr == hkErr;
	}); err != hkErrors.end())
	{
		return err->text;
	}

	return L"No error text available";
}
