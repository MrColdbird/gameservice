// ======================================================================================
// File         : Interface.cpp
// Author       : Li Chen 
// Last Change  : 11/17/2010 | 11:46:27 AM | Wednesday,November
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
GS_VOID Initialize(GS_BOOL requireOnline, GS_INT achieveCount, GS_INT versionId, GS_BOOL bTrial, GS_INT bDumpLog
#if defined(_PS3)
                   , GS_INT iFreeSpaceAvail
#endif
                   )
{
	Master::G()->Initialize(requireOnline, 1, achieveCount, versionId, bTrial, bDumpLog
#if defined(_PS3)
                            , iFreeSpaceAvail
#endif
                            ); 
}
GS_VOID Update()
{
	Master::G()->Update();
}
GS_VOID Destroy()
{
    Master::G()->Destroy();
}

// Profile check function
GS_BOOL NotifyNoProfileSignedIn()
{
#if defined(_XBOX) || defined(_XENON)
	return SignIn::NotifyNoProfileSignedIn();
#endif
	return FALSE;
}
GS_INT GetSignedInUserCount()
{
#if defined(_XBOX) || defined(_XENON)
    return SignIn::GetSignedInUserCount();
#elif defined(_PS3)
	// no sub-signin now
	return 1;
#endif
	return 0;
}
GS_VOID SetBeforePressStart(GS_INT before)
{
    SignIn::SetBeforePressStart(before);
}
GS_INT GetBeforePressStart()
{
    return SignIn::GetBeforePressStart();
}
GS_VOID StartGame(GS_INT userIndex)
{
    SignIn::StartGame(userIndex);
}
#if defined(_PS3)
GS_INT GetUserAge()
{
    return SignIn::GetUserAge();
}
GS_INT GetUserLanguage()
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

GS_INT GetSystemLanguage()
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
    GS_INT language;

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

GS_INT IsCableConnected()
{
#if defined(_XBOX) || defined(_XENON)
    return 1;
#elif defined(_PS3)
    return SignIn::IsCableConnected();
#endif
    return 1;
}

GS_VOID RequestSignIn()
{
#if defined(_XBOX) || defined(_XENON)
    SignIn::ShowSignInUI();
#elif defined(_PS3)
    SignIn::SignInNP();
#endif
}

