#pragma once

class DLL DataManager : public Singleton<DataManager>
{
	std::string dataPath;
	std::map<EquipId, Light> lights;

  public:
	DataManager()
	{
		INI_Reader ini;
		ini.open("freelancer.ini", false);
		ini.find_header("Freelancer");
		while (ini.read_value())
		{
			if (ini.is_value("data path"))
			{
				dataPath = ini.get_value_string();
				break;
			}
		}
		ini.close();
	}

	DataManager(const DataManager&) = delete;

#ifdef FLHOOK
	void LoadLights();
#endif

	// Lights
	const std::map<EquipId, Light>& GetLights() const;
	cpp::result<const Light&, Error> FindLightByHash(EquipId hash) const;
};