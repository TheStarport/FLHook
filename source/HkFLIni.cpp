#include "hook.h"
#include "flcodec.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HK_ERROR HkFLIniGet(std::variant<uint, std::wstring> player, const std::wstring &wscKey,
                    std::wstring &wscRet) {
    wscRet = L"";
    std::wstring wscDir;
    if (!HKHKSUCCESS(HkGetAccountDirName(player, wscDir)))
        return HKE_CHAR_DOES_NOT_EXIST;

    std::wstring wscFile;
    HkGetCharFileName(player, wscFile);

    std::string scCharFile =
        scAcctPath + wstos(wscDir) + "\\" + wstos(wscFile) + ".fl";
    if (HkIsEncoded(scCharFile)) {
        std::string scCharFileNew = scCharFile + ".ini";
        if (!flc_decode(scCharFile.c_str(), scCharFileNew.c_str()))
            return HKE_COULD_NOT_DECODE_CHARFILE;

        wscRet =
            stows(IniGetS(scCharFileNew, "Player", wstos(wscKey).c_str(), ""));
        DeleteFile(scCharFileNew.c_str());
    } else {
        wscRet =
            stows(IniGetS(scCharFile, "Player", wstos(wscKey).c_str(), ""));
    }

    return HKE_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HK_ERROR HkFLIniWrite(std::variant<uint, std::wstring> player,
                      const std::wstring &wscKey,
                      const std::wstring &wscValue) {
    std::wstring wscDir;
    if (!HKHKSUCCESS(HkGetAccountDirName(player, wscDir)))
        return HKE_CHAR_DOES_NOT_EXIST;

    std::wstring wscFile;
    HkGetCharFileName(player, wscFile);

    std::string scCharFile =
        scAcctPath + wstos(wscDir) + "\\" + wstos(wscFile) + ".fl";
    if (HkIsEncoded(scCharFile)) {
        std::string scCharFileNew = scCharFile + ".ini";
        if (!flc_decode(scCharFile.c_str(), scCharFileNew.c_str()))
            return HKE_COULD_NOT_DECODE_CHARFILE;

        IniWrite(scCharFileNew, "Player", wstos(wscKey).c_str(),
                 wstos(wscValue).c_str());

        // keep decoded
        DeleteFile(scCharFile.c_str());
        MoveFile(scCharFileNew.c_str(), scCharFile.c_str());
        /*		if(!flc_encode(scCharFileNew.c_str(), scCharFile.c_str()))
                                return HKE_COULD_NOT_ENCODE_CHARFILE;

                        DeleteFile(scCharFileNew.c_str()); */
    } else {
        IniWrite(scCharFile, "Player", wstos(wscKey).c_str(),
                 wstos(wscValue).c_str());
    }

    return HKE_OK;
}
