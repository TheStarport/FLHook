#include "Core/ServerOptimizer.hpp"

namespace ServerOptimizer
{
    // TODO: implement
    /*
    bool __stdcall CEGun_Update(const CEGun* gun)
    {
        if (gun->owner->ownerPlayer)
        {
            return false;
        }
        return true;
    }

    __declspec(naked) void CEGun_Update_naked()
    {
        __asm
        {
            push ecx
            push ecx
            call CEGun_Update
            pop ecx
            test al, al
            jz skipLabel
            jmp fpOldUpdateCEGun
            skipLabel:
            ret 0x8
        }
    }
    */

    struct StarSystemMock
    {
            uint systemId;
            StarSystem starSystem;
    };

    struct iobjCache
    {
            StarSystem* cacheStarSystem;
            CObject::Class objClass;
    };

    std::unordered_set<uint> playerShips;
    std::unordered_map<uint, iobjCache> cacheSolarIObjs;
    std::unordered_map<uint, iobjCache> cacheNonsolarIObjs;

    auto FindStarListRet = reinterpret_cast<FARPROC>(0x6D0C846);

    PBYTE fpOldStarSystemFind;

    typedef MetaListNode*(__thiscall* FindIObjOnList)(MetaList&, uint searchedId);
    auto FindIObjOnListFunc = reinterpret_cast<FindIObjOnList>(0x6CF4F00);

    typedef IObjRW*(__thiscall* FindIObjInSystem)(StarSystemMock& starSystem, uint searchedId);
    auto FindIObjFunc = reinterpret_cast<FindIObjInSystem>(0x6D0C840);

    GameObject* FindNonSolar(StarSystemMock* starSystem, const uint searchedId)
    {
        const MetaListNode* node = FindIObjOnListFunc(starSystem->starSystem.shipList, searchedId);
        if (node)
        {
            cacheNonsolarIObjs[searchedId] = { node->value->starSystem, node->value->cobj->objectClass };
            return node->value;
        }
        node = FindIObjOnListFunc(starSystem->starSystem.lootList, searchedId);
        if (node)
        {
            cacheNonsolarIObjs[searchedId] = { node->value->starSystem, node->value->cobj->objectClass };
            return node->value;
        }
        node = FindIObjOnListFunc(starSystem->starSystem.guidedList, searchedId);
        if (node)
        {
            cacheNonsolarIObjs[searchedId] = { node->value->starSystem, node->value->cobj->objectClass };
            return node->value;
        }
        node = FindIObjOnListFunc(starSystem->starSystem.mineList, searchedId);
        if (node)
        {
            cacheNonsolarIObjs[searchedId] = { node->value->starSystem, node->value->cobj->objectClass };
            return node->value;
        }
        node = FindIObjOnListFunc(starSystem->starSystem.counterMeasureList, searchedId);
        if (node)
        {
            cacheNonsolarIObjs[searchedId] = { node->value->starSystem, node->value->cobj->objectClass };
            return node->value;
        }
        return nullptr;
    }

    GameObject* FindSolar(StarSystemMock* starSystem, const uint searchedId)
    {
        const MetaListNode* node = FindIObjOnListFunc(starSystem->starSystem.solarList, searchedId);
        if (node)
        {
            cacheSolarIObjs[searchedId] = { node->value->starSystem, node->value->cobj->objectClass };
            return node->value;
        }
        node = FindIObjOnListFunc(starSystem->starSystem.asteroidList, searchedId);
        if (node)
        {
            cacheSolarIObjs[searchedId] = { node->value->starSystem, node->value->cobj->objectClass };
            return node->value;
        }
        return nullptr;
    }