// Leaderboard function:
GS_VOID RetrieveLocalStats(GS_INT immediately, GS_INT boardId, GS_INT columnNum, GS_INT* columnIds, GSCB_Func fun_cb)
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
GS_INT ReadLeaderboardFinished()
{
    return !( Master::G()->GetStatsSrv()->IsReadingLeaderboard() );
}
GS_INT GetLeaderboardCount()
{
    return Master::G()->GetStatsSrv()->GetRetrievedCount();
}
char* GetLeaderboardName(GS_INT index)
{
    return Master::G()->GetStatsSrv()->GetRetrievedName(index);
}
GS_INT GetLeaderboardRank(GS_INT index)
{
    return Master::G()->GetStatsSrv()->GetRetrievedRank(index);
}
GS_INT GetLeaderboardScore(GS_INT index)
{
    return Master::G()->GetStatsSrv()->GetRetrievedScore(index);
}
GS_INT GetStatsErrorCode()
{
	return Master::G()->GetStatsSrv()->GetErrorCode();
}
#if defined(_XBOX) || defined(_XENON)
ULONGLONG GetKeyValueFromStats(GS_INT lbIndex)
{
    if (!Master::G()->GetStatsSrv())
        return 0;

	return Master::G()->GetStatsSrv()->GetLocalStats_Key(0,lbIndex);
}
GS_BOOL CanWriteStats()
{
    if (!Master::G()->GetStatsSrv())
        return FALSE;

	return Master::G()->GetStatsSrv()->CanWriteStats();
}
GS_VOID WriteStats(GSCB_Func fun_cb, GS_INT lbNum, XSESSION_VIEW_PROPERTIES* views)
{
    if (!Master::G()->GetStatsSrv())
        return;

	Master::G()->GetStatsSrv()->PreWriteLeaderboard(lbNum, views);
	Master::G()->GetStatsSrv()->WriteLeaderboard(SignIn::GetActiveUserIndex()); // always the first user
	GS_CallBackFunc[EGSTaskType_StatsWrite] = fun_cb;
}
GS_VOID FlushStats(GSCB_Func fun_cb)
{
    if (!Master::G()->GetStatsSrv())
        return;

	Master::G()->GetStatsSrv()->FlushLeaderboard();
	GS_CallBackFunc[EGSTaskType_StatsFlush] = fun_cb;
}
GS_BOOL ReadStats(GS_INT boardId, GS_INT columnNum, GS_INT* columnIds, GS_INT rankIdx, GS_INT userIndex, GS_INT maxRowNum, GS_INT myScoreOffset, GSCB_Func fun_cb)
{
    if (!Master::G()->GetStatsSrv())
        return FALSE;

	Master::G()->GetStatsSrv()->GetLBDef().Set(boardId,columnNum,columnIds);
	GS_CallBackFunc[EGSTaskType_StatsRead] = fun_cb;
	return Master::G()->GetStatsSrv()->ReadLeaderboard(rankIdx, userIndex, maxRowNum, myScoreOffset);
}
GS_BOOL ReadFriendsStats(GS_INT boardId, GS_INT columnNum, GS_INT* columnIds, GS_INT rankIdx, GS_INT userIndex, GS_INT maxRowNum, GSCB_Func fun_cb)
{
    if (!Master::G()->GetStatsSrv())
        return FALSE;

	Master::G()->GetStatsSrv()->GetLBDef().Set(boardId,columnNum,columnIds);
	GS_CallBackFunc[EGSTaskType_StatsRead] = fun_cb;
	userIndex = SignIn::GetActiveUserIndex();
	return Master::G()->GetStatsSrv()->ReadFriendsLeaderboard(rankIdx, userIndex, maxRowNum);
}
#elif defined(_PS3)
GS_INT GetKeyValueFromStats(GS_INT lbIndex)
{
    if (!Master::G()->GetStatsSrv())
        return 0;

	return Master::G()->GetStatsSrv()->GetLocalStats_Key(0,lbIndex);
}
GS_VOID WriteStats(GSCB_Func fun_cb, GS_INT iLBId, GS_INT score)
{
    if (!Master::G()->GetStatsSrv())
        return;

	Master::G()->GetStatsSrv()->GetLBDef().Set(iLBId,score);
	GS_CallBackFunc[EGSTaskType_StatsRead] = fun_cb;
    Master::G()->GetStatsSrv()->WriteLeaderboard(0);
}
GS_BOOL ReadStats(GS_INT boardId, GS_INT score, GS_INT rankIdx, GS_INT userIndex, GS_INT maxRowNum, GS_INT myScoreOffset, GSCB_Func fun_cb)
{
    if (!Master::G()->GetStatsSrv())
        return FALSE;

	Master::G()->GetStatsSrv()->GetLBDef().Set(boardId,score);
	GS_CallBackFunc[EGSTaskType_StatsRead] = fun_cb;
	return Master::G()->GetStatsSrv()->ReadLeaderboard(rankIdx, userIndex, maxRowNum, myScoreOffset);
}
GS_BOOL ReadFriendsStats(GS_INT boardId, GS_INT score, GS_INT rankIdx, GS_INT userIndex, GS_INT maxRowNum, GSCB_Func fun_cb)
{
    if (!Master::G()->GetStatsSrv())
        return FALSE;

	Master::G()->GetStatsSrv()->GetLBDef().Set(boardId,score);
	GS_CallBackFunc[EGSTaskType_StatsRead] = fun_cb;
	return Master::G()->GetStatsSrv()->ReadFriendsLeaderboard(rankIdx, userIndex, maxRowNum);
}
#endif
GS_VOID DebugOutputStats()
{
    if (!Master::G()->GetStatsSrv())
        return;

	Master::G()->GetStatsSrv()->DebugOutputLeaderboard();
}
GS_VOID ShowGamerCard(GS_INT index)
{
    if (!Master::G()->GetStatsSrv())
        return;

    SignIn::ShowGamerCardUI(Master::G()->GetStatsSrv()->GetRetrievedIDByIndex(index));
}

