// ======================================================================================
// File         : Achievement.cpp
// Author       : Li Chen 
// Last Change  : 07/29/2010 | 15:44:25 PM | Thursday,July
// Description  : 
// ======================================================================================

#include "stdafx.h"
#include "Achievement.h"
#include "SignIn.h"
#include "Task.h"
#include "Master.h"
#include "SysMsgBox.h"

namespace GameService
{

AchievementSrv::AchievementSrv(MessageMgr* msgMgr, GS_INT count
#if defined(_PS3)
                               , GS_INT freespace
#endif
                               )
: m_dwCountMax(count)
#if defined(_XBOX) || defined(_XENON)
    , m_Achievements(NULL) 
	, m_hReadHandle(NULL)
#elif defined(_PS3)
	, m_TrophyCtx(SCE_NP_TROPHY_INVALID_CONTEXT)
	, m_TrophyHandle(SCE_NP_TROPHY_INVALID_HANDLE)
    , m_TrophyDetails(NULL)
	, m_TrophyData(NULL)
	, m_iTrophySpace(0)
	, m_iFreeSpaceAvailable(freespace)
#endif
{
#if defined(_XBOX) || defined(_XENON)
    m_Achievements = GS_NEW GS_BYTE[XACHIEVEMENT_SIZE_FULL * m_dwCountMax];
#elif defined(_PS3)
    m_TrophyDetails = GS_NEW SceNpTrophyDetails[m_dwCountMax];
    m_TrophyData = GS_NEW SceNpTrophyData[m_dwCountMax];

    m_iWantUnlockIDs = NULL;
    m_iWantUnlockIDNum = 0;
#endif

	if (msgMgr)
	{
		msgMgr->Register(EMessage_SignInChanged,this);
		msgMgr->Register(EMessage_OnlineTaskDone,this);
	}

    m_bHasRead = FALSE;
}

AchievementSrv::~AchievementSrv()
{
    Finalize();

#if defined(_XBOX) || defined(_XENON)
	if(m_Achievements)
	{
		GS_DELETE [] m_Achievements;
		m_Achievements = NULL;
	}
#elif defined(_PS3)
    if (m_TrophyDetails)
    {
        GS_DELETE [] m_TrophyDetails;
        m_TrophyDetails = NULL;
    }
    if (m_TrophyData)
    {
        GS_DELETE [] m_TrophyData;
        m_TrophyData = NULL;
    }
#endif

	m_dwCountMax = 0;
}

#if defined(_PS3)
int PS3TrophyCb(SceNpTrophyContext context, SceNpTrophyStatus status, int completed, int total, void *arg)
{
    return 0;
}
#endif

GS_BOOL AchievementSrv::Initialize()
{
#if defined(_XBOX) || defined(_XENON)
    m_hReadHandle = NULL;

#elif defined(_PS3)
	int ret;

	ret = sceNpTrophyInit(NULL, 0, SYS_MEMORY_CONTAINER_ID_INVALID, 0);
	if (ret < 0) {
		Master::G()->Log("sceNpTrophyInit() failed. ret = 0x%x", ret);
		return FALSE;
	}

	ret = sceNpTrophyCreateContext(
		&m_TrophyCtx,
		SignIn::GetNpCommID(),
		SignIn::GetNpCommSig(),
		0);
	if (ret < 0) {
        m_TrophyHandle = SCE_NP_TROPHY_INVALID_HANDLE;

		Master::G()->Log("sceNpTrophyCreateContext() failed. ret = 0x%x", ret);
		return FALSE;
	}

	ret = sceNpTrophyCreateHandle(&m_TrophyHandle);
	if (ret < 0) {
        m_TrophyCtx = SCE_NP_TROPHY_INVALID_CONTEXT;

		Master::G()->Log("sceNpTrophyCreateHandle() failed. ret = 0x%x", ret);
		return FALSE;
	}

    ret = sceNpTrophyGetRequiredDiskSpace(m_TrophyCtx,m_TrophyHandle,&m_iTrophySpace,0);
    if (ret < 0)
    {
		Master::G()->Log("sceNpTrophyGetRequiredDiskSpace() failed. ret = 0x%x", ret);
		return FALSE;
    }
    m_iTrophySpace /= 1024;

    if (m_iFreeSpaceAvailable < 0)
    {
        GS_INT need_space = (GS_INT)m_iTrophySpace - m_iFreeSpaceAvailable;
        Master::G()->GetSysMsgBoxManager()->Display(EMODE_SaveDataNoSpace, &need_space);
		Master::G()->Log("No Free Space for SaveData and Trophy: %d Bytes.", need_space);
        return FALSE;
    }

	ret = sceNpTrophyRegisterContext(
		m_TrophyCtx,
		m_TrophyHandle,
		PS3TrophyCb,
		NULL,
		0);
	if (ret < 0) 
    {
		if (ret == (int)SCE_NP_TROPHY_ERROR_ABORT) 
        {
            // aborted = true;
		}

        if (ret == (int)SCE_NP_TROPHY_ERROR_INSUFFICIENT_DISK_SPACE)
        {
            GS_INT need_space = (GS_INT)m_iTrophySpace - m_iFreeSpaceAvailable;
            Master::G()->GetSysMsgBoxManager()->Display(EMODE_SaveDataNoSpace, &need_space);
            Master::G()->Log("No Free Space for Trophy: %d Bytes.", need_space);
        }

		Master::G()->Log("sceNpTrophyRegisterContext() failed. ret = 0x%x", ret);
		return FALSE;
	}

    // aborted = false;
#endif

	return TRUE;
}

void AchievementSrv::Finalize()
{
#if defined(_PS3)
    if (Master::G()->IsForceQuit())
        return;

	int ret;

	if (m_TrophyHandle != SCE_NP_TROPHY_INVALID_HANDLE) {
		ret = sceNpTrophyDestroyHandle(m_TrophyHandle);
		if (ret < 0) {
			if (ret == (int)SCE_NP_TROPHY_ERROR_ABORT) {
                // aborted = true;
			}
			Master::G()->Log("sceNpTrophyDestroyHandle() failed. ret = 0x%x", ret);
		}
	}

	if (m_TrophyCtx != SCE_NP_TROPHY_INVALID_CONTEXT) {
		ret = sceNpTrophyDestroyContext(m_TrophyCtx);
		if (ret < 0) {
			if (ret == (int)SCE_NP_TROPHY_ERROR_ABORT) {
                // aborted = true;
			}
			Master::G()->Log("sceNpTrophyDestroyContext() failed. ret = 0x%x", ret);
		}
	}

    sceNpTrophyTerm();
#endif
}

void AchievementSrv::ReadAchievements( GS_UINT userIndex, GS_INT startIndex, GS_INT count )
{
#if defined(_XBOX) || defined(_XENON)
    if (m_hReadHandle != NULL)
    {
        return;
    }

    // Create enumerator for the default device
    if (!SignIn::IsUserOnline(userIndex))
    {
		return;
    }

    m_bHasRead = FALSE;

    GS_DWORD cbBuffer;
    GS_DWORD dwStatus;

    XUID xid = INVALID_XUID;
	// TODO: consider current user
    xid = SignIn::GetXUID(userIndex); 

    dwStatus = XUserCreateAchievementEnumerator(
        0,                              // Enumerate for the current title
        userIndex,
        xid,                            // If INVALID_XUID, the current user's achievements
                                        // are enumerated
        XACHIEVEMENT_DETAILS_ALL,
        startIndex,                     // starting achievement index
        count,                          // number of achievements to return
        &cbBuffer,                      // bytes needed
        &m_hReadHandle );

    GS_Assert( dwStatus == ERROR_SUCCESS );

    // Enumerate achievements
    XACHIEVEMENT_DETAILS* rgAchievements = ( XACHIEVEMENT_DETAILS* )m_Achievements;
	CTaskID id = 0;
    dwStatus = XEnumerate( m_hReadHandle, &(rgAchievements[startIndex]), XACHIEVEMENT_SIZE_FULL * count,
                    0, Master::G()->GetTaskMgr()->AddTask(EGSTaskType_ReadAchievement,this,&id) );
	Master::G()->GetTaskMgr()->StartTask(id,dwStatus);

    if( dwStatus != ERROR_IO_PENDING )
    {
		Master::G()->Log("XEnumerate() failed with error %d", dwStatus);
    }

#elif defined(_PS3)
    m_bHasRead = FALSE;

	int ret;

    memset(&m_TrophyDetails[startIndex], 0x00, sizeof(SceNpTrophyDetails)*count);
    memset(&m_TrophyData[startIndex], 0x00, sizeof(SceNpTrophyData)*count);

    for (GS_INT i=startIndex; i<startIndex+count; i++)
    {
        ret = sceNpTrophyGetTrophyInfo(m_TrophyCtx, m_TrophyHandle, i, &m_TrophyDetails[i], &m_TrophyData[i]);
        if (ret < 0) {
            if (ret == (int)SCE_NP_TROPHY_ERROR_ABORT) {
                // aborted = true;
				return;
            }
            Master::G()->Log("sceNetTrophyGetFlagInfo() failed %x\n", ret);
        }
    }

    m_bHasRead = TRUE;
#endif
}

GS_BOOL AchievementSrv::IsEarned(GS_INT achieveIndex)
{
    if (!m_bHasRead)
        return FALSE;

#if defined(_XBOX) || defined(_XENON)
    XACHIEVEMENT_DETAILS* rgAchievements = ( XACHIEVEMENT_DETAILS* )m_Achievements;

    for (GS_UINT i=0; i<m_dwCountMax; i++)
    {
        if (achieveIndex == rgAchievements[i].dwId)
        {
            return AchievementEarnedOnline(rgAchievements[i].dwFlags) 
                    || AchievementEarned(rgAchievements[i].dwFlags);
        }
    }

    return FALSE;

#elif defined(_PS3)
    return m_TrophyData[achieveIndex-1].unlocked==true ? 1:0;
#else
	return FALSE;
#endif
}

// running sychronized
void AchievementSrv::Write(GS_INT num, GS_INT* ids)
{
#if defined(_XBOX) || defined(_XENON)
	// prepare for writing achievements
    //HANDLE hEventComplete = CreateEvent( NULL, FALSE, FALSE, NULL );

    //if( hEventComplete == NULL )
    //{
    //    Master::G()->Log("GS Achievement: Couldn't create event.\n");
    //    return;
    //}

    //XOVERLAPPED xov;

    //ZeroMemory( &xov, sizeof( XOVERLAPPED ) );
    //xov.hEvent = hEventComplete;

    // determine which achievements have occured
    XUSER_ACHIEVEMENT* pAchievements = GS_NEW XUSER_ACHIEVEMENT[num];

	for (GS_INT i=0;i<num;i++)
	{
		pAchievements[i].dwUserIndex = SignIn::GetActiveUserIndex();
		pAchievements[i].dwAchievementId = ids[i];
	}

    //
    // Write achievements
    //
    // Before writing an achievement, XContentGetCreator must be called on the
    // loaded save game to verify that the current user is in fact the same user
    // who created the save game. Only call XUserWriteAchievements if this is
    // true, otherwise the title is violating TCR 069 [GP No Sharing of Achievements]
    //

	// TODO: change below to async mode!

    GS_DWORD dwStatus = XUserWriteAchievements( num, pAchievements, NULL );
    //GS_Assert( dwStatus == ERROR_IO_PENDING );

    //dwStatus = XGetOverlappedResult( &xov, NULL, TRUE );
    //GS_Assert( dwStatus == ERROR_SUCCESS );

	GS_DELETE [] pAchievements;

    //CloseHandle( hEventComplete );

    Master::G()->Log("Achievement:(%d) has been unlocked!", ids[0]);

    // after write, read to make sychronization
    ReadAchievements(SignIn::GetSignedInUser(), 0, m_dwCountMax);
#elif defined(_PS3)

    if (m_iWantUnlockIDs)
        GS_DELETE [] m_iWantUnlockIDs;

    m_iWantUnlockIDNum = num;
    m_iWantUnlockIDs = GS_NEW GS_INT[num];
    for (GS_INT i=0; i<num; i++)
    {
        // make compatible with 360 version
        m_iWantUnlockIDs[i] = ids[i]-1;
    }

    sys_ppu_thread_t temp_id;
	int ret = -1;
    ret = sys_ppu_thread_create(
        &temp_id, PS3_WriteAchievement_Thread,
        (uintptr_t) this, THREAD_PRIO, STACK_SIZE,
        0, "GameService ReadAchievement Thread");
    if (ret < 0) {
        Master::G()->Log("[GameService] - sys_ppu_thread_create() failed (%x)", ret);
    }

#endif
}

#if defined(_PS3)
void AchievementSrv::WriteAchievment_PS3()
{
    int ret;
    SceNpTrophyId pid = SCE_NP_TROPHY_INVALID_TROPHY_ID;

    for (GS_INT i=0;i<m_iWantUnlockIDNum;i++)
    {
        ret = sceNpTrophyUnlockTrophy(m_TrophyCtx, m_TrophyHandle, m_iWantUnlockIDs[i], &pid);
        if (ret < 0) {
            if (ret == (int)SCE_NP_TROPHY_ERROR_ABORT) {
                // aborted = true;
            }
            Master::G()->Log("sceNpTrophyUnlockTrophy() index:(%d) failed (%x)", ret);
        }

        if (pid != SCE_NP_TROPHY_INVALID_TROPHY_ID) {
            Master::G()->Log("Platinum Trophy(%d) was unlocked!", pid);
            //platinumId = pid;
        }

        Master::G()->Log("Trophy:(%d) has been unlocked!", m_iWantUnlockIDs[i]);
    }
	
}
void AchievementSrv::PS3_WriteAchievement_Thread(uint64_t instance)
{
    // after write, read to make sychronization
    AchievementSrv* real_ins = (AchievementSrv*)instance;
    real_ins->WriteAchievment_PS3();
    //real_ins->ReadAchievements(0, 0, real_ins->GetCountMax());

    sys_ppu_thread_exit(0);
}
#endif

void AchievementSrv::ShowSystemUI(GS_INT userIndex)
{
#if defined(_XBOX) || defined(_XENON)
    XShowAchievementsUI(userIndex);
#else
#endif
}

void AchievementSrv::MessageResponse(Message* message)
{
    if (EMessage_SignInChanged == message->GetMessageID())
    {
        // signed in
        /*
        if (1 == *(GS_INT*)message->ReadPayload(0))
        {
            // retrieve all achievments
#if defined(_XBOX) || defined(_XENON)
            ReadAchievements(SignIn::GetActiveUserIndex(), 0, m_dwCountMax);
#elif defined(_PS3)
			ReadAchievements(0,0,m_dwCountMax);
#endif
        }
        */
        return;
    }

	CTaskID task_id = *(CTaskID*)message->ReadPayload(0);
	GS_TaskType taskType = (GS_TaskType)(*(GS_INT*)message->ReadPayload(1));
	GS_DWORD taskResult = *(GS_DWORD*)message->ReadPayload(2);
	GS_DWORD taskDetailedResult = *(GS_DWORD*)message->ReadPayload(3);
#if defined(_XBOX) || defined(_XENON)
	GS_DWORD taskExtendedResult = *(GS_DWORD*)message->ReadPayload(4);
#endif

#if defined(_XBOX) || defined(_XENON)
    if (taskResult == ERROR_SUCCESS)
#elif defined(_PS3)
    if (0 == taskResult && taskDetailedResult >= 0)
#endif
    {
        switch(taskType)
        {
        case EGSTaskType_ReadAchievement:
#if defined(_XBOX) || defined(_XENON)
            {
                if (m_hReadHandle != NULL)
                    CloseHandle( m_hReadHandle );

                m_hReadHandle = NULL;
            }
#endif
            m_bHasRead = TRUE;
            break;
		default:
			break;
        }
    }
}

} // namespace GameService
