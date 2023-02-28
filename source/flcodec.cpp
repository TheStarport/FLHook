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

#include <fstream>
#include "../include/Tools/Typedefs.hpp"

/* Very Secret Key - this is Microsoft Security In Action[tm] */
const char gene[] = "Gene";

std::string ReadFile(const char* input)
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

std::string FlcDecode(std::string& input)
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
		auto c = static_cast<BYTE>(((&input[0]) + 4)[i]);
		auto k = static_cast<BYTE>((gene[i % 4] + i) % 256);
		
		BYTE r = c ^ (k | 0x80);

		output += r;

		i++;
	}

	return output;
}

std::string FlcEncode(std::string& input)
{
	// Create our output vector, start with the magic string.
	std::string output = {'F', 'L', 'S', '1'};

	const int length = input.size();

	int i = 0;
	while (i < length)
	{
		BYTE c = (&input[0])[i];
		auto k = static_cast<BYTE>((gene[i % 4] + i) % 256);

		BYTE r = c ^ (k | 0x80);

		output += r;

		i++;
	}

	return output;
}

bool EncodeDecode(const char* input, const char* output, bool encode)
{
	auto undecodedBytes = ReadFile(input);
	const auto decodedBytes = encode ? FlcEncode(undecodedBytes) : FlcDecode(undecodedBytes);

	if (decodedBytes.empty())
	{
		return false;
	}

	std::ofstream outputFile(output, std::ios::out | std::ios::binary);
	outputFile.write((char*)&decodedBytes[0], decodedBytes.size() * sizeof(BYTE));
	outputFile.close();

	return true;
}

bool FlcDecodeFile(const char* input, const char* outputFile)
{
	return EncodeDecode(input, outputFile, false);
}

bool FlcEncodeFile(const char* input, const char* outputFile)
{
	return EncodeDecode(input, outputFile, true);
}