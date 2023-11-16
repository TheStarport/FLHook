#include "PCH.hpp"

#include "Core/ZoneUtilities.hpp"

ZoneUtilities::TransformMatrix ZoneUtilities::MultiplyMatrix(const TransformMatrix& mat1, const TransformMatrix& mat2)
{
    TransformMatrix result = { 0 };
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            for (int k = 0; k < 4; k++)
            {
                result.d[i][j] += mat1.d[i][k] * mat2.d[k][j];
            }
        }
    }
    return result;
}
ZoneUtilities::TransformMatrix ZoneUtilities::SetupTransform(const Vector& p, const Vector& r)
{
    // Convert degrees into radians

    const float ax = r.x * (static_cast<float>(std::numbers::pi) / 180);
    const float ay = r.y * (static_cast<float>(std::numbers::pi) / 180);
    const float az = r.z * (static_cast<float>(std::numbers::pi) / 180);

    // Initial matrix
    TransformMatrix smat = { 0 };
    smat.d[0][0] = smat.d[1][1] = smat.d[2][2] = smat.d[3][3] = 1;

    // Translation matrix
    TransformMatrix tmat;
    tmat.d[0][0] = 1;
    tmat.d[0][1] = 0;
    tmat.d[0][2] = 0;
    tmat.d[0][3] = 0;
    tmat.d[1][0] = 0;
    tmat.d[1][1] = 1;
    tmat.d[1][2] = 0;
    tmat.d[1][3] = 0;
    tmat.d[2][0] = 0;
    tmat.d[2][1] = 0;
    tmat.d[2][2] = 1;
    tmat.d[2][3] = 0;
    tmat.d[3][0] = -p.x;
    tmat.d[3][1] = -p.y;
    tmat.d[3][2] = -p.z;
    tmat.d[3][3] = 1;

    // X-axis rotation matrix
    TransformMatrix xmat;
    xmat.d[0][0] = 1;
    xmat.d[0][1] = 0;
    xmat.d[0][2] = 0;
    xmat.d[0][3] = 0;
    xmat.d[1][0] = 0;
    xmat.d[1][1] = std::cos(ax);
    xmat.d[1][2] = std::sin(ax);
    xmat.d[1][3] = 0;
    xmat.d[2][0] = 0;
    xmat.d[2][1] = -std::sin(ax);
    xmat.d[2][2] = std::cos(ax);
    xmat.d[2][3] = 0;
    xmat.d[3][0] = 0;
    xmat.d[3][1] = 0;
    xmat.d[3][2] = 0;
    xmat.d[3][3] = 1;

    // Y-axis rotation matrix
    TransformMatrix ymat;
    ymat.d[0][0] = std::cos(ay);
    ymat.d[0][1] = 0;
    ymat.d[0][2] = -std::sin(ay);
    ymat.d[0][3] = 0;
    ymat.d[1][0] = 0;
    ymat.d[1][1] = 1;
    ymat.d[1][2] = 0;
    ymat.d[1][3] = 0;
    ymat.d[2][0] = std::sin(ay);
    ymat.d[2][1] = 0;
    ymat.d[2][2] = std::cos(ay);
    ymat.d[2][3] = 0;
    ymat.d[3][0] = 0;
    ymat.d[3][1] = 0;
    ymat.d[3][2] = 0;
    ymat.d[3][3] = 1;

    // Z-axis rotation matrix
    TransformMatrix zmat;
    zmat.d[0][0] = std::cos(az);
    zmat.d[0][1] = std::sin(az);
    zmat.d[0][2] = 0;
    zmat.d[0][3] = 0;
    zmat.d[1][0] = -std::sin(az);
    zmat.d[1][1] = std::cos(az);
    zmat.d[1][2] = 0;
    zmat.d[1][3] = 0;
    zmat.d[2][0] = 0;
    zmat.d[2][1] = 0;
    zmat.d[2][2] = 1;
    zmat.d[2][3] = 0;
    zmat.d[3][0] = 0;
    zmat.d[3][1] = 0;
    zmat.d[3][2] = 0;
    zmat.d[3][3] = 1;

    TransformMatrix tm;
    tm = MultiplyMatrix(smat, tmat);
    tm = MultiplyMatrix(tm, xmat);
    tm = MultiplyMatrix(tm, ymat);
    tm = MultiplyMatrix(tm, zmat);

    return tm;
}

