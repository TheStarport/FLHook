#pragma once

class DLL FlCodec
{
	static std::string ReadFile(const std::string& input);
  public:
	FlCodec() = delete;

	static std::string Decode(const std::string& input);
	static std::string Encode(const std::string& input);
	static bool EncodeDecode(const std::string& input, const std::string& output, bool encode);
	static bool DecodeFile(const std::string& input, const std::string& outputFile);
	static bool EncodeFile(const std::string& input, const std::string& outputFile);
};