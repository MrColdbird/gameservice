// ======================================================================================
// File         : GS_INTerface.h
// Author       : Li Chen 
// Last Change  : 07/29/2010 | 15:47:24 PM | Thursday,July
// Description  : 
// ======================================================================================

#ifndef GAMESERVICE_GS_INTERFACE_H
#define GAMESERVICE_GS_INTERFACE_H

#if defined(_XENON) || defined(_XBOX)
#include <xtl.h>
#include <xboxmath.h>
#elif defined(_PS3)
#include <np.h>
#endif

//#define INGAMEBROWSING
#define SCEA_SUBMISSION 0
#define SCEE_SUBMISSION 1

#if SCEA_SUBMISSION
#define NP_GUI_PRODUCT_ID "UP0001-NPUB30394_00-KEY0000000000001"
#elif SCEE_SUBMISSION
#define NP_GUI_PRODUCT_ID "EP0001-NPEB00435_00-KEY0000000000001"
#else
#define NP_GUI_PRODUCT_ID "UP0001-NPXX00865_00-KEY0000000000001"
#endif


// ======================================================== 
// Tracking system trigger:
// ======================================================== 
//#define GAMESERVIE_TRACKING_ENABLE 1


#if defined(_XBOX) || defined(_XENON) || defined(_PS3)

#include "Utils.h"

#ifdef __cplusplus
namespace GameService
{

extern "C" {
#endif

#include "TaskType.h"

// Language enum
enum
{
    GS_ELang_English = 1,
    GS_ELang_Japanese,
    GS_ELang_French,
    GS_ELang_Spanish,
    GS_ELang_German,
    GS_ELang_Italian,
    GS_ELang_Dutch,
    GS_ELang_Portugese,
    GS_ELang_Russian,
    GS_ELang_Korean,
    GS_ELang_Chinese_t,
    GS_ELang_Chinese_s,
    GS_ELang_Finish,
    GS_ELang_Swedish,
    GS_ELang_Danish,
    GS_ELang_Norwegian,
    GS_ELang_Polish,
    GS_ELang_MAX
};

// Tracking enum
enum 
{
    GS_ETrackingTag_GAMESTART = 1,
	GS_ETrackingTag_LEVELSTART,
	GS_ETrackingTag_LEVELSTOP,
	GS_ETrackingTag_GAMECOMPLETE,
    GS_ETrackingTag_AWARD_UNLOCK,
};

GS_VOID GetConsoleLangLocaleAbbr(char* output);

// callback mechanism:
// TODO: callback function definition
typedef void (*GSCB_Func)(int);

// GS_INTerface functions:
GS_VOID Initialize(GS_BOOL requireOnline, GS_INT achieveCount, GS_INT versionId, GS_BOOL bTrial, GS_INT bDumpLog
#if defined(_PS3)
                   , GS_INT iFreeSpaceAvail
#endif
                   );
GS_VOID Update(GS_VOID);
GS_VOID Destroy(GS_VOID);

// Profile fucntions:
GS_BOOL NotifyNoProfileSignedIn();
GS_INT GetSignedInUserCount();
GS_VOID SetBeforePressStart(GS_INT before);
GS_INT GetBeforePressStart();
#if defined(_PS3)
GS_INT GetUserAge();
GS_INT GetUserLanguage();
#endif
GS_INT GetSystemLanguage();
GS_INT IsCableConnected();
GS_VOID RequestSignIn();
#if defined(_XBOX) || defined(_XENON)
GS_VOID PressStartButton(GS_INT userIndex);
#endif

// session:
GS_VOID CreateSession(GSCB_Func fun_cb);
GS_VOID JoinSession(GSCB_Func fun_cb);
GS_VOID StartSession(GSCB_Func fun_cb);
GS_VOID EndSession(GSCB_Func fun_cb);
GS_VOID LeaveSession(GSCB_Func fun_cb);
GS_VOID DeleteSession(GSCB_Func fun_cb);
GS_BOOL IsSessionCreated();
GS_BOOL IsSessionStarted();

// leaderboard definition:
GS_VOID RetrieveLocalStats(GS_INT immediately, GS_INT boardId, GS_INT columnNum, GS_INT* columnIds, GSCB_Func fun_cb);
GS_INT ReadLeaderboardFinished();
GS_INT GetLeaderboardCount();
char* GetLeaderboardName(GS_INT index);
GS_INT GetLeaderboardRank(GS_INT index);
GS_INT GetLeaderboardScore(GS_INT index);
GS_INT GetStatsErrorCode();
#if defined(_XBOX) || defined (_XENON)
ULONGLONG GetKeyValueFromStats(GS_INT lbIndex);
GS_VOID WriteStats(GSCB_Func fun_cb, GS_INT lbNum, XSESSION_VIEW_PROPERTIES* views);
GS_VOID FlushStats(GSCB_Func fun_cb);
GS_BOOL ReadStats(GS_INT boardId, GS_INT columnNum, GS_INT* columnIds, GS_INT rankIdx, GS_INT userIndex, GS_INT maxRowNum, GS_INT myScoreOffset, GSCB_Func fun_cb);
GS_BOOL ReadFriendsStats(GS_INT boardId, GS_INT columnNum, GS_INT* columnIds, GS_INT rankIdx, GS_INT userIndex, GS_INT maxRowNum, GSCB_Func fun_cb);
GS_BOOL CanWriteStats();
#elif defined(_PS3)
GS_INT GetKeyValueFromStats(GS_INT lbIndex);
GS_VOID WriteStats(GSCB_Func fun_cb, GS_INT iLBId, GS_INT score);
GS_BOOL ReadStats(GS_INT boardId, GS_INT score, GS_INT rankIdx, GS_INT userIndex, GS_INT maxRowNum, GS_INT myScoreOffset, GSCB_Func fun_cb);
GS_BOOL ReadFriendsStats(GS_INT boardId, GS_INT score, GS_INT rankIdx, GS_INT userIndex, GS_INT maxRowNum, GSCB_Func fun_cb);
#endif
GS_VOID DebugOutputStats();
GS_VOID ShowGamerCard(GS_INT index);

// achievement:
GS_VOID WriteAchievement(GS_INT num, GS_INT* ids);
GS_VOID ShowAchievementUI();
GS_BOOL IsAchievementEarned(GS_INT index);
GS_BOOL HasAchievementRead();
GS_VOID UnlockFullGame();
GS_BOOL IsUserSignInLive();

// tracking function:
GS_BOOL TrackingSendTag(char* tag, char* attributes);

// sysmsgbox function:
GS_VOID ShowSysMsgBox_SaveDataNoFreeSpace(GSCB_Func fun_cb, GS_INT needExtraKB);
GS_VOID ShowSysMsgBox_KeyFileCorrupted();
GS_VOID ShowSysMsgBox_TrophyNoFreeSpace();
GS_VOID ShowSysMsgBox_PlayOtherUserSaveData();
GS_VOID ShowSysMsgBox_PlayerAgeForbidden();

// rich presence
#if defined(_XBOX) || defined(_XENON)
GS_VOID SetRichPresenceMode(GS_INT userIndex, GS_INT mode);
GS_VOID UpdateDefaultPresenceInfo(GS_INT defaultInfo, GS_INT activeInfo);
#endif

// for log outside
GS_VOID Log(const GS_CHAR* strFormat, ...);

GS_INT AreUsersSignedIn();

#ifdef __cplusplus
} // extern "C"
} // namespace GameService
#endif

#endif

#endif // GAMESERVICE_GS_INTERFACE_H
