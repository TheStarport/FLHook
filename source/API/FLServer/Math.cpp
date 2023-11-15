#include "PCH.hpp"

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"

namespace Hk::Math
{
    template <typename Str>
    Str VectorToSectorCoord(uint systemId, Vector pos)
    {
        float scale = 1.0;
        if (const Universe::ISystem* system = Universe::get_system(systemId))
        {
            scale = system->navMapScale;
        }

        const float gridSize = 34000.0f / scale;
        int gridRefX = static_cast<int>((pos.x + gridSize * 5) / gridSize) - 1;
        int gridRefZ = static_cast<int>((pos.z + gridSize * 5) / gridSize) - 1;

        gridRefX = std::min(std::max(gridRefX, 0), 7);
        char xPos = 'A' + static_cast<char>(gridRefX);

        gridRefZ = std::min(std::max(gridRefZ, 0), 7);
        char zPos = '1' + static_cast<char>(gridRefZ);

        Str currentLocation;
        if constexpr (std::is_same_v<Str, std::wstring>)
        {
            currentLocation = std::format(L"{}-{}", xPos, zPos);
        }
        else
        {
            currentLocation = std::format(L"{}-{}", xPos, zPos);
        }

        return currentLocation;
    }
} // namespace Hk::Math
