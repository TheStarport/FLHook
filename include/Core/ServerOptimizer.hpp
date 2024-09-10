#pragma once

#include "PCH.hpp"
namespace ServerOptimizer
{
    void CEGun_Update_naked();
    void GameObjectDestructorNaked();
    void CAsteroidInitNaked();
    void CObjDestrOrgNaked();
    void FindInStarListNaked();
    void FindInStarListNaked2();
    CObject* __cdecl CObjectFindDetour(const uint& spaceObjId, CObject::Class objClass);
    CObject* __cdecl CObjAllocDetour(CObject::Class objClass);

} // namespace ServerOptimizer
