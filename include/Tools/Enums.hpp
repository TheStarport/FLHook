#pragma once
#include "Typedefs.hpp"

enum class LogLevel : int
{
	Trace,
	Debug,
	Info,
	Warn,
	Err,
	Critical
};

enum class LogType : int
{
	Normal,
	Cheater,
	Kick,
	Connects,
	AdminCmds,
	UserLogCmds,
	SocketCmds,
	PerfTimers
};

enum class Error
{
	NicknameNotFound,
	PlayerNotInSpace,
	PlayerNotLoggedIn,
	PlayerNotDocked,
	CharacterNotSelected,
	CharacterDoesNotExist,
	CannotGetAccount,
	MpNewCharacterFileNotFoundOrInvalid,
	NoTargetSelected,
	TargetIsNotPlayer,
	CouldNotDecodeCharFile,
	CouldNotEncodeCharFile,
	NoAdmin,
	WrongXmlSyntax,
	InvalidClientId,
	InvalidGood,
	InvalidShip,
	InvalidBase,
	InvalidBaseName,
	InvalidIdString,
	InvalidSystem,
	InvalidRepGroup,
	InvalidGroupId,
	AlreadyExists,
	CharacterNameTooLong,
	CharacterNameTooShort,
	AmbiguousShortcut,
	NoMatchingPlayer,
	InvalidShortcutString,
	PluginCannotBeLoaded,
	PluginNotFound,
	InvalidIdType,
	InvalidSpaceObjId,
	UnknownError = 1000,
};

enum DIEMSGTYPE
{
	DIEMSG_ALL = 0,
	DIEMSG_SYSTEM = 1,
	DIEMSG_NONE = 2,
	DIEMSG_SELF = 3,
};

enum CHATSIZE
{
	CS_DEFAULT = 0,
	CS_SMALL = 1,
	CS_BIG = 2,
};

enum CHATSTYLE
{
	CST_DEFAULT = 0,
	CST_BOLD = 1,
	CST_ITALIC = 2,
	CST_UNDERLINE = 3,
};

enum EngineState
{
	ES_CRUISE,
	ES_THRUSTER,
	ES_ENGINE,
	ES_KILLED,
	ES_TRADELANE
};

enum EquipmentType
{
	ET_GUN,
	ET_TORPEDO,
	ET_CD,
	ET_MISSILE,
	ET_MINE,
	ET_CM,
	ET_SHIELDGEN,
	ET_THRUSTER,
	ET_SHIELDBAT,
	ET_NANOBOT,
	ET_MUNITION,
	ET_ENGINE,
	ET_OTHER,
	ET_SCANNER,
	ET_TRACTOR,
	ET_LIGHT
};

enum CCMDS_RIGHTS
{
	RIGHT_NOTHING = 0,
	RIGHT_SUPERADMIN = 0xFFFFFFFF,
	RIGHT_MSG = (1 << 0),
	RIGHT_KICKBAN = (1 << 1),
	RIGHT_EVENTMODE = (1 << 2),
	RIGHT_CASH = (1 << 3),
	RIGHT_BEAMKILL = (1 << 4),
	RIGHT_REPUTATION = (1 << 5),
	RIGHT_CARGO = (1 << 6),
	RIGHT_CHARACTERS = (1 << 7),
	RIGHT_SETTINGS = (1 << 8),
	RIGHT_PLUGINS = (1 << 9),
	RIGHT_OTHER = (1 << 10),
	RIGHT_SPECIAL1 = (1 << 11),
	RIGHT_SPECIAL2 = (1 << 12),
	RIGHT_SPECIAL3 = (1 << 13),
};

enum class PluginMajorVersion
{
	UNDEFINED = -1,
	// We started doing this from 4 onwards
	VERSION_04 = 4,
};

// Define most ahead of time
enum class PluginMinorVersion
{
	UNDEFINED = -1,
	VERSION_00 = 0,
	VERSION_01,
	VERSION_02,
	VERSION_03,
	VERSION_04,
	VERSION_05,
	VERSION_06,
	VERSION_07,
	VERSION_08,
	VERSION_09,
};

enum class ReturnCode
{
	Default = 0,
	SkipPlugins = 1,
	SkipFunctionCall = 2,
	SkipAll = SkipPlugins | SkipFunctionCall,
};

inline ReturnCode operator&(ReturnCode a, ReturnCode b)
{
	return ReturnCode(static_cast<uint>(a) & static_cast<uint>(b));
}

enum class HookStep
{
	Before,
	After,
	Mid,
	Count
};

