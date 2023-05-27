#include "PCH.hpp"

#include "Global.hpp"

#include "Helpers/Chat.hpp"

#include "Defs/FLHookConfig.hpp"
#include "Helpers/Client.hpp"
#include "Helpers/Math.hpp"
#include "Helpers/Player.hpp"


// Very hacky way to call non-header function
namespace IServerImplHook
{
	void __stdcall SubmitChat(CHAT_ID cidFrom, ulong size, const void* rdlReader, CHAT_ID cidTo, int _genArg1);
}

bool g_bMsg;
bool g_bMsgU;
bool g_bMsgS;

_RCSendChatMsg RCSendChatMsg;

namespace Hk::Chat
{
	cpp::result<void, Error> Msg(const std::variant<uint, std::wstring>& player, const std::wstring& message)
	{
		ClientId client = Client::ExtractClientID(player);

		if (client == UINT_MAX)
			return cpp::fail(Error::PlayerNotLoggedIn);

		const CHAT_ID ci = {0};
		const CHAT_ID ciClient = {client};

		const std::wstring XML = L"<TRA data=\"0x19BD3A00\" mask=\"-1\"/><TEXT>" + XMLText(message) + L"</TEXT>";
		uint retVal;
		char buf[1024];
		if (const auto err = FMsgEncodeXML(XML, buf, sizeof(buf), retVal); err.has_error())
			return cpp::fail(err.error());

		IServerImplHook::SubmitChat(ci, retVal, buf, ciClient, -1);
		return {};
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	cpp::result<void, Error> MsgS(const std::variant<std::wstring, uint>& system, const std::wstring& message)
	{
		uint systemId = 0;
		if (!system.index())
		{
			const auto systemName = std::get<std::wstring>(system);
			pub::GetSystemID(systemId, StringUtils::wstos(systemName).c_str());
		}
		else
		{
			systemId = std::get<uint>(system);
		}

		// prepare xml
		const std::wstring xml = L"<TRA data=\"0xE6C68400\" mask=\"-1\"/><TEXT>" + XMLText(message) + L"</TEXT>";
		uint retVal;
		char buffer[1024];
		if (const auto err = FMsgEncodeXML(xml, buffer, sizeof(buffer), retVal); err.has_error())
			return cpp::fail(err.error());

		const CHAT_ID ci = {0};

		// for all players in system...

		for (const auto player : Client::getAllPlayersInSystem(systemId))
		{
			const CHAT_ID ciClient = {player};
			IServerImplHook::SubmitChat(ci, retVal, buffer, ciClient, -1);
		}

		return {};
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	cpp::result<void, Error> MsgU(const std::wstring& message)
	{
		const CHAT_ID ci = {0};
		const CHAT_ID ciClient = {0x00010000};

		const std::wstring xml = L"<TRA font=\"1\" color=\"#FFFFFF\"/><TEXT>" + XMLText(message) + L"</TEXT>";
		uint retVal;
		char buf[1024];
		if (const auto err = FMsgEncodeXML(xml, buf, sizeof(buf), retVal); err.has_error())
			return cpp::fail(err.error());

		IServerImplHook::SubmitChat(ci, retVal, buf, ciClient, -1);
		return {};
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	cpp::result<void, Error> FMsgEncodeXML(const std::wstring& xmring, char* buffer, uint size, uint& ret)
	{
		XMLReader rdr;
		RenderDisplayList rdl;
		std::wstring msg = L"<?xml version=\"1.0\" encoding=\"UTF-16\"?><RDL><PUSH/>";
		msg += xmring;
		msg += L"<PARA/><POP/></RDL>\x000A\x000A";
		if (!rdr.read_buffer(rdl, (const char*)msg.c_str(), msg.length() * 2))
			return cpp::fail(Error::WrongXmlSyntax);

		BinaryRDLWriter rdlwrite;
		rdlwrite.write_buffer(rdl, buffer, size, ret);
		return {};
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void FMsgSendChat(ClientId client, char* buffer, uint size)
	{
		auto p4 = (uint)buffer;
		uint p3 = size;
		uint p2 = 0x00010000;
		uint p1 = client;

		__asm {
			push [p4]
			push [p3]
			push [p2]
			push [p1]
			mov ecx, [HookClient]
			add ecx, 4
			call [RCSendChatMsg]
			}
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	cpp::result<void, Error> FMsg(ClientId client, const std::wstring& xmring)
	{
		char buf[0xFFFF];
		uint ret;
		if (const auto err = FMsgEncodeXML(xmring, buf, sizeof(buf), ret); err.has_error())
			return cpp::fail(err.error());

		FMsgSendChat(client, buf, ret);
		return {};
	}

	cpp::result<void, Error> FMsg(const std::variant<uint, std::wstring>& player, const std::wstring& xmring)
	{
		ClientId client = Client::ExtractClientID(player);

		if (client == UINT_MAX)
			return cpp::fail(Error::PlayerNotLoggedIn);

		return FMsg(client, xmring);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	cpp::result<void, Error> FMsgS(const std::variant<std::wstring, uint>& system, const std::wstring& xmring)
	{
		uint systemId = 0;
		if (!system.index())
		{
			const auto systemName = std::get<std::wstring>(system);
			pub::GetSystemID(systemId, StringUtils::wstos(systemName).c_str());
		}
		else
		{
			systemId = std::get<uint>(system);
		}
		// encode xml std::string
		char buf[0xFFFF];
		uint ret;
		if (const auto err = FMsgEncodeXML(xmring, buf, sizeof(buf), ret); err.has_error())
			return cpp::fail(err.error());

		// for all players in system...
		for (const auto player : Client::getAllPlayersInSystem(systemId))
		{
			FMsgSendChat(player, buf, ret);
		}

		return {};
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	cpp::result<void, Error> FMsgU(const std::wstring& xmring)
	{
		// encode xml std::string
		char buf[0xFFFF];
		uint ret;
		const auto err = FMsgEncodeXML(xmring, buf, sizeof(buf), ret);
		if (err.has_error())
			return cpp::fail(err.error());

		// for all players
		PlayerData* playerDb = nullptr;
		while ((playerDb = Players.traverse_active(playerDb)))
		{
			ClientId client = playerDb->onlineId;
			FMsgSendChat(client, buf, ret);
		}

		return {};
	}

	/** Format a chat std::string in accordance with the receiver's preferences and
	send it. Will check that the receiver accepts chatConfig from sender and
	refuses to send if necessary. */
	cpp::result<void, Error> FormatSendChat(uint toClientId, const std::wstring& sender, const std::wstring& text, const std::wstring& textColor)
	{
		#define HAS_FLAG(a, b) ((a).flags.find(b) != -1)

		if (FLHookConfig::i()->userCommands.userCmdIgnore)
		{
			for (const auto& ignore : ClientInfo[toClientId].ignoreInfoList)
			{
				if (!HAS_FLAG(ignore, L"i") && !(ToLower(sender).compare(ToLower(ignore.character))))
					return {}; // ignored
				if (HAS_FLAG(ignore, L"i") && (ToLower(sender).find(ToLower(ignore.character)) != -1))
					return {};
				// ignored
			}
		}

		uchar format;
		// adjust chatsize
		switch (ClientInfo[toClientId].chatSize)
		{
			case CS_SMALL:
				format = 0x90;
				break;
			case CS_BIG:
				format = 0x10;
				break;
			default:
				format = 0x00;
				break;
		}

		// adjust chatstyle
		switch (ClientInfo[toClientId].chatStyle)
		{
			case CST_BOLD:
				format += 0x01;
				break;
			case CST_ITALIC:
				format += 0x02;
				break;
			case CST_UNDERLINE:
				format += 0x04;
				break;
			default:
				format += 0x00;
				break;
		}

		wchar_t wFormatBuf[8];
		swprintf(wFormatBuf, _countof(wFormatBuf), L"%02X", format);
		const std::wstring TRADataFormat = wFormatBuf;
		const std::wstring TRADataSenderColor = L"FFFFFF"; // white

		const std::wstring XML = L"<TRA data=\"0x" + TRADataSenderColor + TRADataFormat + L"\" mask=\"-1\"/><TEXT>" + XMLText(sender) +
			L": </TEXT>" + L"<TRA data=\"0x" + textColor + TRADataFormat + L"\" mask=\"-1\"/><TEXT>" + XMLText(text) + L"</TEXT>";

		if (const auto err = FMsg(toClientId, XML); err.has_error())
			return cpp::fail(err.error());

		return {};
	}

	/** Send a player to player message */
	cpp::result<void, Error> SendPrivateChat(uint fromClientId, uint toClientId, const std::wstring& text)
	{
		const auto Sender = Client::GetCharacterNameByID(fromClientId);
		if (Sender.has_error())
		{
			Logger::i()->Log(LogLevel::Err, std::format("Unable to send private chat message from client {}", fromClientId));
			return {};
		}

		if (FLHookConfig::i()->userCommands.userCmdIgnore)
		{
			for (const auto& ignore : ClientInfo[toClientId].ignoreInfoList)
			{
				if (HAS_FLAG(ignore, L"p"))
					return {};
			}
		}

		// Send the message to both the sender and receiver.
		auto err = FormatSendChat(toClientId, Sender.value(), text, L"19BD3A");
		if (err.has_error())
			return cpp::fail(err.error());

		err = FormatSendChat(fromClientId, Sender.value(), text, L"19BD3A");
		if (err.has_error())
			return cpp::fail(err.error());

		return {};
	}

	/** Send a player to system message */
	void SendSystemChat(uint fromClientId, const std::wstring& text)
	{
		const std::wstring Sender = (const wchar_t*)Players.GetActiveCharacterName(fromClientId);

		// Get the player's current system.
		uint systemId;
		pub::Player::GetSystem(fromClientId, systemId);

		// For all players in system...
		for (const auto player : Client::getAllPlayersInSystem(systemId))
		{
			// Send the message a player in this system.
			FormatSendChat(player, Sender, text, L"E6C684");
		}
	}

	/** Send a player to local system message */
	void SendLocalSystemChat(uint fromClientId, const std::wstring& text)
	{
		// Don't even try to send an empty message
		if (text.empty())
		{
			return;
		}

		const auto Sender = Client::GetCharacterNameByID(fromClientId);
		if (Sender.has_error())
		{
			Logger::i()->Log(LogLevel::Err, std::format("Unable to send local system chat message from client {}", fromClientId));
			return;
		}

		// Get the player's current system and location in the system.
		uint systemId;
		pub::Player::GetSystem(fromClientId, systemId);

		uint fromShip;
		pub::Player::GetShip(fromClientId, fromShip);

		Vector fromShipLoc;
		Matrix fromShipDir;
		pub::SpaceObj::GetLocation(fromShip, fromShipLoc, fromShipDir);

		// For all players in system...
		for (auto player : Client::getAllPlayersInSystem(systemId))
		{
			uint ship;
			pub::Player::GetShip(player, ship);

			Vector shipLoc;
			Matrix shipDir;
			pub::SpaceObj::GetLocation(ship, shipLoc, shipDir);

			// Cheat in the distance calculation. Ignore the y-axis
			// Is player within scanner range (15K) of the sending char.
			if (static_cast<float>(sqrt(pow(shipLoc.x - fromShipLoc.x, 2) + pow(shipLoc.z - fromShipLoc.z, 2))) > 14999.0f)
				continue;

			// Send the message a player in this system.
			FormatSendChat(player, Sender.value(), text, L"FF8F40");
		}
	}

	/** Send a player to group message */
	void SendGroupChat(uint fromClientId, const std::wstring& text)
	{
		auto Sender = (const wchar_t*)Players.GetActiveCharacterName(fromClientId);
		// Format and send the message a player in this group.
		auto Members = Hk::Player::GetGroupMembers(Sender);
		if (Members.has_error())
		{
			return;
		}

		for (const auto& gm : Members.value())
		{
			FormatSendChat(gm.client, Sender, text, L"FF7BFF");
		}
	}

	std::vector<HINSTANCE> vDLLs;

	void UnloadStringDLLs()
	{
		for (uint i = 0; i < vDLLs.size(); i++)
			FreeLibrary(vDLLs[i]);
		vDLLs.clear();
	}

	void LoadStringDLLs()
	{
		UnloadStringDLLs();

		HINSTANCE hDLL = LoadLibraryEx("resources.dll",
			nullptr,
			LOAD_LIBRARY_AS_DATAFILE); // typically resources.dll
		if (hDLL)
			vDLLs.push_back(hDLL);

		INI_Reader ini;
		if (ini.open("freelancer.ini", false))
		{
			while (ini.read_header())
			{
				if (ini.is_header("Resources"))
				{
					while (ini.read_value())
					{
						if (ini.is_value("DLL"))
						{
							hDLL = LoadLibraryEx(ini.get_value_string(0), nullptr, LOAD_LIBRARY_AS_DATAFILE);
							if (hDLL)
								vDLLs.push_back(hDLL);
						}
					}
				}
			}
			ini.close();
		}
	}

	std::wstring GetWStringFromIdS(uint idS)
	{
		if (wchar_t buf[1024]; LoadStringW(vDLLs[idS >> 16], idS & 0xFFFF, buf, 1024))
			return buf;
		return L"";
	}

	std::wstring FormatMsg(MessageColor color, MessageFormat format, const std::wstring& msg)
	{
		const uint bgrColor = Math::RgbToBgr(static_cast<uint>(color));
		const std::wstring tra = Math::UintToHexString(bgrColor, 6, true) + Math::UintToHexString(static_cast<uint>(format), 2);

		return L"<TRA data=\"" + tra + L"\" mask=\"-1\"/><TEXT>" + XMLText(msg) + L"</TEXT>";
	}
} // namespace Hk::Chat

