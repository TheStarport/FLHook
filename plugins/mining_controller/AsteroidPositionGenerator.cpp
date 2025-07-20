#include "MiningController.hpp"
#include "PCH.hpp"

#include "MiningController.hpp"

namespace Plugins
{
    enum direction
    {
        NoDir = 0,
        U = 1 << 0,
        D = 1 << 1,
        W = 1 << 2,
        E = 1 << 3,
        N = 1 << 4,
        S = 1 << 5
    };

    void ExploreZone(CmnAsteroid::CAsteroidField* field, Vector& pos, Matrix& rot, float size, direction dir)
    {
        if (!field->near_field(pos))
        {
            return;
        }
        field->populate_asteroids(pos, pos);
        if (dir == NoDir || !(dir & (N | E)))
        {
            Vector vecW = pos;
            vecW.TranslateX(rot, size);
            ExploreZone(field, vecW, rot, size, (direction)(dir | W));
        }
        if (dir == NoDir || !(dir & (S | W)))
        {
            Vector vecE = pos;
            vecE.TranslateX(rot, -size);
            ExploreZone(field, vecE, rot, size, (direction)(dir | E));
        }
        if (dir == NoDir || !(dir & (S | E)))
        {
            Vector vecN = pos;
            vecN.TranslateY(rot, size);
            ExploreZone(field, vecN, rot, size, (direction)(dir | N));
        }
        if (dir == NoDir || !(dir & (N | W)))
        {
            Vector vecS = pos;
            vecS.TranslateY(rot, -size);
            ExploreZone(field, vecS, rot, size, (direction)(dir | S));
        }
        if (dir == NoDir || dir == U)
        {
            Vector vecU = pos;
            vecU.TranslateZ(rot, size);
            ExploreZone(field, vecU, rot, size, U);
        }
        if (dir == NoDir || dir == D)
        {
            Vector vecD = pos;
            vecD.TranslateZ(rot, -size);
            ExploreZone(field, vecD, rot, size, D);
        }
    }

    void MiningControllerPlugin::LogAsteroidField(std::wstring_view& zoneNick)
    {
        Id zoneHash = Id(zoneNick);
        const Universe::IZone* iZone = Universe::get_zone(zoneHash.GetValue());
        if (!iZone)
        {
            WARN("Zone Not Found\n");
            return;
        }

        CmnAsteroid::CAsteroidSystem* asteroidSystem = CmnAsteroid::Find(iZone->systemId.GetValue());
        if (!asteroidSystem)
        {
            WARN("Asteroid System Error\n");
            return;
        }

        CmnAsteroid::CAsteroidField* field = asteroidSystem->FindFirst();
        while (field)
        {
            if (field->zone->zoneId == zoneHash)
            {
                break;
            }
            field = asteroidSystem->FindNext();
        }

        if (!field)
        {
            return;
        }

        float cubeSize = (float)field->get_cube_size();
        uint lastCount = 0;
        Vector initialPos = field->closest_cube_pos(field->zone->position);
        ExploreZone(field, initialPos, field->zone->orientation, cubeSize, NoDir);

        FILE* fLog = fopen("./Asteroids.log", "w");

        CAsteroid* cobj = reinterpret_cast<CAsteroid*>(CObject::FindFirst(CObject::CASTEROID_OBJECT));
        while (cobj)
        {
            fprintf(fLog, "%0.0f %0.0f %0.0f\n", cobj->position.x, cobj->position.y, cobj->position.z);
            cobj = reinterpret_cast<CAsteroid*>(CObject::FindNext());
        }
        fclose(fLog);
    }
} // namespace Plugins