enum class MessageColor
{
	AliceBlue = 0xF0F8FF,
	AntiqueWhite = 0xFAEBD7,
	Aqua = 0x00FFFF,
	Aquamarine = 0x7FFFD4,
	Azure = 0xF0FFFF,
	Beige = 0xF5F5DC,
	Bisque = 0xFFE4C4,
	Black = 0x000000,
	BlanchedAlmond = 0xFFEBCD,
	Blue = 0x0000FF,
	BlueViolet = 0x8A2BE2,
	Brown = 0xA52A2A,
	BurlyWood = 0xDEB887,
	CadetBlue = 0x5F9EA0,
	Chartreuse = 0x7FFF00,
	Chocolate = 0xD2691E,
	Coral = 0xFF7F50,
	CornflowerBlue = 0x6495ED,
	Cornsilk = 0xFFF8DC,
	Crimson = 0xDC143C,
	Cyan = 0x00FFFF,
	DarkBlue = 0x00008B,
	DarkCyan = 0x008B8B,
	DarkGoldenrod = 0xB8860B,
	DarkGray = 0xA9A9A9,
	DarkGreen = 0x006400,
	DarkKhaki = 0xBDB76B,
	DarkMagenta = 0x8B008B,
	DarkOliveGreen = 0x556B2F,
	DarkOrange = 0xFF8C00,
	DarkOrchid = 0x9932CC,
	DarkRed = 0x8B0000,
	DarkSalmon = 0xE9967A,
	DarkSeaGreen = 0x8FBC8F,
	DarkSlateBlue = 0x483D8B,
	DarkSlateGray = 0x2F4F4F,
	DarkTurquoise = 0x00CED1,
	DarkViolet = 0x9400D3,
	DeepPink = 0xFF1493,
	DeepSkyBlue = 0x00BFFF,
	DimGray = 0x696969,
	DodgerBlue = 0x1E90FF,
	Firebrick = 0xB22222,
	FloralWhite = 0xFFFAF0,
	ForestGreen = 0x228B22,
	Fuchsia = 0xFF00FF,
	Gainsboro = 0xDCDCDC,
	GhostWhite = 0xF8F8FF,
	Gold = 0xFFD700,
	Goldenrod = 0xDAA520,
	Gray = 0x808080,
	Green = 0x008000,
	GreenYellow = 0xADFF2F,
	Honeydew = 0xF0FFF0,
	HotPink = 0xFF69B4,
	IndianRed = 0xCD5C5C,
	Indigo = 0x4B0082,
	Ivory = 0xFFFFF0,
	Khaki = 0xF0E68C,
	Lavender = 0xE6E6FA,
	LavenderBlush = 0xFFF0F5,
	LawnGreen = 0x7CFC00,
	LemonChiffon = 0xFFFACD,
	LightBlue = 0xADD8E6,
	LightCoral = 0xF08080,
	LightCyan = 0xE0FFFF,
	LightGoldenrodYellow = 0xFAFAD2,
	LightGray = 0xD3D3D3,
	LightGreen = 0x90EE90,
	LightPink = 0xFFB6C1,
	LightSalmon = 0xFFA07A,
	LightSeaGreen = 0x20B2AA,
	LightSkyBlue = 0x87CEFA,
	LightSlateGray = 0x778899,
	LightSteelBlue = 0xB0C4DE,
	LightYellow = 0xFFFFE0,
	Lime = 0x00FF00,
	LimeGreen = 0x32CD32,
	Linen = 0xFAF0E6,
	Magenta = 0xFF00FF,
	Maroon = 0x800000,
	MediumAquamarine = 0x66CDAA,
	MediumBlue = 0x0000CD,
	MediumOrchid = 0xBA55D3,
	MediumPurple = 0x9370DB,
	MediumSeaGreen = 0x3CB371,
	MediumSlateBlue = 0x7B68EE,
	MediumSpringGreen = 0x00FA9A,
	MediumTurquoise = 0x48D1CC,
	MediumVioletRed = 0xC71585,
	MidnightBlue = 0x191970,
	MintCream = 0xF5FFFA,
	MistyRose = 0xFFE4E1,
	Moccasin = 0xFFE4B5,
	NavajoWhite = 0xFFDEAD,
	Navy = 0x000080,
	OldLace = 0xFDF5E6,
	Olive = 0x808000,
	OliveDrab = 0x6B8E23,
	Orange = 0xFFA500,
	OrangeRed = 0xFF4500,
	Orchid = 0xDA70D6,
	PaleGoldenrod = 0xEEE8AA,
	PaleGreen = 0x98FB98,
	PaleTurquoise = 0xAFEEEE,
	PaleVioletRed = 0xDB7093,
	PapayaWhip = 0xFFEFD5,
	PeachPuff = 0xFFDAB9,
	Peru = 0xCD853F,
	Pink = 0xFFC0CB,
	Plum = 0xDDA0DD,
	PowderBlue = 0xB0E0E6,
	Purple = 0x800080,
	Red = 0xFF0000,
	RosyBrown = 0xBC8F8F,
	RoyalBlue = 0x4169E1,
	SaddleBrown = 0x8B4513,
	Salmon = 0xFA8072,
	SandyBrown = 0xF4A460,
	SeaGreen = 0x2E8B57,
	SeaShell = 0xFFF5EE,
	Sienna = 0xA0522D,
	Silver = 0xC0C0C0,
	SkyBlue = 0x87CEEB,
	SlateBlue = 0x6A5ACD,
	SlateGray = 0x708090,
	Snow = 0xFFFAFA,
	SpringGreen = 0x00FF7F,
	SteelBlue = 0x4682B4,
	Tan = 0xD2B48C,
	Teal = 0x008080,
	Thistle = 0xD8BFD8,
	Tomato = 0xFF6347,
	Turquoise = 0x40E0D0,
	Violet = 0xEE82EE,
	Wheat = 0xF5DEB3,
	White = 0xFFFFFF,
	WhiteSmoke = 0xF5F5F5,
	Yellow = 0xFFFF00,
	YellowGreen = 0x9ACD32,
};

enum class MessageFormat
{
	Normal = 0x0,
	Bold = 0x1,
	Italic = 0x2,
	Underline = 0x4,
	Big = 0x8,
	BigAndWide = 0x10,
	VeryBig = 0x20,
	Smoothest = 0x40,
	Smoother = 0x80,
	Small = 0x90,

	BoldAndItalic = Bold | Italic,
	BoldAndItalicAndUnderline = Bold | Italic | Underline,
	BoldAndUnderline = Bold | Underline,
	ItalicAndUnderline = Italic | Underline,
	BigAndBold = Big | Bold,
};

enum class IdType
{
	Client,
	Ship = 1,
	Solar = 1,
	Equip,
	Arch,
	Reputation
};