    GameObject* __stdcall FindInStarList(StarSystemMock* starSystem, const uint searchedId)
    {
        static StarSystem* lastFoundInSystem = nullptr;
        static uint lastFoundItem = 0;
        GameObject* retVal = nullptr;

        if (searchedId == 0)
        {
            return nullptr;
        }

        if (lastFoundItem == searchedId && lastFoundInSystem != &starSystem->starSystem)
        {
            return nullptr;
        }

        if (searchedId & 0x80000000) // check if solar
        {
            const auto iter = cacheSolarIObjs.find(searchedId);
            if (iter == cacheSolarIObjs.end())
            {
                return FindSolar(starSystem, searchedId);
            }

            if (iter->second.cacheStarSystem != &starSystem->starSystem)
            {
                lastFoundItem = searchedId;
                lastFoundInSystem = iter->second.cacheStarSystem;
                return nullptr;
            }

            MetaListNode* node;
            switch (iter->second.objClass)
            {
                case CObject::Class::CSOLAR_OBJECT:
                    node = FindIObjOnListFunc(starSystem->starSystem.solarList, searchedId);
                    if (node)
                    {
                        retVal = node->value;
                    }
                    break;
                case CObject::Class::CASTEROID_OBJECT:
                    node = FindIObjOnListFunc(starSystem->starSystem.asteroidList, searchedId);
                    if (node)
                    {
                        retVal = node->value;
                    }
                    break;
                default:;
            }

            cacheSolarIObjs.erase(iter);
        }
        else
        {
            if (!playerShips.contains(searchedId)) // player can swap systems, for them search just the system's shiplist
            {
                const auto iter = cacheNonsolarIObjs.find(searchedId);
                if (iter == cacheNonsolarIObjs.end())
                {
                    return FindNonSolar(starSystem, searchedId);
                }

                if (iter->second.cacheStarSystem != &starSystem->starSystem)
                {
                    lastFoundItem = searchedId;
                    lastFoundInSystem = iter->second.cacheStarSystem;
                    return nullptr;
                }

                MetaListNode* node;
                switch (iter->second.objClass)
                {
                    case CObject::Class::CSHIP_OBJECT:
                        node = FindIObjOnListFunc(starSystem->starSystem.shipList, searchedId);
                        if (node)
                        {
                            retVal = node->value;
                        }
                        break;
                    case CObject::Class::CLOOT_OBJECT:
                        node = FindIObjOnListFunc(starSystem->starSystem.lootList, searchedId);
                        if (node)
                        {
                            retVal = node->value;
                        }
                        break;
                    case CObject::Class::CGUIDED_OBJECT:
                        node = FindIObjOnListFunc(starSystem->starSystem.guidedList, searchedId);
                        if (node)
                        {
                            retVal = node->value;
                        }
                        break;
                    case CObject::Class::CMINE_OBJECT:
                        node = FindIObjOnListFunc(starSystem->starSystem.mineList, searchedId);
                        if (node)
                        {
                            retVal = node->value;
                        }
                        break;
                    case CObject::Class::CCOUNTERMEASURE_OBJECT:
                        node = FindIObjOnListFunc(starSystem->starSystem.counterMeasureList, searchedId);
                        if (node)
                        {
                            retVal = node->value;
                        }
                        break;
                    default:;
                }

                cacheNonsolarIObjs.erase(iter);
            }
            else
            {
                if (const MetaListNode* node = FindIObjOnListFunc(starSystem->starSystem.shipList, searchedId))
                {
                    retVal = node->value;
                }
            }
        }

        return retVal;
    }

    __declspec(naked) void FindInStarListNaked()
    {
        __asm
        {
            push ecx
            push[esp + 0x8]
            sub ecx, 4
            push ecx
            call FindInStarList
            pop ecx
            ret 0x4
        }
    }

    __declspec(naked) void FindInStarListNaked2()
    {
        __asm
        {
            mov eax, [esp+0x4]
            mov edx, [eax]
             mov [esp+0x4], edx
            push ecx
            push[esp + 0x8]
            sub ecx, 4
            push ecx
            call FindInStarList
            pop ecx
            ret 0x4
        }
    }

    void __stdcall GameObjectDestructor(const uint id)
    {
        if (id & 0x8000000)
        {
            cacheSolarIObjs.erase(id);
        }
        else
        {
            cacheNonsolarIObjs.erase(id);
        }
    }

    uint GameObjectDestructorRet = 0x6CEE4A7;
    __declspec(naked) void GameObjectDestructorNaked()
    {
        __asm {
            push ecx
            mov ecx, [ecx+0x4]
            mov ecx, [ecx+0xB0]
            push ecx
            call GameObjectDestructor
            pop ecx
            push 0xFFFFFFFF
            push 0x6d60776
            jmp GameObjectDestructorRet
        }
    }

    struct CObjNode
    {
            CObjNode* next;
            CObjNode* prev;
            CObject* cobj;
    };

    struct CObjEntryNode
    {
            CObjNode* first;
            CObjNode* last;
    };

    struct CObjList
    {
            uint dunno;
            CObjEntryNode* entry;
            uint size;
    };

    std::unordered_map<CObject*, CObjNode*> CMineMap;
    std::unordered_map<CObject*, CObjNode*> CCmMap;
    std::unordered_map<CObject*, CObjNode*> CBeamMap;
    std::unordered_map<CObject*, CObjNode*> CGuidedMap;
    std::unordered_map<CObject*, CObjNode*> CSolarMap;
    std::unordered_map<CObject*, CObjNode*> CShipMap;
    std::unordered_map<CObject*, CObjNode*> CAsteroidMap;
    std::unordered_map<CObject*, CObjNode*> CLootMap;
    std::unordered_map<CObject*, CObjNode*> CEquipmentMap;
    std::unordered_map<CObject*, CObjNode*> CObjectMap;

    std::unordered_map<uint, CAsteroid*> CAsteroidMap2;

    typedef CObjList*(__cdecl* CObjListFunc)(CObject::Class);
    auto CObjListFind = reinterpret_cast<CObjListFunc>(0x62AE690);

