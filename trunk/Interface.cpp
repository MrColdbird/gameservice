// ======================================================================================
// File         : Interface.cpp
// Author       : Li Chen 
// Last Change  : 07/29/2010 | 15:45:38 PM | Thursday,July
// Description  : 
// ======================================================================================

#include "stdafx.h"
#include "Interface.h"
#include "InterfaceMgr.h"
#include "Master.h"
#include "Task.h"
#include "Session.h"
#include "Stats.h"
#include "Achievement.h"
#include "SignIn.h"
#include "SysMsgBox.h"
#if defined(_PS3)
#ifdef INGAMEBROWSING
#include "InGameBrowsing.h"
#else
#include "StoreBrowsing.h"
#endif
#endif

// ======================================================== 
// GameService Interface for Game
// ======================================================== 
namespace GameService
{

GSCB_Func GS_CallBackFunc[EGSTaskType_MAX];

// Update function
void GS_Initialize(bool requireOnline, int achieveCount, int versionId, bool bTrial, int bDumpLog
#if defined(_PS3)
                   , int iFreeSpaceAvail
#endif
                   )
{
	Master::G()->Initialize(requireOnline==true?1:0, 1, achieveCount, versionId, bTrial==true?1:0, bDumpLog
#if defined(_PS3)
                            , iFreeSpaceAvail
#endif
                            ); 
}
void GS_Update()
{
	Master::G()->Update();
}
void GS_Destroy()
{
    Master::G()->Destroy();
}

// Profile check function
bool GS_NotifyNoProfileSignedIn()
{
#if defined(_XBOX) || defined(_XENON)
	return SignIn::NotifyNoProfileSignedIn();
#endif
	return false;
}
int GS_GetSignedInUserCount()
{
#if defined(_XBOX) || defined(_XENON)
    return SignIn::GetSignedInUserCount();
#elif defined(_PS3)
	// no sub-signin now
	return 1;
#endif
	return 0;
}
void GS_SetBeforePressStart(int before)
{
    SignIn::SetBeforePressStart(before);
}
int GetBeforePressStart()
{
    return SignIn::GetBeforePressStart();
}
void GS_StartGame(int userIndex)
{
    SignIn::StartGame(userIndex);
}
#if defined(_PS3)
int GS_GetUserAge()
{
    return SignIn::GetUserAge();
}
int GS_GetUserLanguage()
{
    switch(SignIn::GetLanguage().language1)
    {
    case SCE_NP_LANG_JAPANESE:
        return GS_ELang_Japanese;
    default:
    case SCE_NP_LANG_ENGLISH:
        return GS_ELang_English;
    case SCE_NP_LANG_FRENCH:
        return GS_ELang_French;
    case SCE_NP_LANG_SPANISH:
        return GS_ELang_Spanish;
    case SCE_NP_LANG_GERMAN:
        return GS_ELang_German;
    case SCE_NP_LANG_ITALIAN:
        return GS_ELang_Italian;
    case SCE_NP_LANG_DUTCH:
        return GS_ELang_Dutch;
    case SCE_NP_LANG_PORTUGUESE:
        return GS_ELang_Portugese;
    case SCE_NP_LANG_RUSSIAN:
        return GS_ELang_Russian;
    case SCE_NP_LANG_KOREAN:
        return GS_ELang_Korean;
    case SCE_NP_LANG_CHINESE_T:
        return GS_ELang_Chinese_t;
    case SCE_NP_LANG_CHINESE_S:
        return GS_ELang_Chinese_s;
    case SCE_NP_LANG_FINNISH:
        return GS_ELang_Finish;
    case SCE_NP_LANG_SWEDISH:
        return GS_ELang_Swedish;
    case SCE_NP_LANG_DANISH:
        return GS_ELang_Danish;
    case SCE_NP_LANG_NORWEGIAN:
        return GS_ELang_Norwegian;
    case SCE_NP_LANG_POLISH:
        return GS_ELang_Polish;
    }
}
#endif

int GS_GetSystemLanguage()
{
#if defined(_XBOX) || defined(_XENON)
	switch(XGetLanguage())
    {
	default:
    case XC_LANGUAGE_ENGLISH:
        return GS_ELang_English;
    case XC_LANGUAGE_SCHINESE:
        return GS_ELang_Chinese_s;
    case XC_LANGUAGE_TCHINESE:
        return GS_ELang_Chinese_t;
    case XC_LANGUAGE_KOREAN:
        return GS_ELang_Korean;
    case XC_LANGUAGE_JAPANESE:
        return GS_ELang_Japanese;
    case XC_LANGUAGE_PORTUGUESE: 
        return GS_ELang_Portugese;
    case XC_LANGUAGE_FRENCH:
        return GS_ELang_French;
    case XC_LANGUAGE_SPANISH:
        return GS_ELang_Spanish;
    case XC_LANGUAGE_ITALIAN:
        return GS_ELang_Italian;
    case XC_LANGUAGE_POLISH:
        return GS_ELang_Polish;
    case XC_LANGUAGE_RUSSIAN:
        return GS_ELang_Russian;
    case XC_LANGUAGE_GERMAN:
        return GS_ELang_German;
        // no nl lang for xbox
        // no no lang for xbox
        // no hu lang for xbox
        // no tr lang for xbox
        // no cs lang for xbox
        // no sl lang for xbox
        // no sv lang for xbox
        // no el lang for xbox
    }

#elif defined(_PS3)
    int language;

    if( cellSysutilGetSystemParamInt(CELL_SYSUTIL_SYSTEMPARAM_ID_LANG, &language) < 0 ) 
    {
        return GS_ELang_English;
    }

    switch(language)
    {
    case CELL_SYSUTIL_LANG_JAPANESE:
        return GS_ELang_Japanese;
    default:
    case CELL_SYSUTIL_LANG_ENGLISH:
        return GS_ELang_English;
    case CELL_SYSUTIL_LANG_FRENCH:
        return GS_ELang_French;
    case CELL_SYSUTIL_LANG_SPANISH:
        return GS_ELang_Spanish;
    case CELL_SYSUTIL_LANG_GERMAN:
        return GS_ELang_German;
    case CELL_SYSUTIL_LANG_ITALIAN:
        return GS_ELang_Italian;
    case CELL_SYSUTIL_LANG_DUTCH:
        return GS_ELang_Dutch;
    case CELL_SYSUTIL_LANG_PORTUGUESE:
        return GS_ELang_Portugese;
    case CELL_SYSUTIL_LANG_RUSSIAN:
        return GS_ELang_Russian;
    case CELL_SYSUTIL_LANG_KOREAN:
        return GS_ELang_Korean;
    case CELL_SYSUTIL_LANG_CHINESE_T:
        return GS_ELang_Chinese_t;
    case CELL_SYSUTIL_LANG_CHINESE_S:
        return GS_ELang_Chinese_s;
    case CELL_SYSUTIL_LANG_FINNISH:
        return GS_ELang_Finish;
    case CELL_SYSUTIL_LANG_SWEDISH:
        return GS_ELang_Swedish;
    case CELL_SYSUTIL_LANG_DANISH:
        return GS_ELang_Danish;
    case CELL_SYSUTIL_LANG_NORWEGIAN:
        return GS_ELang_Norwegian;
    case CELL_SYSUTIL_LANG_POLISH:
        return GS_ELang_Polish;
    }
#endif
}

int IsCableConnected()
{
#if defined(_XBOX) || defined(_XENON)
    return 1;
#elif defined(_PS3)
    return SignIn::IsCableConnected();
#endif
    return 1;
}

void RequestSignIn()
{
#if defined(_XBOX) || defined(_XENON)
    SignIn::ShowSignInUI();
#elif defined(_PS3)
    SignIn::SignInNP();
#endif
}

// Leaderboard function:
void GS_RetrieveLocalStats(int immediately, int boardId, int columnNum, int* columnIds, GSCB_Func fun_cb)
{
    if (!Master::G()->GetStatsSrv())
        return;

#if defined(_XBOX) || defined(_XENON)
	Master::G()->GetStatsSrv()->GetLBDef().Set(boardId,columnNum,columnIds);
#elif defined(_PS3)
	Master::G()->GetStatsSrv()->GetLBDef().Set(boardId,0);
#endif
	GS_CallBackFunc[EGSTaskType_StatsRetrieveLocal] = fun_cb;
	Master::G()->GetStatsSrv()->RetrieveLocalUserStats(immediately);
}
int GS_ReadLeaderboardFinished()
{
    return !( Master::G()->GetStatsSrv()->IsReadingLeaderboard() );
}
int GS_GetLeaderboardCount()
{
    return Master::G()->GetStatsSrv()->GetRetrievedCount();
}
char* GS_GetLeaderboardName(int index)
{
    return Master::G()->GetStatsSrv()->GetRetrievedName(index);
}
int GS_GetLeaderboardRank(int index)
{
    return Master::G()->GetStatsSrv()->GetRetrievedRank(index);
}
int GS_GetLeaderboardScore(int index)
{
    return Master::G()->GetStatsSrv()->GetRetrievedScore(index);
}
int GetStatsErrorCode()
{
	return Master::G()->GetStatsSrv()->GetErrorCode();
}
#if defined(_XBOX) || defined(_XENON)
ULONGLONG GS_GetKeyValueFromStats(int lbIndex)
{
    if (!Master::G()->GetStatsSrv())
        return 0;

	return Master::G()->GetStatsSrv()->GetLocalStats_Key(0,lbIndex);
}
bool GS_CanWriteStats()
{
    if (!Master::G()->GetStatsSrv())
        return false;

	return (Master::G()->GetStatsSrv()->CanWriteStats() == TRUE) ? true : false;
}
void GS_WriteStats(GSCB_Func fun_cb, int lbNum, XSESSION_VIEW_PROPERTIES* views)
{
    if (!Master::G()->GetStatsSrv())
        return;

	Master::G()->GetStatsSrv()->PreWriteLeaderboard(lbNum, views);
	Master::G()->GetStatsSrv()->WriteLeaderboard(SignIn::GetActiveUserIndex()); // always the first user
	GS_CallBackFunc[EGSTaskType_StatsWrite] = fun_cb;
}
void GS_FlushStats(GSCB_Func fun_cb)
{
    if (!Master::G()->GetStatsSrv())
        return;

	Master::G()->GetStatsSrv()->FlushLeaderboard();
	GS_CallBackFunc[EGSTaskType_StatsFlush] = fun_cb;
}
bool GS_ReadStats(int boardId, int columnNum, int* columnIds, int rankIdx, int userIndex, int maxRowNum, int myScoreOffset, GSCB_Func fun_cb)
{
    if (!Master::G()->GetStatsSrv())
        return FALSE;

	Master::G()->GetStatsSrv()->GetLBDef().Set(boardId,columnNum,columnIds);
	GS_CallBackFunc[EGSTaskType_StatsRead] = fun_cb;
	return Master::G()->GetStatsSrv()->ReadLeaderboard(rankIdx, userIndex, maxRowNum, myScoreOffset) == 1 ? true : false;
}
bool GS_ReadFriendsStats(int boardId, int columnNum, int* columnIds, int rankIdx, int userIndex, int maxRowNum, GSCB_Func fun_cb)
{
    if (!Master::G()->GetStatsSrv())
        return FALSE;

	Master::G()->GetStatsSrv()->GetLBDef().Set(boardId,columnNum,columnIds);
	GS_CallBackFunc[EGSTaskType_StatsRead] = fun_cb;
	userIndex = SignIn::GetActiveUserIndex();
	return Master::G()->GetStatsSrv()->ReadFriendsLeaderboard(rankIdx, userIndex, maxRowNum) == 1 ? true : false;
}
#elif defined(_PS3)
int GS_GetKeyValueFromStats(int lbIndex)
{
    if (!Master::G()->GetStatsSrv())
        return 0;

	return Master::G()->GetStatsSrv()->GetLocalStats_Key(0,lbIndex);
}
void GS_WriteStats(GSCB_Func fun_cb, int iLBId, int score)
{
    if (!Master::G()->GetStatsSrv())
        return;

	Master::G()->GetStatsSrv()->GetLBDef().Set(iLBId,score);
	GS_CallBackFunc[EGSTaskType_StatsRead] = fun_cb;
    Master::G()->GetStatsSrv()->WriteLeaderboard(0);
}
bool GS_ReadStats(int boardId, int score, int rankIdx, int userIndex, int maxRowNum, int myScoreOffset, GSCB_Func fun_cb)
{
    if (!Master::G()->GetStatsSrv())
        return FALSE;

	Master::G()->GetStatsSrv()->GetLBDef().Set(boardId,score);
	GS_CallBackFunc[EGSTaskType_StatsRead] = fun_cb;
	return Master::G()->GetStatsSrv()->ReadLeaderboard(rankIdx, userIndex, maxRowNum, myScoreOffset);
}
bool GS_ReadFriendsStats(int boardId, int score, int rankIdx, int userIndex, int maxRowNum, GSCB_Func fun_cb)
{
    if (!Master::G()->GetStatsSrv())
        return FALSE;

	Master::G()->GetStatsSrv()->GetLBDef().Set(boardId,score);
	GS_CallBackFunc[EGSTaskType_StatsRead] = fun_cb;
	return Master::G()->GetStatsSrv()->ReadFriendsLeaderboard(rankIdx, userIndex, maxRowNum);
}
#endif
void GS_DebugOutputStats()
{
    if (!Master::G()->GetStatsSrv())
        return;

	Master::G()->GetStatsSrv()->DebugOutputLeaderboard();
}
void GS_ShowGamerCard(int index)
{
    if (!Master::G()->GetStatsSrv())
        return;

    SignIn::ShowGamerCardUI(Master::G()->GetStatsSrv()->GetRetrievedIDByIndex(index));
}

// Session Service
void GS_CreateSession(GSCB_Func fun_cb)
{
    if (!Master::G()->GetSessionSrv())
        return;

	GS_CallBackFunc[EGSTaskType_SessionCreate] = fun_cb;
	Master::G()->GetSessionSrv()->BeginSession();
}
void GS_JoinSession(GSCB_Func fun_cb)
{
    if (!Master::G()->GetSessionSrv())
        return;

	GS_CallBackFunc[EGSTaskType_SessionJoin] = fun_cb;
	Master::G()->GetSessionSrv()->JoinSession();
}
bool GS_IsSessionCreated()
{
    if (!Master::G()->GetSessionSrv())
        return false;

	return Master::G()->GetSessionSrv()->IsCreated()==TRUE ? true : false;
}
void GS_StartSession(GSCB_Func fun_cb)
{
    if (!Master::G()->GetSessionSrv())
        return;

	GS_CallBackFunc[EGSTaskType_SessionStart] = fun_cb;
	Master::G()->GetSessionSrv()->StartSession();
}
bool GS_IsSessionStarted()
{
    if (!Master::G()->GetSessionSrv())
        return false;

	return Master::G()->GetSessionSrv()->IsStarted()==TRUE ? true : false;
}
void GS_EndSession(GSCB_Func fun_cb)
{
    if (!Master::G()->GetSessionSrv())
        return;

	GS_CallBackFunc[EGSTaskType_SessionEnd] = fun_cb;
	Master::G()->GetSessionSrv()->EndSession();
}
void GS_LeaveSession(GSCB_Func fun_cb)
{
    if (!Master::G()->GetSessionSrv())
        return;

	GS_CallBackFunc[EGSTaskType_SessionLeave] = fun_cb;
	Master::G()->GetSessionSrv()->LeaveSession();
}
void GS_DeleteSession(GSCB_Func fun_cb)
{
    if (!Master::G()->GetSessionSrv())
        return;

	GS_CallBackFunc[EGSTaskType_SessionDelete] = fun_cb;
	Master::G()->GetSessionSrv()->DeleteSession();
}

// Achievement functions:
void GS_WriteAchievement(int num, int* ids)
{
    if (!Master::G()->GetAchievementSrv())
        return;

	Master::G()->GetAchievementSrv()->Write(num,ids);
}

void GS_ShowAchievementUI()
{
	#if defined(_XBOX) || defined(_XENON)
    if (!Master::G()->GetAchievementSrv())
        return;

    Master::G()->GetAchievementSrv()->ShowSystemUI(SignIn::GetActiveUserIndex());
	#endif
}

void GS_UnlockFullGame()
{
#if defined(_XBOX) || defined(_XENON)
	XShowMarketplaceUI(SignIn::GetActiveUserIndex(),XSHOWMARKETPLACEUI_ENTRYPOINT_CONTENTLIST,0,-1);
    //Master::G()->GetAchievementSrv()->ShowSystemUI(SignIn::GetActiveUserIndex());
#elif defined(_PS3)
#ifdef INGAMEBROWSING
	Master::G()->GetInGameBrowsingSrv()->Start();
#else
	Master::G()->GetStoreBrowsingSrv()->Start();
#endif
#endif
}

bool GS_IsUserSignInLive()
{
#if defined(_XBOX) || defined(_XENON)
	return SignIn::IsUserSignedInOnline(SignIn::GetActiveUserIndex())==1 ? true:false;
#endif
    return true;
}

bool GS_IsAchievementEarned(int index)
{
    if (!Master::G()->GetAchievementSrv())
        return false;

    return Master::G()->GetAchievementSrv()->IsEarned(index);
}

bool HasAchievementRead()
{
    if (!Master::G()->GetAchievementSrv())
        return false;

    return Master::G()->GetAchievementSrv()->HasRead();
}

// tracking functions:
bool GS_TrackingSendTag(char* tag, char* attributes)
{
	return Master::G()->SendTrackingTag(tag, attributes)==1?true:false;
}

void GS_StorageDeviceReset()
{
	#if defined(_XBOX) || defined(_XENON)
	SignIn::StorageDeviceReset();
	#endif
}


int AreUsersSignedIn()
{
	#if defined(_XBOX) || defined(_XENON)
		return SignIn::AreUsersSignedIn();
	#endif
		return 0;
}

#if defined(_XBOX) || defined(_XENON)
void PressStartButton(int userIndex)
{
	SignIn::SetActiveUserIndex(userIndex);
	SignIn::SetBeforePressStart(FALSE);
	SignIn::QuerySigninStatus();
	SignIn::StorageDeviceReset();//clear storage device id.
}
#endif

// ======================================================== 
// Get Language and Locale code
// ======================================================== 
void GetConsoleLangLocaleAbbr(char* output)
{
	char* lang;
	char linker[] = "-";

#if defined(_XBOX) || defined(_XENON)
	char* locale;
	switch(XGetLanguage())
    {
	default:
    case XC_LANGUAGE_ENGLISH:
        lang = "EN";
        break;
    case XC_LANGUAGE_SCHINESE:
    case XC_LANGUAGE_TCHINESE:
        lang = "ZH";
        break;
    case XC_LANGUAGE_KOREAN:
        lang = "KO";
        break;
    case XC_LANGUAGE_JAPANESE:
        lang = "JA";
        break;
    case XC_LANGUAGE_PORTUGUESE: 
        lang = "PT";
        break;
    case XC_LANGUAGE_FRENCH:
        lang = "FR";
        break;
    case XC_LANGUAGE_SPANISH:
        lang = "ES";
        break;
    case XC_LANGUAGE_ITALIAN:
        lang = "IT";
        break;
    case XC_LANGUAGE_POLISH:
        lang = "PL";
        break;
    case XC_LANGUAGE_RUSSIAN:
        lang = "RU";
        break;
    case XC_LANGUAGE_GERMAN:
        lang = "DE";
        break;
        // no nl lang for xbox
        // no no lang for xbox
        // no hu lang for xbox
        // no tr lang for xbox
        // no cs lang for xbox
        // no sl lang for xbox
        // no sv lang for xbox
        // no el lang for xbox
    }

    switch(XGetLocale())
    {
	default:
    case XC_LOCALE_UNITED_STATES:
        locale = "US";
        break;
    case XC_LOCALE_GREAT_BRITAIN:
        locale = "GB";
        break;
    case XC_LOCALE_HONG_KONG:
        locale = "HK";
        break;
    case XC_LOCALE_CANADA:
        locale = "CA";
        break;
    case XC_LOCALE_AUSTRALIA:
        locale = "AU";
        break;
    case XC_LOCALE_IRELAND:
        locale = "IE";
        break;
    case XC_LOCALE_FINLAND:
        locale = "FI";
        break;
    case XC_LOCALE_DENMARK:
        locale = "DK";
        break;
        // no il locale for xbox
    case XC_LOCALE_SOUTH_AFRICA:
        locale = "ZA";
        break;
    case XC_LOCALE_NORWAY:
        locale = "NO";
        break;
    case XC_LOCALE_NEW_ZEALAND:
        locale = "NZ";
        break;
        // no ph locale for xbox
        // no my locale for xbox
    case XC_LOCALE_INDIA:
        locale = "IN";
        break;
    case XC_LOCALE_SINGAPORE:
        locale = "SG";
        break;
        // no id locale for xbox
        // no th locale for xbox
        // no xa locale for xbox
    case XC_LOCALE_CHINA:
        locale = "CN";
        break;
    case XC_LOCALE_TAIWAN:
        locale = "TW";
        break;
    case XC_LOCALE_KOREA:
        locale = "KR";
        break;
    case XC_LOCALE_JAPAN:
        locale = "JP";
        break;
    case XC_LOCALE_PORTUGAL:
        locale = "PT";
        break;
    case XC_LOCALE_BRAZIL:
        locale = "BR";
        break;
    case XC_LOCALE_FRANCE:
        locale = "FR";
        break;
    case XC_LOCALE_SWITZERLAND:
        locale = "CH";
        break;
    case XC_LOCALE_BELGIUM:
        locale = "BE";
        break;
        // no la locale for xbox
    case XC_LOCALE_SPAIN:
        locale = "SE";
        break;
        // no ar locale for xbox
    case XC_LOCALE_MEXICO:
        locale = "MX";
        break;
    case XC_LOCALE_COLOMBIA:
        locale = "CO";
        break;
        // no pr locale for xbox
    case XC_LOCALE_GERMANY:
        locale = "DE";
        break;
    case XC_LOCALE_AUSTRIA:
        locale = "AT";
        break;
        // no ru locale for xbox......
    case XC_LOCALE_ITALY:
        locale = "IT";
        break;
    case XC_LOCALE_GREECE:
        locale = "GR";
        break;
    case XC_LOCALE_HUNGARY:
        locale = "HU";
        break;
        // no tr locale for xbox
    case XC_LOCALE_CZECH_REPUBLIC:
        locale = "CZ";
        break;
    case XC_LOCALE_SLOVAK_REPUBLIC:
        locale = "SL";
        break;
    case XC_LOCALE_POLAND:
        locale = "PL";
        break;
    case XC_LOCALE_SWEDEN:
        locale = "SE";
        break;
    case XC_LOCALE_CHILE:
        locale = "CL";
        break;
    case XC_LOCALE_NETHERLANDS:
        locale = "NL";
        break;
    }
	strcpy_s(output, 10, lang);
    strcat_s(output, 10, linker);
    strcat_s(output, 10, locale);

#elif defined(_PS3)
    if (!SignIn::IsUserOnline())
        return;

    int ps3_lang = -1;
    SceNpCountryCode ps3_country;

    if (sceNpManagerGetAccountRegion(&ps3_country, &ps3_lang) < 0)
    {
        strcpy(ps3_country.data, "fr");
    }

    switch(GS_GetSystemLanguage())
    {
    default:
    case GS_ELang_English:
        lang = "EN";
        break;
    case GS_ELang_Japanese:
        lang = "JA";
        break;
    case GS_ELang_French:
        lang = "FR";
        break;
    case GS_ELang_Spanish:
        lang = "ES";
        break;
    case GS_ELang_German:
        lang = "DE";
        break;
    case GS_ELang_Italian:
        lang = "IT";
        break;
    case GS_ELang_Dutch:
        lang = "NL";
        break;
    case GS_ELang_Portugese:
        lang = "PT";
        break;
    case GS_ELang_Russian:
        lang = "RU";
        break;
    case GS_ELang_Korean:
        lang = "KO";
        break;
    case GS_ELang_Chinese_t:
    case GS_ELang_Chinese_s:
        lang = "ZH";
        break;
    case GS_ELang_Finish:
        lang = "FI";
        break;
    case GS_ELang_Swedish:
        lang = "SV";
        break;
    case GS_ELang_Danish:
        lang = "DA";
        break;
    case GS_ELang_Norwegian:
        lang = "NO";
        break;
    case GS_ELang_Polish:
        lang = "PL";
        break;
    }
    strcpy(output, lang);
    strcat(output, linker);
    strcat(output, ps3_country.data);
#endif
}


// ======================================================== 
// SysMsgBox functions
// ======================================================== 
void ShowSysMsgBox_SaveDataNoFreeSpace( GSCB_Func fun_cb, int needExtraKB )
{
    if (Master::G()->GetSysMsgBoxManager())
	{
        Master::G()->GetSysMsgBoxManager()->Display(EMODE_SaveDataNoSpace, &needExtraKB);
	}

	GS_CallBackFunc[EGSTaskType_SysMsgBox_SaveDataNoFreeSpace] = fun_cb;
}

void ShowSysMsgBox_KeyFileCorrupted()
{
    if (Master::G()->GetSysMsgBoxManager())
	{
        Master::G()->GetSysMsgBoxManager()->Display(EMODE_KeyFileCorrupted);
	}
}
void ShowSysMsgBox_TrophyNoFreeSpace()
{
    if (Master::G()->GetSysMsgBoxManager())
        Master::G()->GetSysMsgBoxManager()->Display(EMODE_TrophyNoSpace);
}

void ShowSysMsgBox_PlayOtherUserSaveData()
{
    if (Master::G()->GetSysMsgBoxManager())
        Master::G()->GetSysMsgBoxManager()->Display(EMODE_PlayOtherUserSaveData);
}

// ======================================================== 
// RichPresence
// ======================================================== 
#if defined(_XBOX) || defined(_XENON)
void SetRichPresenceMode(int userIndex, int mode) 
{
    if (userIndex == -1)
    {
        userIndex = SignIn::GetActiveUserIndex();
    }

    // Update the presence mode
    // TODO: 
    // set interval of invoking XUserSetContext
    XUserSetContext( userIndex , X_CONTEXT_PRESENCE, mode );
}
void UpdateDefaultPresenceInfo(int defaultInfo, int activeInfo)
{
    for (int i=0;i<XUSER_MAX_COUNT;i++)
    {
        if (SignIn::IsUserOnline(i))
        {
            if (SignIn::GetActiveUserIndex() == i)
                SetRichPresenceMode(i, activeInfo);
            else
                SetRichPresenceMode(i, defaultInfo);
        }
    }
}
#endif

// ======================================================== 
// Log outside
// ======================================================== 
GS_VOID Log(const GS_CHAR* strFormat, ...)
{
    va_list pArgList;
    va_start( pArgList, strFormat );

    GS_CHAR str[1024];
#if defined(_XBOX) || defined(_XENON)
    sprintf_s( str, 1024, strFormat, pArgList );
#elif defined(_PS3)
    sprintf( str, strFormat, pArgList );
#endif
    Master::G()->Log(str);

    va_end( pArgList );
 }

// ======================================================== 
// InterfaceMgr implementation
// for internal use only
// ======================================================== 
InterfaceMgr::InterfaceMgr(MessageMgr* msgMgr)
{
	if (msgMgr)
	{
		msgMgr->Register(EMessage_CallBackInterface,this);
	}
}

#define GS_CALLBACK(_index, _ret) { if (GS_CallBackFunc[_index]) {(*(GS_CallBackFunc[_index]))(_ret);} }

void InterfaceMgr::MessageResponse(Message* message)
{
	assert(message->GetMessageID() == EMessage_CallBackInterface);

	int task_type = *(int*)message->ReadPayload(0);
	bool result = *(bool*)message->ReadPayload(1);

	GS_CALLBACK(task_type, result);
}

void ShowSysMsgBox_PlayerAgeForbidden()
{
	if (Master::G()->GetSysMsgBoxManager())
		Master::G()->GetSysMsgBoxManager()->Display(EMODE_PlayerAgeForbidden);
}
} // namespace GameService