// Session Service
GS_VOID CreateSession(GSCB_Func fun_cb)
{
    if (!Master::G()->GetSessionSrv())
        return;

	GS_CallBackFunc[EGSTaskType_SessionCreate] = fun_cb;
	Master::G()->GetSessionSrv()->BeginSession();
}
GS_VOID JoinSession(GSCB_Func fun_cb)
{
    if (!Master::G()->GetSessionSrv())
        return;

	GS_CallBackFunc[EGSTaskType_SessionJoin] = fun_cb;
	Master::G()->GetSessionSrv()->JoinSession();
}
GS_BOOL IsSessionCreated()
{
    if (!Master::G()->GetSessionSrv())
        return FALSE;

	return Master::G()->GetSessionSrv()->IsCreated();
}
GS_VOID StartSession(GSCB_Func fun_cb)
{
    if (!Master::G()->GetSessionSrv())
        return;

	GS_CallBackFunc[EGSTaskType_SessionStart] = fun_cb;
	Master::G()->GetSessionSrv()->StartSession();
}
GS_BOOL IsSessionStarted()
{
    if (!Master::G()->GetSessionSrv())
        return false;

	return Master::G()->GetSessionSrv()->IsStarted();
}
GS_VOID EndSession(GSCB_Func fun_cb)
{
    if (!Master::G()->GetSessionSrv())
        return;

	GS_CallBackFunc[EGSTaskType_SessionEnd] = fun_cb;
	Master::G()->GetSessionSrv()->EndSession();
}
GS_VOID LeaveSession(GSCB_Func fun_cb)
{
    if (!Master::G()->GetSessionSrv())
        return;

	GS_CallBackFunc[EGSTaskType_SessionLeave] = fun_cb;
	Master::G()->GetSessionSrv()->LeaveSession();
}
GS_VOID DeleteSession(GSCB_Func fun_cb)
{
    if (!Master::G()->GetSessionSrv())
        return;

	GS_CallBackFunc[EGSTaskType_SessionDelete] = fun_cb;
	Master::G()->GetSessionSrv()->DeleteSession();
}

// Achievement functions:
GS_VOID WriteAchievement(GS_INT num, GS_INT* ids)
{
    if (!Master::G()->GetAchievementSrv())
        return;

	Master::G()->GetAchievementSrv()->Write(num,ids);
}

GS_VOID ShowAchievementUI()
{
	#if defined(_XBOX) || defined(_XENON)
    if (!Master::G()->GetAchievementSrv())
        return;

    Master::G()->GetAchievementSrv()->ShowSystemUI(SignIn::GetActiveUserIndex());
	#endif
}

GS_VOID UnlockFullGame()
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

GS_BOOL IsUserSignInLive()
{
#if defined(_XBOX) || defined(_XENON)
	return SignIn::IsUserSignedInOnline(SignIn::GetActiveUserIndex());
#endif
    return TRUE;
}

GS_BOOL IsAchievementEarned(GS_INT index)
{
    if (!Master::G()->GetAchievementSrv())
        return FALSE;

    return Master::G()->GetAchievementSrv()->IsEarned(index);
}

GS_BOOL HasAchievementRead()
{
    if (!Master::G()->GetAchievementSrv())
        return FALSE;

    return Master::G()->GetAchievementSrv()->HasRead();
}

// tracking functions:
GS_BOOL TrackingSendTag(char* tag, char* attributes)
{
	return Master::G()->SendTrackingTag(tag, attributes);
}

