#include "PCH.hpp"
/*
Freelancer .FL Savegame encode/decoder

Credits to Sherlog <sherlog@t-online.de> for finding out the algorithm

(c) 2003 by Jor <flcodec@jors.net>

This is free software. Permission to copy, store and use granted as long
as this copyright note remains intact.

Compilation in a POSIX environment:

   cc -O -o flcodec flcodec.c

Or in Wintendo 32 (get the free lcc compiler):

   lcc -O flcodec.c
   lcclnk -o flcodec.exe flcodec.obj

*******
EDITED by mc_horst for use in FLHook
Updated 2022 to use proper C++ syntax and work in memory ~ Laz

*/

#include "Helpers/FlCodec.hpp"


/* Very Secret Key - this is Microsoft Security In Action[tm] */
const char gene[] = "Gene";

std::string FlCodec::ReadFile(const std::string& input)
{
	std::ifstream file(input, std::ios::binary);

	if (!file.is_open() || !file.good())
		return {};

	file.unsetf(std::ios::skipws);

	std::streampos fileSize;

	file.seekg(0, std::ios::end);
	fileSize = file.tellg();
	file.seekg(0, std::ios::beg);

	std::string vec;
	vec.reserve(static_cast<uint>(fileSize));

	vec.insert(vec.begin(), std::istream_iterator<BYTE>(file), std::istream_iterator<BYTE>());
	return vec;
}

std::string FlCodec::Decode(const std::string& input)
{
	if (!input.starts_with("FLS1"))
	{
		return {};
	}

	std::string output;

	int i = 0;
	const int length = input.size() - 4;
	while (i < length)
	{
		const auto c = static_cast<byte>(((&input[0]) + 4)[i]);
		const auto k = static_cast<byte>((gene[i % 4] + i) % 256);

		const byte r = c ^ (k | 0x80);

		output += r;

		i++;
	}

	return output;
}

std::string FlCodec::Encode(const std::string& input)
{
	// Create our output vector, start with the magic string.
	std::string output = {'F', 'L', 'S', '1'};

	const uint length = input.size();

	uint i = 0;
	while (i < length)
	{
		const byte c = (input.data())[i];
		const auto k = static_cast<byte>((gene[i % 4] + i) % 256);

		const byte r = c ^ (k | 0x80);

		output += r;

		i++;
	}

	return output;
}

bool FlCodec::EncodeDecode(const std::string& input, const std::string& output, bool encode)
{
	auto undecodedBytes = ReadFile(input);
	const auto decodedBytes = encode ? Encode(undecodedBytes) : Decode(undecodedBytes);

	if (decodedBytes.empty())
	{
		return false;
	}

	std::ofstream outputFile(output, std::ios::out | std::ios::binary);
	outputFile.write(decodedBytes.data(), static_cast<std::streamsize>(decodedBytes.size()) * sizeof(byte));
	outputFile.close();

	return true;
}

bool FlCodec::DecodeFile(const std::string& input, const std::string& outputFile)
{
	return EncodeDecode(input, outputFile, false);
}

bool FlCodec::EncodeFile(const std::string& input, const std::string& outputFile)
{
	return EncodeDecode(input, outputFile, true);
}