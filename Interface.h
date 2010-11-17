// ======================================================================================
// File         : Interface.h
// Author       : Li Chen 
// Last Change  : 07/29/2010 | 15:47:24 PM | Thursday,July
// Description  : 
// ======================================================================================

#ifndef GAMESERVICE_INTERFACE_H
#define GAMESERVICE_INTERFACE_H

#if defined(_XENON) || defined(_XBOX)
#include <xtl.h>
#include <xboxmath.h>
#elif defined(_PS3)
#include <np.h>
#endif

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

void GetConsoleLangLocaleAbbr(char* output);

// callback mechanism:
// TODO: callback function definition
typedef void (*GSCB_Func)(bool);

// Interface functions:
void GS_Initialize(bool requireOnline, int achieveCount, int versionId, bool bTrial, int bDumpLog
#if defined(_PS3)
                   , int iFreeSpaceAvail
#endif
                   );
void GS_Update(void);
void GS_Destroy(void);

// Profile fucntions:
bool GS_NotifyNoProfileSignedIn();
int GS_GetSignedInUserCount();
void GS_SetBeforePressStart(int before);
int GetBeforePressStart();
#if defined(_PS3)
int GS_GetUserAge();
int GS_GetUserLanguage();
#endif
int GS_GetSystemLanguage();
int IsCableConnected();
void RequestSignIn();
#if defined(_XBOX) || defined(_XENON)
void PressStartButton(int userIndex);
#endif

// session:
void GS_CreateSession(GSCB_Func fun_cb);
void GS_JoinSession(GSCB_Func fun_cb);
void GS_StartSession(GSCB_Func fun_cb);
void GS_EndSession(GSCB_Func fun_cb);
void GS_LeaveSession(GSCB_Func fun_cb);
void GS_DeleteSession(GSCB_Func fun_cb);
bool GS_IsSessionCreated();
bool GS_IsSessionStarted();

// leaderboard definition:
void GS_RetrieveLocalStats(int immediately, int boardId, int columnNum, int* columnIds, GSCB_Func fun_cb);
int GS_ReadLeaderboardFinished();
int GS_GetLeaderboardCount();
char* GS_GetLeaderboardName(int index);
int GS_GetLeaderboardRank(int index);
int GS_GetLeaderboardScore(int index);
int GetStatsErrorCode();
#if defined(_XBOX) || defined (_XENON)
ULONGLONG GS_GetKeyValueFromStats(int lbIndex);
void GS_WriteStats(GSCB_Func fun_cb, int lbNum, XSESSION_VIEW_PROPERTIES* views);
void GS_FlushStats(GSCB_Func fun_cb);
bool GS_ReadStats(int boardId, int columnNum, int* columnIds, int rankIdx, int userIndex, int maxRowNum, int myScoreOffset, GSCB_Func fun_cb);
bool GS_ReadFriendsStats(int boardId, int columnNum, int* columnIds, int rankIdx, int userIndex, int maxRowNum, GSCB_Func fun_cb);
bool GS_CanWriteStats();
#elif defined(_PS3)
int GS_GetKeyValueFromStats(int lbIndex);
void GS_WriteStats(GSCB_Func fun_cb, int iLBId, int score);
bool GS_ReadStats(int boardId, int score, int rankIdx, int userIndex, int maxRowNum, int myScoreOffset, GSCB_Func fun_cb);
bool GS_ReadFriendsStats(int boardId, int score, int rankIdx, int userIndex, int maxRowNum, GSCB_Func fun_cb);
#endif
void GS_DebugOutputStats();
void GS_ShowGamerCard(int index);

// achievement:
void GS_WriteAchievement(int num, int* ids);
void GS_ShowAchievementUI();
bool GS_IsAchievementEarned(int index);
bool HasAchievementRead();
void GS_UnlockFullGame();
bool GS_IsUserSignInLive();

// tracking function:
bool GS_TrackingSendTag(char* tag, char* attributes);

// sysmsgbox function:
void ShowSysMsgBox_SaveDataNoFreeSpace(GSCB_Func fun_cb, int needExtraKB);
void ShowSysMsgBox_KeyFileCorrupted();
void ShowSysMsgBox_TrophyNoFreeSpace();
void ShowSysMsgBox_PlayOtherUserSaveData();
void ShowSysMsgBox_PlayerAgeForbidden();

// rich presence
#if defined(_XBOX) || defined(_XENON)
void SetRichPresenceMode(int userIndex, int mode);
void UpdateDefaultPresenceInfo(int defaultInfo, int activeInfo);
#endif

// for log outside
GS_VOID Log(const GS_CHAR* strFormat, ...);
    
void GS_StorageDeviceReset();
int AreUsersSignedIn();

#ifdef __cplusplus
} // extern "C"
} // namespace GameService
#endif

#endif

#endif // GAMESERVICE_INTERFACE_H
