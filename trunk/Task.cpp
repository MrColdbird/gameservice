// ======================================================================================
// File         : Task.cpp
// Author       : Li Chen 
// Last Change  : 07/29/2010 | 15:48:39 PM | Thursday,July
// Description  : 
// ======================================================================================

#include "stdafx.h"
#include "Task.h"
#include "Message.h"
#include "Master.h"
#include "HttpSrv.h"
#include "LogFile.h"

namespace GameService
{

#define GET_TASKTYPE_NAME(type) \
case type:\
strcpy(out, #type);\
break;

void GetTaskTypeName(GS_TaskType type, GS_CHAR* out)
{
    switch(type)
    {
    default:
        strcpy(out, "Unknown TaskType");
        break;
    GET_TASKTYPE_NAME(EGSTaskType_ShowKeyboard_SetCustomizeMotto)
    GET_TASKTYPE_NAME(EGSTaskType_ShowKeyboard_SetReplayName)
    GET_TASKTYPE_NAME(EGSTaskType_ShowKeyboard_SetClanURL)
    GET_TASKTYPE_NAME(EGSTaskType_ShowKeyboard_SetClanMessageOfDay)
    GET_TASKTYPE_NAME(EGSTaskType_SysMsgBox_SaveDataNoFreeSpace)
    GET_TASKTYPE_NAME(EGSTaskType_SysMsgBox_TrophyNoFreeSpace)
    GET_TASKTYPE_NAME(EGSTaskType_SysMsgBox_PlayOtherUserSaveData)
    GET_TASKTYPE_NAME(EGSTaskType_SessionCreate)
    GET_TASKTYPE_NAME(EGSTaskType_SessionStart)
    GET_TASKTYPE_NAME(EGSTaskType_SessionJoin)
    GET_TASKTYPE_NAME(EGSTaskType_SessionEnd)
    GET_TASKTYPE_NAME(EGSTaskType_SessionLeave)
    GET_TASKTYPE_NAME(EGSTaskType_SessionDelete)
    GET_TASKTYPE_NAME(EGSTaskType_StatsRetrieveLocal)
    GET_TASKTYPE_NAME(EGSTaskType_StatsWrite)
    GET_TASKTYPE_NAME(EGSTaskType_StatsFlush)
    GET_TASKTYPE_NAME(EGSTaskType_StatsRead)
    GET_TASKTYPE_NAME(EGSTaskType_StatsReadFriend)
    GET_TASKTYPE_NAME(EGSTaskType_XContentCreate)
    GET_TASKTYPE_NAME(EGSTaskType_XContentClose)
    GET_TASKTYPE_NAME(EGSTaskType_XContentTestCreate)
    GET_TASKTYPE_NAME(EGSTaskType_XContentTestClose)
    GET_TASKTYPE_NAME(EGSTaskType_UnlockAchievement)
    GET_TASKTYPE_NAME(EGSTaskType_ReadAchievement)
    GET_TASKTYPE_NAME(EGSTaskType_Test)
    GET_TASKTYPE_NAME(EGSTaskType_GameClipRankCheck)
    GET_TASKTYPE_NAME(EGSTaskType_UpdateGameClipSize)
    GET_TASKTYPE_NAME(EGSTaskType_UploadGameClip)
    GET_TASKTYPE_NAME(EGSTaskType_DownloadGameClip)
    GET_TASKTYPE_NAME(EGSTaskType_CheckGameClipInfo)
    GET_TASKTYPE_NAME(EGSTaskType_ListFriend)
    GET_TASKTYPE_NAME(EGSTaskType_StorageService_SelectDevice)
    GET_TASKTYPE_NAME(EGSTaskType_StorageService_Check)
    GET_TASKTYPE_NAME(EGSTaskType_StorageService_CheckClose)
    GET_TASKTYPE_NAME(EGSTaskType_StorageService_Read)
    GET_TASKTYPE_NAME(EGSTaskType_StorageService_ReadClose)
    GET_TASKTYPE_NAME(EGSTaskType_StorageService_Write)
    GET_TASKTYPE_NAME(EGSTaskType_StorageService_WriteClose)
    GET_TASKTYPE_NAME(EGSTaskType_StorageService_Delete)
    GET_TASKTYPE_NAME(EGSTaskType_Remote_Read)
    GET_TASKTYPE_NAME(EGSTaskType_Remote_Refresh)
    GET_TASKTYPE_NAME(EGSTaskType_Remote_Write_Step_I)
    GET_TASKTYPE_NAME(EGSTaskType_Remote_Write_Step_II)
    GET_TASKTYPE_NAME(EGSTaskType_Remote_Write_Step_III)
    GET_TASKTYPE_NAME(EGSTaskType_Remote_Delete)
    GET_TASKTYPE_NAME(EGSTaskType_ReadProfile)
    GET_TASKTYPE_NAME(EGSTaskType_WriteProfile)
    GET_TASKTYPE_NAME(EGSTaskType_HostMigration)
    GET_TASKTYPE_NAME(EGSTaskType_GetFriendsFactionInfo)
    GET_TASKTYPE_NAME(EGSTaskType_GetPlayerFriends)
    GET_TASKTYPE_NAME(EGSTaskType_ViewReplay)
    GET_TASKTYPE_NAME(EGSTaskType_InputSpecialCode)
    GET_TASKTYPE_NAME(EGSTaskType_MarketplaceEnumeration)
    GET_TASKTYPE_NAME(EGSTaskType_ContentMgrEnumeration)
    GET_TASKTYPE_NAME(EGSTaskType_OfferMgrEnumeration)
    }
}

Task::Task(CTaskID taskId
#if defined(_PS3)
           , GS_INT ctxId
#endif
           )
: m_TaskRecipient(NULL)
, m_TaskID(taskId)
, m_TaskStarted(FALSE)
, m_TaskFinished(FALSE)
, m_TaskResult(0)
#if defined(_PS3)
 , m_sceTransId(ctxId)
#endif
{
#if defined(_XBOX) || defined(_XENON)
	ZeroMemory( &m_XOverlapped, sizeof( XOVERLAPPED ) );
#endif
}

void  Task::Start()
{
	m_TaskStarted = TRUE;
#if defined(_XBOX) || defined(_XENON)
	m_TaskResult = ERROR_IO_INCOMPLETE;
#elif PS3
	m_TaskResult = 0; // TO DO ...
#endif

    // get TaskType string for log
    GetTaskTypeName(GetTaskType(),m_cTypeName);
    Master::G()->Log("Task Started - %s", m_cTypeName);
}
void  Task::Stop()
{
	m_TaskFinished = TRUE;
}

GS_BOOL  Task::IsFinished()
{
	return m_TaskFinished;
}

GS_BOOL Task::Update()
{
	if ((!m_TaskStarted) || m_TaskFinished)
	{
		return FALSE;
	}

	GS_DWORD DetailedResult=0;
#if defined(_PS3)
    GS_INT bExecuting = 1;
    switch(GetTaskType())
    {
    case EGSTaskType_StatsRetrieveLocal:
    case EGSTaskType_StatsWrite:
    case EGSTaskType_StatsRead:
    case EGSTaskType_StatsReadFriend:
        bExecuting = sceNpScorePollAsync(m_sceTransId, &DetailedResult);
		break;
    case EGSTaskType_RetrieveImageFromHTTP:
        if (Master::G()->GetHttpSrv()->IsLastQueryFinished())
        {
            if (Master::G()->WriteLocalFile(GS_EFileIndex_TmpImg, Master::G()->GetHttpSrv()->GetRecvData(), Master::G()->GetHttpSrv()->GetRecvSize()))
                bExecuting = 0;
            else
                bExecuting = -1;
        }
        else
        {
            bExecuting = 1;
        }
        break;
	default:
        Master::G()->Log("Task::Update - Cannot do task for type: %d\n", GetTaskType());
        break;
    }

    if (1 == bExecuting)
    {
        // executing:
        return FALSE;
    }
    else if(bExecuting < 0)
    {
        // error occured:
        m_TaskResult = bExecuting;
    }
    else 
    {
        // finished:
        m_TaskResult = 0;
    }

#elif defined(_XBOX) || defined(_XENON)
	DWORD ExtendResult;

    switch(GetTaskType())
    {
    case EGSTaskType_RetrieveImageFromHTTP:
        if (Master::G()->GetHttpSrv()->IsLastQueryFinished())
        {
            if (Master::G()->WriteLocalFile(GS_EFileIndex_TmpImg, Master::G()->GetHttpSrv()->GetRecvData(), Master::G()->GetHttpSrv()->GetRecvSize()))
                m_TaskResult = ERROR_SUCCESS;
            else
                m_TaskResult = -1;
        }
        else
        {
            return FALSE;
        }
        break;
    default:
        if(!XHasOverlappedIoCompleted(&m_XOverlapped))
            return FALSE;

        m_TaskResult = XGetOverlappedResult(&m_XOverlapped, &DetailedResult, false );
        ExtendResult = XGetOverlappedExtendedError(&m_XOverlapped);
        break;
    }

#endif

	Message* message = NULL;

    switch(GetTaskType())
    {
    case EGSTaskType_RetrieveImageFromHTTP:
		if (m_TaskRecipient)
			message = Message::Create(EMessage_InternalTaskDone);
        break;
    default:
		if (m_TaskRecipient)
			message = Message::Create(EMessage_OnlineTaskDone);
        break;
    }

    Master::G()->Log("Task Finished - %s with %x", m_cTypeName, m_TaskResult);

	/*Message Payload
	0. task_id CTaskID
	1. taskType GS_TaskType
	2. OverlappedResult GS_DWORD
	3. DetailedResult GS_DWORD
	4. ExtendOverlappedResult GS_DWORD*/
	if (message)
	{
		message->AddPayload(m_TaskID);
		message->AddPayload((GS_INT)GetTaskType());
		message->AddPayload(m_TaskResult);
		message->AddPayload(DetailedResult);
#if defined(_XBOX) || defined(_XENON)
		message->AddPayload(ExtendResult);
#endif
		message->AddTarget(m_TaskRecipient);

		Master::G()->GetMessageMgr()->Send(message);
		m_TaskFinished = TRUE;
	}
	
	return m_TaskFinished;
}

void Task::Close()
{
	if ((m_TaskStarted) && (!m_TaskFinished))
		Cancel();
}

void Task::Cancel()
{
#if PS3
    // TODO:
    // AbortTransaction according to different API
	if( m_TaskStarted )
	{
		GS_DELETE this;
	}
#endif
#if defined(_XBOX) || defined(_XENON)
	if (m_TaskStarted && !XHasOverlappedIoCompleted(&m_XOverlapped))
	{
		GS_DWORD errcode = XCancelOverlapped(&m_XOverlapped);

		if (errcode != ERROR_SUCCESS)
		{
			//ONLINE_ERROR_LOG_FORCE(TEXT("Task::Cancel failed , task id(%d) , task type(%d) , error code (%d)"),(GS_INT)m_TaskID,(GS_INT)GetTaskType(),errcode);
		}
	}
#endif

}

TaskMgr::TaskMgr()
: m_Generator(0)
{
	for(GS_INT i=0;i<ONLINE_TASK_ARRAY_NUM;i++)
	{
		m_TasksArray[i] = NULL;
	}
}

TaskMgr::~TaskMgr()
{
	Stop();
}

void TaskMgr::Start()
{

}

void TaskMgr::Stop()
{
	//ONLINE_ERROR_LOG_FORCE(TEXT("TaskMgr::Stop"));

	for(GS_INT i=0;i<ONLINE_TASK_TYPE_NUM;i++)
	{
		if (m_TasksArray[i] != NULL)
		{
			RemoveTask(m_TasksArray[i]->GetTaskID());
		}
	}
}

void TaskMgr::UpdateAll()
{
	for(GS_INT i=0;i<ONLINE_TASK_ARRAY_NUM;i++)
	{
		if(m_TasksArray[i] && IsTaskIDValid(m_TasksArray[i]->GetTaskID()))
		{
			/*if(m_TasksArray[i]->Update())
			{
				m_TasksArray[i]->Close();
				m_TasksArray[i] = NULL;
			}*/

			m_TasksArray[i]->Update();

			if (m_TasksArray[i]->IsFinished())
			{
				RemoveTask(m_TasksArray[i]->GetTaskID());
			}
		}
	}
}

CTaskID TaskMgr::GenerateTaskID(GS_TaskType taskType, GS_BYTE pos)
{
	//if(taskType > 0 && taskType < ONLINE_TASK_TYPE_NUM && pos >= 0 && pos < ONLINE_TASK_ARRAY_NUM)
	if(++m_Generator == 0xFFFF)
		m_Generator = 0;

	return ( pos | ((taskType&0xFF) << 8) | ((m_Generator&0xFFFF) << 16) );
}

#if defined(_XBOX) || defined(_XENON)
PXOVERLAPPED TaskMgr::AddTask(GS_TaskType taskType, MessageRecipient* taskRecipient, CTaskID* taskId)
{
	if(taskType <= EGSTaskType_InValid || taskType >= ONLINE_TASK_TYPE_NUM)
		return NULL;

	UINT8 pos = 0;
	for(; pos < ONLINE_TASK_ARRAY_NUM; pos++)
	{
		if(NULL == m_TasksArray[pos])
			break;
	}

	if(ONLINE_TASK_ARRAY_NUM == pos)
	{
		return NULL;
	}

	CTaskID NewId = GenerateTaskID(taskType, pos);
	m_TasksArray[pos] = GS_NEW Task(NewId);

	if (taskId)
		*taskId = NewId;

	if (taskRecipient)
		m_TasksArray[pos]->m_TaskRecipient = taskRecipient;

	return m_TasksArray[pos]->GetXOverlapped();
}
void TaskMgr::StartTask(CTaskID taskId , GS_DWORD errorCode)
{
	UINT8 pos = GetPosInTaskArray(taskId);
	if(pos >= 0 && pos < ONLINE_TASK_ARRAY_NUM)
	{
		if(m_TasksArray[pos])
		{
			if (errorCode == ERROR_IO_PENDING)
			{
				m_TasksArray[pos]->Start();
			}
			else
			{
				m_TasksArray[pos]->Stop();
			}
		}
	}
	else
		GS_Assert(0);
}
#elif defined(_PS3)
void TaskMgr::AddTask(GS_TaskType taskType, GS_INT ctxId, MessageRecipient* taskRecipient, CTaskID* taskId)
{
	if(taskType <= EGSTaskType_InValid || taskType >= ONLINE_TASK_TYPE_NUM)
		return;

	GS_BYTE pos = 0;
	for(; pos < ONLINE_TASK_ARRAY_NUM; pos++)
	{
		if(NULL == m_TasksArray[pos])
			break;
	}

	if(ONLINE_TASK_ARRAY_NUM == pos)
	{
		return;
	}

	CTaskID NewId = GenerateTaskID(taskType, pos);
	m_TasksArray[pos] = GS_NEW Task(NewId, ctxId);

	if (taskId)
		*taskId = NewId;

	if (taskRecipient)
		m_TasksArray[pos]->m_TaskRecipient = taskRecipient;
}
void TaskMgr::StartTask(CTaskID taskId)
{
	GS_BYTE pos = GetPosInTaskArray(taskId);
	if(m_TasksArray[pos])
	{
        m_TasksArray[pos]->Start();
	}
}
#endif


void TaskMgr::RemoveTask(CTaskID taskId)
{
	GS_BYTE pos = GetPosInTaskArray(taskId);

	if(m_TasksArray[pos])
	{
		m_TasksArray[pos]->Close();
		GS_DELETE m_TasksArray[pos];
		m_TasksArray[pos] = NULL;
	}
}

void TaskMgr::CancelTask(CTaskID taskId)
{
	GS_BYTE pos = GetPosInTaskArray(taskId);

	if(m_TasksArray[pos])
	{
		m_TasksArray[pos]->Cancel();
	}
}

#if defined(_XBOX) || defined(_XENON)
XOVERLAPPED* TaskMgr::GetTaskXOverlapped(CTaskID taskId)
{
	UINT8 pos = GetPosInTaskArray(taskId);
	if(pos >= 0 && pos < ONLINE_TASK_ARRAY_NUM)
	{
		if(m_TasksArray[pos])
		{
			return m_TasksArray[pos]->GetXOverlapped();
		}
	}
	return NULL;
}

XPLAYERLIST_RESULT& TaskMgr::GetTaskCustomUIResult(CTaskID taskId)
{
	static XPLAYERLIST_RESULT for_return = {0,0};

	UINT8 pos = GetPosInTaskArray(taskId);
	if(pos >= 0 && pos < ONLINE_TASK_ARRAY_NUM)
	{
		if(m_TasksArray[pos])
		{
			return m_TasksArray[pos]->GetCustomUIResult();
		}
	}
	return for_return;
}
#endif

GS_DWORD TaskMgr::GetTaskResult(CTaskID taskId)
{
	GS_BYTE pos = GetPosInTaskArray(taskId);

	if(m_TasksArray[pos])
	{
		return m_TasksArray[pos]->GetResult();
	}

	return -1;
}

}
