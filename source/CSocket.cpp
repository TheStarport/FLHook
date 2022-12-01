#include "CSocket.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CSocket::DoPrint(const std::wstring& textIn)
{
	std::wstring text(textIn);
	for (uint i = 0; (i < text.length()); i++)
	{
		if (text[i] == '\n')
		{
			text.replace(i, 1, L"\r\n");
			i++;
		}
	}

	int iSndBuf = 300000; // fix: set send-buffer to this size (bytes)
	int size = sizeof(int);
	setsockopt(this->s, SOL_SOCKET, SO_SNDBUF, (const char*)&iSndBuf, size);

	if (bEncrypted)
	{
		int iLen;
		char* data;
		const char* tempData;
		if (bUnicode)
		{
			uint iRem = (uint)(text.length() % 4);
			if (iRem) // Data to be encrypted is not a multiple of 8 bytes, add
			          // 0x00s to compensate
			{
				iRem = 4 - iRem;
				text.resize(text.length() + iRem, L'\x00');
			}
			tempData = (const char*)text.data();
			iLen = (uint)(text.length() * 2);
			data = (char*)malloc(iLen);
			memcpy(data, tempData, iLen);
		}
		else
		{
			uint iRem = (uint)(text.length() % 8);
			if (iRem)
			{
				iRem = 8 - iRem;
				std::string scText = wstos(text);
				scText.resize(scText.length() + iRem, '\x00');
				tempData = scText.data();
				iLen = (int)scText.length();
				data = (char*)malloc(iLen);
				memcpy(data, tempData, iLen);
			}
			else
			{
				std::string scText = wstos(text);
				tempData = scText.data();
				iLen = (int)text.length();
				data = (char*)malloc(iLen);
				memcpy(data, tempData, iLen);
			}
		}

		SwapBytes(data, iLen);
		if (Blowfish_Encrypt(bfc, data, iLen))
		{
			SwapBytes(data, iLen);
			send(this->s, data, iLen, 0);
			free(data);
		}
	}
	else
	{
		if (bUnicode)
			send(this->s, (const char*)text.c_str(), (int)text.length() * 2, 0);
		else
			send(this->s, wstos(text).c_str(), (int)text.length(), 0);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::wstring CSocket::GetAdminName()
{
	std::wstring wscReturn = L"Socket connection (";
	wscReturn += stows(this->sIP);
	wscReturn += L":";
	wscReturn += std::to_wstring(this->iPort);
	wscReturn += L")";
	return wscReturn;
}