GS_INT AreUsersSignedIn()
{
	#if defined(_XBOX) || defined(_XENON)
		return SignIn::AreUsersSignedIn();
	#endif
		return 0;
}

#if defined(_XBOX) || defined(_XENON)
GS_VOID PressStartButton(GS_INT userIndex)
{
	SignIn::SetActiveUserIndex(userIndex);
	SignIn::SetBeforePressStart(FALSE);
	SignIn::QuerySigninStatus();
	//SignIn::StorageDeviceReset();//clear storage device id.
}
#endif

// ======================================================== 
// Get Language and Locale code
// ======================================================== 
GS_VOID GetConsoleLangLocaleAbbr(char* output)
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

    GS_INT ps3_lang = -1;
    SceNpCountryCode ps3_country;

    if (sceNpManagerGetAccountRegion(&ps3_country, &ps3_lang) < 0)
    {
        strcpy(ps3_country.data, "fr");
    }

    switch(GetSystemLanguage())
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
GS_VOID ShowSysMsgBox_SaveDataNoFreeSpace( GSCB_Func fun_cb, GS_INT needExtraKB )
{
    if (Master::G()->GetSysMsgBoxManager())
	{
        Master::G()->GetSysMsgBoxManager()->Display(EMODE_SaveDataNoSpace, &needExtraKB);
	}

	GS_CallBackFunc[EGSTaskType_SysMsgBox_SaveDataNoFreeSpace] = fun_cb;
}

GS_VOID ShowSysMsgBox_KeyFileCorrupted()
{
    if (Master::G()->GetSysMsgBoxManager())
	{
        Master::G()->GetSysMsgBoxManager()->Display(EMODE_KeyFileCorrupted);
	}
}
GS_VOID ShowSysMsgBox_TrophyNoFreeSpace()
{
    if (Master::G()->GetSysMsgBoxManager())
        Master::G()->GetSysMsgBoxManager()->Display(EMODE_TrophyNoSpace);
}

GS_VOID ShowSysMsgBox_PlayOtherUserSaveData()
{
    if (Master::G()->GetSysMsgBoxManager())
        Master::G()->GetSysMsgBoxManager()->Display(EMODE_PlayOtherUserSaveData);
}

GS_VOID ShowSysMsgBox_PlayerAgeForbidden()
{
	if (Master::G()->GetSysMsgBoxManager())
		Master::G()->GetSysMsgBoxManager()->Display(EMODE_PlayerAgeForbidden);
}

// ======================================================== 
// RichPresence
// ======================================================== 
#if defined(_XBOX) || defined(_XENON)
GS_VOID SetRichPresenceMode(GS_INT userIndex, GS_INT mode) 
{
    if (userIndex == -1)
    {
        userIndex = SignIn::GetActiveUserIndex();
    }

    // Update the presence mode
    // TODO: 
    // set Interval of invoking XUserSetContext
    XUserSetContext( userIndex , X_CONTEXT_PRESENCE, mode );
}
GS_VOID UpdateDefaultPresenceInfo(GS_INT defaultInfo, GS_INT activeInfo)
{
    for (GS_INT i=0;i<XUSER_MAX_COUNT;i++)
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
// for Internal use only
// ======================================================== 
InterfaceMgr::InterfaceMgr(MessageMgr* msgMgr)
{
	if (msgMgr)
	{
		msgMgr->Register(EMessage_CallBackInterface,this);
	}
}

#define GS_CALLBACK(_index, _ret) { if (GS_CallBackFunc[_index]) {(*(GS_CallBackFunc[_index]))(_ret);} }

GS_VOID InterfaceMgr::MessageResponse(Message* message)
{
	Assert(message->GetMessageID() == EMessage_CallBackInterface);

	GS_INT task_type = *(GS_INT*)message->ReadPayload(0);
	GS_INT result = *(GS_INT*)message->ReadPayload(1);

	GS_CALLBACK(task_type, result);
}

} // namespace GameService

