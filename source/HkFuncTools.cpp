#include "Hook.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HK_ERROR HkGetClientID(bool &bIdString, uint &iClientID,
                       const std::wstring &wscCharname) {
    bIdString = wscCharname.find(L"id ") == 0;

    HK_ERROR hkErr = HkResolveId(wscCharname, iClientID);
    if (hkErr != HKE_OK) {
        if (hkErr == HKE_INVALID_ID_STRING) {
            hkErr = HkResolveShortCut(wscCharname, iClientID);
            if ((hkErr == HKE_AMBIGUOUS_SHORTCUT) ||
                (hkErr == HKE_NO_MATCHING_PLAYER))
                return hkErr;
            if (hkErr == HKE_INVALID_SHORTCUT_STRING) {
                iClientID = HkGetClientIdFromCharname(wscCharname);
                if (iClientID != (uint)-1)
                    return HKE_OK;
                else
                    return HKE_PLAYER_NOT_LOGGED_IN;
            }
        }
    }
    return hkErr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint HkGetClientIdFromAccount(CAccount *acc) {
    struct PlayerData *pPD = 0;
    while (pPD = Players.traverse_active(pPD)) {
        if (pPD->Account == acc) {
            return pPD->iOnlineID;
        }
    }

    return -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint HkGetClientIdFromPD(struct PlayerData *pPD) { return pPD->iOnlineID; }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

CAccount *HkGetAccountByCharname(const std::wstring &wscCharname) {

    st6::wstring flStr((ushort *)wscCharname.c_str());
    CAccount *acc = Players.FindAccountFromCharacterName(flStr);

    return acc;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint HkGetClientIdFromCharname(const std::wstring &wscCharname) {
    CAccount *acc = HkGetAccountByCharname(wscCharname);
    if (!acc)
        return -1;

    uint iClientID = HkGetClientIdFromAccount(acc);
    if (iClientID == -1)
        return -1;

    wchar_t *wszActiveCharname =
        (wchar_t *)Players.GetActiveCharacterName(iClientID);
    if (!wszActiveCharname)
        return -1;

    std::wstring wscActiveCharname = wszActiveCharname;
    wscActiveCharname = ToLower(wscActiveCharname);
    if (wscActiveCharname.compare(ToLower(wscCharname)) != 0)
        return -1;

    return iClientID;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::wstring HkGetAccountID(CAccount *acc) {
    if (acc && acc->wszAccID)
        return acc->wszAccID;
    return L"";
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool HkIsEncoded(const std::string &scFilename) {
    bool bRet = false;
    FILE *f;
    fopen_s(&f, scFilename.c_str(), "r");
    if (!f)
        return false;

    char szMagic[] = "FLS1";
    char szFile[sizeof(szMagic)] = "";
    fread(szFile, 1, sizeof(szMagic), f);
    if (!strncmp(szMagic, szFile, sizeof(szMagic) - 1))
        bRet = true;
    fclose(f);

    return bRet;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool HkIsInCharSelectMenu(const std::wstring &wscCharname) {
    CAccount *acc = HkGetAccountByCharname(wscCharname);
    if (!acc)
        return false;

    uint iClientID = HkGetClientIdFromAccount(acc);
    if (iClientID == -1)
        return false;

    uint iBase = 0;
    uint iSystem = 0;
    pub::Player::GetBase(iClientID, iBase);
    pub::Player::GetSystem(iClientID, iSystem);
    if (!iBase && !iSystem)
        return true;
    else
        return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool HkIsInCharSelectMenu(uint iClientID) {

    uint iBase = 0;
    uint iSystem = 0;
    pub::Player::GetBase(iClientID, iBase);
    pub::Player::GetSystem(iClientID, iSystem);
    if (!iBase && !iSystem)
        return true;
    else
        return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool HkIsValidClientID(uint iClientID) {

    struct PlayerData *pPD = 0;
    while (pPD = Players.traverse_active(pPD)) {
        if (pPD->iOnlineID == iClientID)
            return true;
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HK_ERROR HkResolveId(const std::wstring &wscCharname, uint &iClientID) {
    std::wstring wscCharnameLower = ToLower(wscCharname);
    if (wscCharnameLower.find(L"id ") == 0) {
        uint iID = 0;
        swscanf_s(wscCharnameLower.c_str(), L"id %u", &iID);
        if (!HkIsValidClientID(iID))
            return HKE_INVALID_CLIENT_ID;
        iClientID = iID;
        return HKE_OK;
    }

    return HKE_INVALID_ID_STRING;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HK_ERROR HkResolveShortCut(const std::wstring &wscShortcut, uint &_iClientID) {
    std::wstring wscShortcutLower = ToLower(wscShortcut);
    if (wscShortcutLower.find(L"sc ") != 0)
        return HKE_INVALID_SHORTCUT_STRING;

    wscShortcutLower = wscShortcutLower.substr(3);

    uint iClientIDFound = -1;
    struct PlayerData *pPD = 0;
    while (pPD = Players.traverse_active(pPD)) {
        uint iClientID = HkGetClientIdFromPD(pPD);

        const wchar_t *wszCharname =
            (wchar_t *)Players.GetActiveCharacterName(iClientID);
        if (!wszCharname)
            continue;

        std::wstring wscCharname = wszCharname;
        if (ToLower(wscCharname).find(wscShortcutLower) != -1) {
            if (iClientIDFound == -1)
                iClientIDFound = iClientID;
            else
                return HKE_AMBIGUOUS_SHORTCUT;
        }
    }

    if (iClientIDFound == -1)
        return HKE_NO_MATCHING_PLAYER;

    _iClientID = iClientIDFound;
    return HKE_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint HkGetClientIDByShip(uint iShip) {
    for (uint i = 0; i <= MAX_CLIENT_ID; i++) {
        if (ClientInfo[i].iShip == iShip || ClientInfo[i].iShipOld == iShip)
            return i;
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HK_ERROR HkGetAccountDirName(CAccount *acc, std::wstring &wscDir) {
    _GetFLName GetFLName = (_GetFLName)((char *)hModServer + 0x66370);

    char szDir[1024] = "";
    GetFLName(szDir, acc->wszAccID);
    wscDir = stows(szDir);
    return HKE_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HK_ERROR HkGetAccountDirName(const std::wstring &wscCharname,
                             std::wstring &wscDir) {
    HK_GET_CLIENTID_OR_LOGGED_OUT(iClientID, wscCharname);
    CAccount *acc;
    if (iClientID != -1)
        acc = Players.FindAccountFromClientID(iClientID);
    else {
        if (!(acc = HkGetAccountByCharname(wscCharname)))
            return HKE_CHAR_DOES_NOT_EXIST;
    }

    return HkGetAccountDirName(acc, wscDir);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HK_ERROR HkGetCharFileName(const std::wstring &wscCharname,
                           std::wstring &wscFilename) {
    static _GetFLName GetFLName = 0;
    if (!GetFLName)
        GetFLName = (_GetFLName)((char *)hModServer + 0x66370);

    char szBuf[1024] = "";

    HK_GET_CLIENTID_OR_LOGGED_OUT(iClientID, wscCharname);
    if (iClientID != -1) {
        GetFLName(szBuf,
                  (const wchar_t *)Players.GetActiveCharacterName(iClientID));
    } else {
        GetFLName(szBuf, wscCharname.c_str());
    }

    wscFilename = stows(szBuf);
    return HKE_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::wstring HkGetBaseNickByID(uint iBaseID) {
    char szBasename[1024] = "";
    pub::GetBaseNickname(szBasename, sizeof(szBasename), iBaseID);
    return stows(szBasename);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::wstring HkGetSystemNickByID(uint iSystemID) {
    char szSystemname[1024] = "";
    pub::GetSystemNickname(szSystemname, sizeof(szSystemname), iSystemID);
    return stows(szSystemname);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::wstring HkGetPlayerSystem(uint iClientID) {
    uint iSystemID;
    pub::Player::GetSystem(iClientID, iSystemID);
    char szSystemname[1024] = "";
    pub::GetSystemNickname(szSystemname, sizeof(szSystemname), iSystemID);
    return stows(szSystemname);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HkLockAccountAccess(CAccount *acc, bool bKick) {
    char szJMP[] = {'\xEB'};
    char szJBE[] = {'\x76'};

    st6::wstring flStr((ushort *)HkGetAccountID(acc).c_str());

    if (!bKick)
        WriteProcMem((void *)0x06D52A6A, &szJMP, 1);

    Players.LockAccountAccess(flStr); // also kicks player on this account
    if (!bKick)
        WriteProcMem((void *)0x06D52A6A, &szJBE, 1);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HkUnlockAccountAccess(CAccount *acc) {
    st6::wstring flStr((ushort *)HkGetAccountID(acc).c_str());
    Players.UnlockAccountAccess(flStr);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HkGetItemsForSale(uint iBaseID, std::list<uint> &lstItems) {
    lstItems.clear();
    char szNOP[] = {'\x90', '\x90'};
    char szJNZ[] = {'\x75', '\x1D'};
    WriteProcMem(SRV_ADDR(ADDR_SRV_GETCOMMODITIES), &szNOP,
                 2); // patch, else we only get commodities
    uint iArray[1024];
    int iSize = sizeof(iArray) / sizeof(uint);
    pub::Market::GetCommoditiesForSale(iBaseID, (uint *const) & iArray, &iSize);
    WriteProcMem(SRV_ADDR(ADDR_SRV_GETCOMMODITIES), &szJNZ, 2);

    for (int i = 0; (i < iSize); i++)
        lstItems.push_back(iArray[i]);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

IObjInspectImpl *HkGetInspect(uint iClientID) {
    uint iShip;
    pub::Player::GetShip(iClientID, iShip);
    uint iDunno;
    IObjInspectImpl *inspect;
    if (!GetShipInspect(iShip, inspect, iDunno))
        return 0;
    else
        return inspect;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

ENGINE_STATE HkGetEngineState(uint iClientID) {
    if (ClientInfo[iClientID].bTradelane)
        return ES_TRADELANE;
    else if (ClientInfo[iClientID].bCruiseActivated)
        return ES_CRUISE;
    else if (ClientInfo[iClientID].bThrusterActivated)
        return ES_THRUSTER;
    else if (!ClientInfo[iClientID].bEngineKilled)
        return ES_ENGINE;
    else
        return ES_KILLED;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

EQ_TYPE HkGetEqType(Archetype::Equipment *eq) {
    uint iVFTableMine = (uint)hModCommon + ADDR_COMMON_VFTABLE_MINE;
    uint iVFTableCM = (uint)hModCommon + ADDR_COMMON_VFTABLE_CM;
    uint iVFTableGun = (uint)hModCommon + ADDR_COMMON_VFTABLE_GUN;
    uint iVFTableShieldGen = (uint)hModCommon + ADDR_COMMON_VFTABLE_SHIELDGEN;
    uint iVFTableThruster = (uint)hModCommon + ADDR_COMMON_VFTABLE_THRUSTER;
    uint iVFTableShieldBat = (uint)hModCommon + ADDR_COMMON_VFTABLE_SHIELDBAT;
    uint iVFTableNanoBot = (uint)hModCommon + ADDR_COMMON_VFTABLE_NANOBOT;
    uint iVFTableMunition = (uint)hModCommon + ADDR_COMMON_VFTABLE_MUNITION;
    uint iVFTableEngine = (uint)hModCommon + ADDR_COMMON_VFTABLE_ENGINE;
    uint iVFTableScanner = (uint)hModCommon + ADDR_COMMON_VFTABLE_SCANNER;
    uint iVFTableTractor = (uint)hModCommon + ADDR_COMMON_VFTABLE_TRACTOR;
    uint iVFTableLight = (uint)hModCommon + ADDR_COMMON_VFTABLE_LIGHT;

    uint iVFTable = *((uint *)eq);
    if (iVFTable == iVFTableGun) {
        Archetype::Gun *gun = (Archetype::Gun *)eq;
        Archetype::Equipment *eqAmmo =
            Archetype::GetEquipment(gun->iProjectileArchID);
        int iMissile;
        memcpy(&iMissile, (char *)eqAmmo + 0x90, 4);
        uint iGunType = gun->get_hp_type_by_index(0);
        if (iGunType == 36)
            return ET_TORPEDO;
        else if (iGunType == 35)
            return ET_CD;
        else if (iMissile)
            return ET_MISSILE;
        else
            return ET_GUN;
    } else if (iVFTable == iVFTableCM)
        return ET_CM;
    else if (iVFTable == iVFTableShieldGen)
        return ET_SHIELDGEN;
    else if (iVFTable == iVFTableThruster)
        return ET_THRUSTER;
    else if (iVFTable == iVFTableShieldBat)
        return ET_SHIELDBAT;
    else if (iVFTable == iVFTableNanoBot)
        return ET_NANOBOT;
    else if (iVFTable == iVFTableMunition)
        return ET_MUNITION;
    else if (iVFTable == iVFTableMine)
        return ET_MINE;
    else if (iVFTable == iVFTableEngine)
        return ET_ENGINE;
    else if (iVFTable == iVFTableLight)
        return ET_LIGHT;
    else if (iVFTable == iVFTableScanner)
        return ET_SCANNER;
    else if (iVFTable == iVFTableTractor)
        return ET_TRACTOR;
    else
        return ET_OTHER;
}

/** Calculate the distance between the two vectors */
float HkDistance3D(Vector v1, Vector v2) {
    float sq1 = v1.x - v2.x, sq2 = v1.y - v2.y, sq3 = v1.z - v2.z;
    return sqrt(sq1 * sq1 + sq2 * sq2 + sq3 * sq3);
}

/** Calculate the distance between the two vectors */
float HkDistance3DByShip(uint iShip1, uint iShip2) {
    Vector v1;
    Matrix m1;
    pub::SpaceObj::GetLocation(iShip1, v1, m1);
    Vector v2;
    Matrix m2;
    pub::SpaceObj::GetLocation(iShip2, v2, m2);

    return HkDistance3D(v1, v2);
}

Quaternion HkMatrixToQuaternion(Matrix m) {
    Quaternion quaternion;
    quaternion.w =
        sqrt(max(0, 1 + m.data[0][0] + m.data[1][1] + m.data[2][2])) / 2;
    quaternion.x =
        sqrt(max(0, 1 + m.data[0][0] - m.data[1][1] - m.data[2][2])) / 2;
    quaternion.y =
        sqrt(max(0, 1 - m.data[0][0] + m.data[1][1] - m.data[2][2])) / 2;
    quaternion.z =
        sqrt(max(0, 1 - m.data[0][0] - m.data[1][1] + m.data[2][2])) / 2;
    quaternion.x = (float)_copysign(quaternion.x, m.data[2][1] - m.data[1][2]);
    quaternion.y = (float)_copysign(quaternion.y, m.data[0][2] - m.data[2][0]);
    quaternion.z = (float)_copysign(quaternion.z, m.data[1][0] - m.data[0][1]);
    return quaternion;
}

template <typename Str> Str VectorToSectorCoord(uint iSystemID, Vector vPos) {
    float scale = 1.0;
    const Universe::ISystem *iSystem = Universe::get_system(iSystemID);
    if (iSystem)
        scale = iSystem->NavMapScale;

    float fGridSize = 34000.0f / scale;
    int gridRefX = (int)((vPos.x + (fGridSize * 5)) / fGridSize) - 1;
    int gridRefZ = (int)((vPos.z + (fGridSize * 5)) / fGridSize) - 1;

    gridRefX = min(max(gridRefX, 0), 7);
    char scXPos = 'A' + char(gridRefX);

    gridRefZ = min(max(gridRefZ, 0), 7);
    char scZPos = '1' + char(gridRefZ);

    typename Str::value_type szCurrentLocation[100];
    if constexpr (std::is_same_v<Str, std::string>)
        _snprintf_s(szCurrentLocation, sizeof(szCurrentLocation), "%c-%c",
                    scXPos, scZPos);
    else
        _snwprintf_s(szCurrentLocation, sizeof(szCurrentLocation), L"%C-%C",
                     scXPos, scZPos);

    return szCurrentLocation;
}

#define PI 3.14159265f

// Convert radians to degrees.
float degrees(float rad) {
    rad *= 180 / PI;

    // Prevent displaying -0 and prefer 180 to -180.
    if (rad < 0) {
        if (rad > -0.005f)
            rad = 0;
        else if (rad <= -179.995f)
            rad = 180;
    }

    // Round to two decimal places here, so %g can display it without decimals.
    float frac = modff(rad * 100, &rad);
    if (frac >= 0.5f)
        ++rad;
    else if (frac <= -0.5f)
        --rad;

    return rad / 100;
}

// Convert an orientation matrix to a pitch/yaw/roll vector.  Based on what
// Freelancer does for the save game.
Vector MatrixToEuler(const Matrix &mat) {
    Vector x = {mat.data[0][0], mat.data[1][0], mat.data[2][0]};
    Vector y = {mat.data[0][1], mat.data[1][1], mat.data[2][1]};
    Vector z = {mat.data[0][2], mat.data[1][2], mat.data[2][2]};

    Vector vec;
    float h = (float)_hypot(x.x, x.y);
    if (h > 1 / 524288.0f) {
        vec.x = degrees(atan2f(y.z, z.z));
        vec.y = degrees(atan2f(-x.z, h));
        vec.z = degrees(atan2f(x.y, x.x));
    } else {
        vec.x = degrees(atan2f(-z.y, y.y));
        vec.y = degrees(atan2f(-x.z, h));
        vec.z = 0;
    }
    return vec;
}

void Rotate180(Matrix &rot) {
    rot.data[0][0] = -rot.data[0][0];
    rot.data[1][0] = -rot.data[1][0];
    rot.data[2][0] = -rot.data[2][0];
    rot.data[0][2] = -rot.data[0][2];
    rot.data[1][2] = -rot.data[1][2];
    rot.data[2][2] = -rot.data[2][2];
}

void TranslateY(Vector &pos, Matrix &rot, float y) {
    pos.x += y * rot.data[0][0];
    pos.y += y * rot.data[1][0];
    pos.z += y * rot.data[2][0];
}

void TranslateX(Vector &pos, Matrix &rot, float x) {
    pos.x += x * rot.data[0][2];
    pos.y += x * rot.data[1][2];
    pos.z += x * rot.data[2][2];
}

void TranslateZ(Vector &pos, Matrix &rot, float z) {
    pos.x += z * rot.data[0][1];
    pos.y += z * rot.data[1][1];
    pos.z += z * rot.data[2][1];
}