void ZoneUtilities::ReadLootableZone(const std::string& systemNick, const std::string& defaultZoneNick, const std::string& file)
{
    std::string path = "..\\data\\";
    path += file;

    INI_Reader ini;
    if (ini.open(path.c_str(), false))
    {
        while (ini.read_header())
        {
            if (ini.is_header("LootableZone"))
            {
                std::string zoneNick = defaultZoneNick;
                std::string crateNick = "";
                std::string lootNick = "";
                int minLoot = 0;
                int maxLoot = 0;
                uint lootDifficulty = 0;
                while (ini.read_value())
                {
                    if (ini.is_value("zone"))
                    {
                        zoneNick = StringUtils::ToLower(std::string(ini.get_value_string()));
                    }
                    else if (ini.is_value("dynamic_loot_container"))
                    {
                        crateNick = StringUtils::ToLower(std::string(ini.get_value_string()));
                    }
                    else if (ini.is_value("dynamic_loot_commodity"))
                    {
                        lootNick = StringUtils::ToLower(std::string(ini.get_value_string()));
                    }
                    else if (ini.is_value("dynamic_loot_count"))
                    {
                        minLoot = ini.get_value_int(0);
                        maxLoot = ini.get_value_int(1);
                    }
                    else if (ini.is_value("dynamic_loot_difficulty"))
                    {
                        lootDifficulty = ini.get_value_int(0);
                    }
                }

                LootableZone lz;
                lz.systemId = CreateID(systemNick.c_str());
                lz.zoneNick = StringUtils::stows(zoneNick);
                lz.lootNick = StringUtils::stows(lootNick);
                lz.lootId = CreateID(lootNick.c_str());
                lz.crateId = CreateID(crateNick.c_str());
                lz.minLoot = minLoot;
                lz.maxLoot = maxLoot;
                lz.lootDifficulty = lootDifficulty;
                lz.pos.x = lz.pos.y = lz.pos.z = 0;
                lz.size.x = lz.size.y = lz.size.z = 0;

                lootableZones.insert({ lz.systemId, lz });
            }
        }
        ini.close();
    }
}

void ZoneUtilities::ReadSystemZones(const std::string& systemNick, const std::string& file)
{
    std::string path = std::format(R"(..\data\universe\{})", file);

    INI_Reader ini;
    if (ini.open(path.c_str(), false))
    {
        while (ini.read_header())
        {
            if (ini.is_header("zone"))
            {
                std::wstring zoneNick = L"";
                Vector size = { 0, 0, 0 };
                Vector pos = { 0, 0, 0 };
                Vector rotation = { 0, 0, 0 };
                int damage = 0;
                bool encounter = false;

                while (ini.read_value())
                {
                    if (ini.is_value("nickname"))
                    {
                        zoneNick = StringUtils::stows(StringUtils::ToLower(std::string(ini.get_value_string())));
                    }
                    else if (ini.is_value("pos"))
                    {
                        pos.x = ini.get_value_float(0);
                        pos.y = ini.get_value_float(1);
                        pos.z = ini.get_value_float(2);
                    }
                    else if (ini.is_value("rotate"))
                    {
                        rotation.x = 180 + ini.get_value_float(0);
                        rotation.y = 180 + ini.get_value_float(1);
                        rotation.z = 180 + ini.get_value_float(2);
                    }
                    else if (ini.is_value("size"))
                    {
                        size.x = ini.get_value_float(0);
                        size.y = ini.get_value_float(1);
                        size.z = ini.get_value_float(2);
                        if (size.y == 0 || size.z == 0)
                        {
                            size.y = size.x;
                            size.z = size.x;
                        }
                    }
                    else if (ini.is_value("damage"))
                    {
                        damage = ini.get_value_int(0);
                    }
                    else if (ini.is_value("encounter"))
                    {
                        encounter = true;
                    }
                }

                Zone lz;
                lz.sysNick = StringUtils::stows(systemNick);
                lz.zoneNick = zoneNick;
                lz.systemId = CreateID(systemNick.c_str());
                lz.size = size;
                lz.pos = pos;
                lz.damage = damage;
                lz.encounter = encounter;
                lz.transform = SetupTransform(pos, rotation);
                allZones.insert({ lz.systemId, lz });
            }
            else if (ini.is_header("Object"))
            {
                std::string nickname;
                std::string jumpDestSysNick;
                bool isJump = false;

                while (ini.read_value())
                {
                    if (ini.is_value("nickname"))
                    {
                        nickname = StringUtils::ToLower(std::string(ini.get_value_string()));
                    }
                    else if (ini.is_value("goto"))
                    {
                        isJump = true;
                        jumpDestSysNick = ini.get_value_string(0);
                    }
                }

                if (isJump)
                {
                    JumpPoint jp;
                    jp.sysNick = StringUtils::stows(systemNick);
                    jp.jumpNick = StringUtils::stows(nickname);
                    jp.jumpDestSysNick = StringUtils::stows(jumpDestSysNick);
                    jp.system = CreateID(systemNick.c_str());
                    jp.jumpId = CreateID(nickname.c_str());
                    jp.jumpDestSysId = CreateID(jumpDestSysNick.c_str());
                    jumpPoints.insert({ jp.system, jp });
                }
            }
            else if (ini.is_header("Asteroids"))
            {
                std::string lootableZoneFile;
                std::string zoneNick;
                while (ini.read_value())
                {
                    if (ini.is_value("zone"))
                    {
                        zoneNick = StringUtils::ToLower(std::string(ini.get_value_string()));
                    }
                    if (ini.is_value("file"))
                    {
                        lootableZoneFile = ini.get_value_string();
                    }
                }
                ReadLootableZone(systemNick, zoneNick, lootableZoneFile);
            }
        }
        ini.close();
    }
}

