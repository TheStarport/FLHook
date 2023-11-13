#pragma once

class DLL FlCodec
{
	static std::wstring ReadFile(const std::wstring& input);
  public:
	FlCodec() = delete;

	static std::wstring Decode(std::wstring_view input);
	static std::wstring Encode(std::wstring_view input);
	static bool EncodeDecode(const std::wstring& input, const std::wstring& output, bool encode);
	static bool DecodeFile(const std::wstring& input, const std::wstring& outputFile);
	static bool EncodeFile(const std::wstring& input, const std::wstring& outputFile);
};