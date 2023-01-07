#include "Global.hpp"

namespace Hk::Err
{
	struct ErrorInfo
	{
		Error Err;
		std::wstring text;
	};

	const std::array<ErrorInfo, 29> Errors = {{
	    {Error::PlayerNotLoggedIn, L"Player not logged in"},
	    {Error::CharacterDoesNotExist, L"Char does not exist"},
	    {Error::CouldNotDecodeCharFile, L"Could not decode charfile"},
	    {Error::CouldNotEncodeCharFile, L"Coult not encode charfile"},
	    {Error::InvalidBaseName, L"Invalid basename"},
	    {Error::NoTargetSelected, L"No target selected"},
	    {Error::TargetIsNotPlayer, L"Target is not a player"},
	    {Error::PlayerNotDocked, L"Player in space"},
	    {Error::UnknownError, L"Unknown error"},
	    {Error::InvalidClientId, L"Invalid client id"},
	    {Error::InvalidIdString, L"Invalid id std::string"},
	    {Error::InvalidSystem, L"Invalid system"},
	    {Error::PlayerNotInSpace, L"Player not in space"},
	    {Error::NoAdmin, L"Player is no admin"},
	    {Error::WrongXmlSyntax, L"Wrong XML syntax"},
	    {Error::InvalidGood, L"Invalid good"},
	    {Error::CharacterNotSelected, L"Player has no char selected"},
	    {Error::AlreadyExists, L"Charname already exists"},
	    {Error::CharacterNameTooLong, L"Charname is too long"},
	    {Error::CharacterNameTooShort, L"Charname is too short"},
	    {Error::AmbiguousShortcut, L"Ambiguous shortcut"},
	    {Error::NoMatchingPlayer, L"No matching player"},
	    {Error::InvalidShortcutString, L"Invalid shortcut std::string"},
	    {Error::MpNewCharacterFileNotFoundOrInvalid, L"mpnewcharacter.fl not found or invalid"},
	    {Error::InvalidRepGroup, L"Invalid reputation group"},
	    {Error::PluginCannotBeLoaded, L"Plugin cannot be unloaded"},
	    {Error::PluginNotFound, L"Plugin not found"},
	    {Error::InvalidIdType, L"Invalid Id Type provided"},
	    {Error::InvalidSpaceObjId, L"Invalid SpaceObj Id provided"},
	}};

	std::wstring ErrGetText(Error Err)
	{
		if (const auto err = std::find_if(Errors.begin(), Errors.end(), [Err](const ErrorInfo& e) { return e.Err == Err; }); err != Errors.end())
		{
			return err->text;
		}

		return L"No error text available";
	}
} // namespace Hk::Err