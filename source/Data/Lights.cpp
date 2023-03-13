#include "Global.hpp"

void DataManager::LoadLights()
{
	INI_Reader ini;
	if (!ini.open("freelancer.ini", false))
		return;

	std::vector<std::string> equipmentFiles;

	ini.find_header("Data");
	while (ini.read_value())
	{
		if (ini.is_value("equipment"))
		{
			equipmentFiles.emplace_back(ini.get_value_string());
		}
	}

	ini.close();

	for (const auto& file : equipmentFiles)
	{
		if (!ini.open(std::format("{}\\{}", this->dataPath, file).c_str(), false))
			continue;

		while (ini.read_header())
		{
			if (!ini.is_header("Light"))
				continue;

			Light light;

			while (ini.read_value())
			{
				if (ini.is_value("nickname"))
				{
					light.nickname = ini.get_value_string();
					light.archId = CreateID(light.nickname.c_str());
				}
				else if (ini.is_value("inherit"))
				{
					const auto inherited = lights.find(CreateID(ini.get_value_string()));
					if (inherited == lights.end())
						continue;

					light.alwaysOn = inherited->second.alwaysOn;
					light.bulbSize = inherited->second.bulbSize;
					light.glowSize = inherited->second.glowSize;
					light.intensity = inherited->second.intensity;
					light.glowColor = inherited->second.glowColor;
					light.color = inherited->second.color;
					light.minColor = inherited->second.minColor;
					light.dockingLight = inherited->second.dockingLight;
					light.flareConeMin = inherited->second.flareConeMin;
					light.flareConeMax = inherited->second.flareConeMax;
					light.lightSourceCone = inherited->second.lightSourceCone;
					light.blinks = inherited->second.blinks;
					light.delay = inherited->second.delay;
					light.blinkDuration = inherited->second.blinkDuration;
				}
				else if (ini.is_value("bulb_size"))
				{
					light.bulbSize = ini.get_value_float(0);
				}
				else if (ini.is_value("glow_size"))
				{
					light.glowSize = ini.get_value_float(0);
				}
				else if (ini.is_value("flare_cone"))
				{
					light.flareConeMin = ini.get_value_float(0);
					light.flareConeMax = ini.get_value_float(1);
				}
				else if (ini.is_value("blink_duration"))
				{
					light.blinks = true;
					light.blinkDuration = ini.get_value_float(0);
				}
				else if (ini.is_value("avg_delay"))
				{
					light.delay = ini.get_value_float(0);
				}
				else if (ini.is_value("color"))
				{
					light.color = ini.get_vector();
				}
				else if (ini.is_value("min_color"))
				{
					light.minColor = ini.get_vector();
				}
				else if (ini.is_value("glow_color"))
				{
					light.glowColor = ini.get_vector();
				}
				else if (ini.is_value("intensity"))
				{
					light.intensity = ini.get_value_float(0);
				}
				else if (ini.is_value("lightsource_cone"))
				{
					light.lightSourceCone = ini.get_value_int(0);
				}
				else if (ini.is_value("always_on"))
				{
					light.alwaysOn = ini.get_value_bool(0);
				}
				else if (ini.is_value("docking_light"))
				{
					light.dockingLight = ini.get_value_bool(0);
				}
			}

			if (!light.nickname.empty())
				lights[light.archId] = std::move(light);
		}
	}

	Console::ConInfo(std::format("Loaded {} Lights.", lights.size()));
}

const std::map<EquipId, Light>& DataManager::GetLights() const
{
	return lights;
}

cpp::result<const Light&, Error> DataManager::FindLightByHash(EquipId hash) const
{
	const auto light = lights.find(hash);
	if (light == lights.end())
	{
		return cpp::fail(Error::NicknameNotFound);
	}

	return light->second;
}
