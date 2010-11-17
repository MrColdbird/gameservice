// ======================================================================================
// File         : TaskType.h
// Author       : Li Chen 
// Last Change  : 07/29/2010 | 15:48:50 PM | Thursday,July
// Description  : 
// ======================================================================================

#ifndef GAMESERVICE_TASKTYPE_H
#define GAMESERVICE_TASKTYPE_H

enum GS_TaskType
{
	EGSTaskType_InValid = 0,
	EGSTaskType_ShowKeyboard_SetCustomizeMotto,
	EGSTaskType_ShowKeyboard_SetReplayName,
	EGSTaskType_ShowKeyboard_SetClanURL,
	EGSTaskType_ShowKeyboard_SetClanMessageOfDay,
	EGSTaskType_ShowXUI_ShowKeybord_Max, // for judgment cannot use!
	EGSTaskType_ShowCustomUI_InviteToClan,
	EGSTaskType_ShowCustomUI_InviteToGame,
	EGSTaskType_ShowCustomUI_InviteToPlaygroupByFriendList,
	EGSTaskType_ShowXUI_Max, // for judgment cannot use!
	EGSTaskType_SysMsgBox_SaveDataNoFreeSpace,
	EGSTaskType_SysMsgBox_TrophyNoFreeSpace,
	EGSTaskType_SysMsgBox_PlayOtherUserSaveData,
	EGSTaskType_SessionCreate,
	EGSTaskType_SessionStart,
	EGSTaskType_SessionJoin,
	EGSTaskType_SessionEnd,
	EGSTaskType_SessionLeave,
	EGSTaskType_SessionDelete,
	EGSTaskType_StatsRetrieveLocal,
	EGSTaskType_StatsWrite,
	EGSTaskType_StatsFlush,
	EGSTaskType_StatsRead,
	EGSTaskType_StatsReadFriend,
	EGSTaskType_XContentCreate,
	EGSTaskType_XContentClose,
	EGSTaskType_XContentTestCreate,
	EGSTaskType_XContentTestClose,
	EGSTaskType_UnlockAchievement,
	EGSTaskType_ReadAchievement,
	EGSTaskType_Test,
	EGSTaskType_GameClipRankCheck,
	EGSTaskType_UpdateGameClipSize,
	EGSTaskType_UploadGameClip,
	EGSTaskType_DownloadGameClip,
	EGSTaskType_CheckGameClipInfo,
	EGSTaskType_ListFriend,
	EGSTaskType_StorageService_SelectDevice,
	EGSTaskType_StorageService_Check,
	EGSTaskType_StorageService_CheckClose,
	EGSTaskType_StorageService_Read,
	EGSTaskType_StorageService_ReadClose,
	EGSTaskType_StorageService_Write,
	EGSTaskType_StorageService_WriteClose,
	EGSTaskType_StorageService_Delete,
	EGSTaskType_Remote_Read,
	EGSTaskType_Remote_Refresh,
	EGSTaskType_Remote_Write_Step_I,
	EGSTaskType_Remote_Write_Step_II,
	EGSTaskType_Remote_Write_Step_III,
	EGSTaskType_Remote_Delete,
	EGSTaskType_ReadProfile,
	EGSTaskType_WriteProfile,
	EGSTaskType_HostMigration,
	EGSTaskType_GetFriendsFactionInfo,
	EGSTaskType_GetPlayerFriends,
	EGSTaskType_ViewReplay,
	EGSTaskType_InputSpecialCode,
	EGSTaskType_MarketplaceEnumeration,
	EGSTaskType_MAX,
};

#endif // GAMESERVICE_TASKTYPE_H
