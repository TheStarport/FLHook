#pragma once

class FLHook;
class IServerImplHook;
class IEngineHook;
class InternalApi
{
        friend UserCommandProcessor;
        friend FLHook;
        friend IClientImpl;
        friend IEngineHook;
        friend IServerImplHook;
        friend ClientId;
        friend ShipId;
        friend BaseId;
        friend RepId;
        friend RepGroupId;
        friend ObjectId;
        friend SystemId;

        static Action<void, Error> FMsgEncodeXml(std::wstring_view xml, char* buffer, uint size, uint& ret);
        static void FMsgSendChat(ClientId client, char* buffer, uint size);
        static Action<void, Error> SendMessage(ClientId to, std::wstring_view message, ClientId from = ClientId(), std::wstring_view = L"");

        static uint CreateID(const std::wstring& nickname);
};
