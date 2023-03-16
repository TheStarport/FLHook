#pragma once

namespace Hk::Personalities
{
	DLL cpp::result<pub::AI::Personality, Error> GetPersonality(const std::string& pilotNickname);
}