#pragma once

class DLL ErrorInfo : public Singleton<ErrorInfo>
{
	const std::map<Error, std::string> errors = {{
	    {Error::PlayerNotLoggedIn, "Player not logged in"},
	    {Error::CharacterDoesNotExist, "Char does not exist"},
	    {Error::CouldNotDecodeCharFile, "Could not decode charfile"},
	    {Error::CouldNotEncodeCharFile, "Coult not encode charfile"},
	    {Error::NoTargetSelected, "No target selected"},
	    {Error::TargetIsNotPlayer, "Target is not a player"},
	    {Error::PlayerNotDocked, "Player in space"},
	    {Error::UnknownError, "unknown error"},
	    {Error::InvalidClientId, "Invalid client id"},
	    {Error::InvalidIdString, "Invalid id std::string"},
	    {Error::InvalidSystem, "Invalid system"},
	    {Error::PlayerNotInSpace, "Player not in space"},
	    {Error::NoAdmin, "Player is no admin"},
	    {Error::WrongXmlSyntax, "Wrong XML syntax"},
	    {Error::InvalidGood, "Invalid good"},
	    {Error::CharacterNotSelected, "Player has no char selected"},
	    {Error::AlreadyExists, "Charname already exists"},
	    {Error::CharacterNameTooLong, "Charname is too long"},
	    {Error::CharacterNameTooShort, "Charname is too short"},
	    {Error::AmbiguousShortcut, "Ambiguous shortcut"},
	    {Error::NoMatchingPlayer, "No matching player"},
	    {Error::InvalidShortcutString, "Invalid shortcut std::string"},
	    {Error::MpNewCharacterFileNotFoundOrInvalid, "mpnewcharacter.fl not found or invalid"},
	    {Error::InvalidRepGroup, "Invalid reputation group"},
	    {Error::PluginCannotBeLoaded, "Plugin cannot be unloaded"},
	    {Error::PluginNotFound, "Plugin not found"},
	    {Error::InvalidIdType, "Invalid Id Type provided"},
	    {Error::InvalidSpaceObjId, "Invalid SpaceObj Id provided"},
	    {Error::InvalidBase, "Invalid base provided"},
	    {Error::InvalidBaseName, "Invalid Base name"},
	}};

public:
	[[nodiscard]] std::string GetText(Error err) const;
};