    typedef void(__thiscall* RemoveCobjFromVector)(CObjList*, void*, CObjNode*);
    auto removeCObjNode = reinterpret_cast<RemoveCobjFromVector>(0x62AF830);

    uint CObjAllocJmp = 0x62AEE55;
    __declspec(naked) CObject* __cdecl CObjAllocCallOrig(CObject::Class objClass)
    {
        __asm {
            push ecx
            mov eax, [esp + 8]
            jmp CObjAllocJmp
        }
    }

    CObject* __cdecl CObjAllocDetour(const CObject::Class objClass)
    {
        CObject* retVal = CObjAllocCallOrig(objClass);
        const CObjList* cobjList = CObjListFind(objClass);

        switch (objClass)
        {
            case CObject::CASTEROID_OBJECT: CAsteroidMap[retVal] = cobjList->entry->last; break;
            case CObject::CEQUIPMENT_OBJECT: CEquipmentMap[retVal] = cobjList->entry->last; break;
            case CObject::COBJECT_MASK: CObjectMap[retVal] = cobjList->entry->last; break;
            case CObject::CSOLAR_OBJECT: CSolarMap[retVal] = cobjList->entry->last; break;
            case CObject::CSHIP_OBJECT: CShipMap[retVal] = cobjList->entry->last; break;
            case CObject::CLOOT_OBJECT: CLootMap[retVal] = cobjList->entry->last; break;
            case CObject::CBEAM_OBJECT: CBeamMap[retVal] = cobjList->entry->last; break;
            case CObject::CGUIDED_OBJECT: CGuidedMap[retVal] = cobjList->entry->last; break;
            case CObject::CCOUNTERMEASURE_OBJECT: CCmMap[retVal] = cobjList->entry->last; break;
            case CObject::CMINE_OBJECT: CMineMap[retVal] = cobjList->entry->last; break;
            default: return nullptr;
        }

        return retVal;
    }

    void __fastcall CAsteroidInit(CAsteroid* casteroid, void* edx, const CSimple::CreateParms& param) { CAsteroidMap2[param.id] = casteroid; }

    uint CAsteroidInitRetAddr = 0x62A28F6;
    __declspec(naked) void CAsteroidInitNaked()
    {
        __asm {
            push ecx
            push [esp+0x8]
            call CAsteroidInit
            pop ecx
            push esi
            push edi
            mov edi, [esp+0xC]
            jmp CAsteroidInitRetAddr
        }
    }

    void __fastcall CObjDestr(CObject* cobj)
    {
        std::unordered_map<CObject*, CObjNode*>* cobjMap;
        switch (cobj->objectClass)
        {
            case CObject::CASTEROID_OBJECT:
                CAsteroidMap2.erase(reinterpret_cast<CSimple*>(cobj)->id);
                cobjMap = &CAsteroidMap;
                break;
            case CObject::CEQUIPMENT_OBJECT: cobjMap = &CEquipmentMap; break;
            case CObject::COBJECT_MASK: cobjMap = &CObjectMap; break;
            case CObject::CSOLAR_OBJECT: cobjMap = &CSolarMap; break;
            case CObject::CSHIP_OBJECT: cobjMap = &CShipMap; break;
            case CObject::CLOOT_OBJECT: cobjMap = &CLootMap; break;
            case CObject::CBEAM_OBJECT: cobjMap = &CBeamMap; break;
            case CObject::CGUIDED_OBJECT: cobjMap = &CGuidedMap; break;
            case CObject::CCOUNTERMEASURE_OBJECT: cobjMap = &CCmMap; break;
            case CObject::CMINE_OBJECT: cobjMap = &CMineMap; break;
            default: return; // will never be hit, but shuts up the InteliSense
        }

        if (const auto item = cobjMap->find(cobj); item != cobjMap->end())
        {
            CObjList* cobjList = CObjListFind(cobj->objectClass);
            static uint dummy;
            removeCObjNode(cobjList, &dummy, item->second);
            cobjMap->erase(item);
        }
    }

    uint CObjDestrRetAddr = 0x62AF447;
    __declspec(naked) void CObjDestrOrgNaked()
    {
        __asm {
            push ecx
            call CObjDestr
            pop ecx
            push 0xFFFFFFFF
            push 0x06394364
            jmp CObjDestrRetAddr
        }
    }

    CObject* __cdecl CObjectFindDetour(const uint& spaceObjId, const CObject::Class objClass)
    {
        if (objClass != CObject::CASTEROID_OBJECT)
        {
            return CObject::Find(spaceObjId, objClass);
        }

        if (const auto result = CAsteroidMap2.find(spaceObjId); result != CAsteroidMap2.end())
        {
            ++result->second->referenceCounter;
            return result->second;
        }
        return nullptr;
    }
} // namespace ServerOptimizer