void ZoneUtilities::ReadUniverse()
{
    allZones.clear();

    // Read all system ini files again this time extracting zone size/postion
    // information for the zone list.
    INI_Reader ini;
    if (ini.open(R"(..\data\universe\universe.ini)", false))
    {
        while (ini.read_header())
        {
            if (ini.is_header("System"))
            {
                std::string systemNick;
                std::string file;
                while (ini.read_value())
                {
                    if (ini.is_value("nickname"))
                    {
                        systemNick = StringUtils::ToLower(std::string(ini.get_value_string()));
                    }
                    else if (ini.is_value("file"))
                    {
                        file = ini.get_value_string();
                    }
                }

                ReadSystemZones(systemNick, file);
            }
        }
        ini.close();
    }

    // Read all system ini files again this time extracting zone size/postion
    // information for the lootable zone list.
    if (ini.open(R"(..\data\universe\universe.ini)", false))
    {
        while (ini.read_header())
        {
            if (ini.is_header("System"))
            {
                std::string systemNick = "";
                std::string file = "";
                while (ini.read_value())
                {
                    if (ini.is_value("nickname"))
                    {
                        systemNick = ini.get_value_string();
                    }
                    if (ini.is_value("file"))
                    {
                        file = ini.get_value_string();
                    }
                }

                ReadSystemZones(systemNick, file);
            }
        }
        ini.close();
    }
}

bool ZoneUtilities::InZone(uint system, const Vector& pos, Zone& rlz)
{
    // For each zone in the system test that pos is inside the
    // zone.
    const auto start = allZones.lower_bound(system);
    const auto end = allZones.upper_bound(system);
    for (auto i = start; i != end; ++i)
    {
        const Zone& lz = i->second;
        /** Transform the point pos onto coordinate system defined by matrix m
         */
        const float x = pos.x * lz.transform.d[0][0] + pos.y * lz.transform.d[1][0] + pos.z * lz.transform.d[2][0] + lz.transform.d[3][0];
        const float y = pos.x * lz.transform.d[0][1] + pos.y * lz.transform.d[1][1] + pos.z * lz.transform.d[2][1] + lz.transform.d[3][1];
        const float z = pos.x * lz.transform.d[0][2] + pos.y * lz.transform.d[1][2] + pos.z * lz.transform.d[2][2] + lz.transform.d[3][2];

        // If r is less than/equal to 1 then the point is inside the ellipsoid.
        const float result = sqrt(powf(x / lz.size.x, 2) + powf(y / lz.size.y, 2) + powf(z / lz.size.z, 2));
        if (result <= 1)
        {
            rlz = lz;
            return true;
        }
    }

    // Return the iterator. If the iter is allZones.end() then the point
    // is not in a lootable zone.
    return false;
}

std::optional<const ZoneUtilities::Zone*> ZoneUtilities::InDeathZone(uint system, const Vector& pos, const float requiredDamage)
{
    // For each zone in the system test that pos is inside the zone.
    const auto start = allZones.lower_bound(system);
    const auto end = allZones.upper_bound(system);
    for (auto i = start; i != end; ++i)
    {
        const Zone& lz = i->second;

        /** Transform the point pos onto coordinate system defined by matrix m
         */
        const float x = pos.x * lz.transform.d[0][0] + pos.y * lz.transform.d[1][0] + pos.z * lz.transform.d[2][0] + lz.transform.d[3][0];
        const float y = pos.x * lz.transform.d[0][1] + pos.y * lz.transform.d[1][1] + pos.z * lz.transform.d[2][1] + lz.transform.d[3][1];
        const float z = pos.x * lz.transform.d[0][2] + pos.y * lz.transform.d[1][2] + pos.z * lz.transform.d[2][2] + lz.transform.d[3][2];

        // If r is less than/equal to 1 then the point is inside the ellipsoid.
        const float result = sqrt(powf(x / lz.size.x, 2) + powf(y / lz.size.y, 2) + powf(z / lz.size.z, 2));
        if (result <= 1 && lz.damage > requiredDamage)
        {
            return {  &lz };
        }
    }

    return {};
}

void ZoneUtilities::Init()
{
    allZones.clear();
    lootableZones.clear();
    ReadUniverse();
}
