#pragma once

class ShipId;
class SystemId;
class FLHook;
class ZoneUtilities
{
        friend ShipId;
        friend SystemId;
        friend FLHook;

        struct TransformMatrix
        {
                float d[4][4];
        };

        struct Zone
        {
                /** The system nickname */
                std::wstring sysNick;

                /** The zone nickname */
                std::wstring zoneNick;

                /** The id of the system for this zone */
                uint systemId;

                /** The zone transformation matrix */
                TransformMatrix transform;

                /** The zone ellipsoid size */
                Vector size;

                /** The zone position */
                Vector pos;

                /** The damage this zone causes per second */
                int damage;

                /** Is this an encounter zone */
                bool encounter;
        };

        class JumpPoint
        {
            public:
                /** The system nickname */
                std::wstring sysNick;

                /** The jump point nickname */
                std::wstring jumpNick;

                /** The jump point destination system nickname */
                std::wstring jumpDestSysNick;

                /** The id of the system for this jump point. */
                uint system;

                /** The id of the jump point. */
                uint jumpId;

                /** The jump point destination system id */
                uint jumpDestSysId;
        };

        struct LootableZone
        {
                /** The zone nickname */
                std::wstring zoneNick;

                /** The id of the system for this lootable zone */
                uint systemId;

                /** The nickname and arch id of the loot dropped by the asteroids */
                std::wstring lootNick;
                uint lootId;

                /** The arch id of the crate the loot is dropped in */
                uint crateId;

                /** The minimum number of loot items to drop */
                uint minLoot;

                /** The maximum number of loot items to drop */
                uint maxLoot;

                /** The drop difficultly */
                uint lootDifficulty;

                /** The lootable zone ellipsoid size */
                Vector size;

                /** The lootable zone position */
                Vector pos;
        };

        inline static std::multimap<uint, LootableZone> lootableZones;
        inline static std::multimap<uint, Zone> allZones;
        inline static std::multimap<uint, JumpPoint> jumpPoints;

        // Multiply mat1 by mat2 and return the result
        static TransformMatrix MultiplyMatrix(const TransformMatrix& mat1, const TransformMatrix& mat2);

        static TransformMatrix SetupTransform(const Vector& p, const Vector& r);

        static void ReadLootableZone(const std::string& systemNick, const std::string& defaultZoneNick, const std::string& file);

        static void ReadSystemZones(const std::string& systemNick, const std::string& file);

        /** Read all systems in the universe ini */
        static void ReadUniverse();

        static bool InZone(uint system, const Vector& pos, Zone& rlz);
        static std::optional<const Zone*> InDeathZone(uint system, const Vector& pos, float requiredDamage = 250);

        static void Init();
};
