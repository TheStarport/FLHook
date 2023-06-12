#include "PCH.hpp"

#include "Global.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"

namespace Hk::Math
{
    float Distance3D(Vector v1, Vector v2)
    {
        const float sq1 = v1.x - v2.x;
        const float sq2 = v1.y - v2.y;
        const float sq3 = v1.z - v2.z;
        return sqrt(sq1 * sq1 + sq2 * sq2 + sq3 * sq3);
    }

    cpp::result<float, Error> Distance3DByShip(uint ship1, uint ship2)
    {
        Vector v1;
        Matrix m1;
        pub::SpaceObj::GetLocation(ship1, v1, m1);

        if (v1.x == 0.f && v1.y == 0.f && v1.z == 0.f)
        {
            return { cpp::fail(Error::InvalidShip) };
        }

        Vector v2;
        Matrix m2;
        pub::SpaceObj::GetLocation(ship2, v2, m2);

        if (v2.x == 0.f && v2.y == 0.f && v2.z == 0.f)
        {
            return { cpp::fail(Error::InvalidShip) };
        }

        return Distance3D(v1, v2);
    }

    Quaternion MatrixToQuaternion(const Matrix& m) { return Quaternion(glm::quat_cast(m)); }

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

    constexpr float PI = std::numbers::pi_v<float>;

    // Convert radians to degrees.
    float Degrees(float rad)
    {
        rad *= 180 / PI;

        // Prevent displaying -0 and prefer 180 to -180.
        if (rad < 0)
        {
            if (rad > -0.005f)
            {
                rad = 0;
            }
            else if (rad <= -179.995f)
            {
                rad = 180;
            }
        }

        // Round to two decimal places here, so %g can display it without decimals.
        if (const float frac = modff(rad * 100, &rad); frac >= 0.5f)
        {
            ++rad;
        }
        else if (frac <= -0.5f)
        {
            --rad;
        }

        return rad / 100;
    }

    // Convert an orientation matrix to a pitch/yaw/roll vector.  Based on what
    // Freelancer does for the save game.
    Vector MatrixToEuler(const Matrix& mat)
    {
        const Quaternion quat(glm::quat_cast(mat));
        const auto vec = glm::eulerAngles(quat);
        const Vector eulerVector = { vec.x, vec.y, vec.z };
        return eulerVector;
    }

    uint RgbToBgr(uint color) { return color & 0xFF000000 | (color & 0xFF0000) >> 16 | color & 0x00FF00 | (color & 0x0000FF) << 16; };

    std::wstring UintToHexString(uint number, uint width, bool addPrefix)
    {
        std::wstringstream stream;
        if (addPrefix)
        {
            stream << L"0x";
        }
        stream << std::setfill(L'0') << std::setw(width) << std::hex << number;
        return stream.str();
    }
} // namespace Hk::Math
