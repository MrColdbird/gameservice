// ======================================================================================
// File         : Task.cpp
// Author       : Li Chen 
// Last Change  : 07/29/2010 | 15:48:39 PM | Thursday,July
// Description  : 
// ======================================================================================

#include "stdafx.h"
#include "Customize/CusMemory.h"
#include "Task.h"
#include "Message.h"
#include "Master.h"

namespace GameService
{

Task::Task(CTaskID TaskId
#if defined(_PS3)
           , GS_INT ctxId
#endif
           )
: m_TaskID(TaskId), m_TaskResult(0), m_TaskRecipient(NULL),m_TaskStarted(FALSE),m_TaskFinished(FALSE)
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
#ifdef _XBOX
	m_TaskResult = ERROR_IO_INCOMPLETE;
#elif PS3
	m_TaskResult = 0; // TO DO ...
#endif
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

#if defined(_PS3)
	GS_INT DetailedResult = 0;
    GS_INT bExecuting = 1;
    switch(GetTaskType())
    {
    case EGSTaskType_StatsRetrieveLocal:
    case EGSTaskType_StatsWrite:
    case EGSTaskType_StatsRead:
    case EGSTaskType_StatsReadFriend:
        bExecuting = sceNpScorePollAsync(m_sceTransId, &DetailedResult);
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

#else
	if(!XHasOverlappedIoCompleted(&m_XOverlapped))
		return FALSE;

	GS_DWORD DetailedResult,ExtendResult;
	m_TaskResult = XGetOverlappedResult(&m_XOverlapped, &DetailedResult, false );
	ExtendResult = XGetOverlappedExtendedError(&m_XOverlapped);
#endif

	Message* message = NULL;

	if(GetTaskType() < EGSTaskType_ShowXUI_Max)
	{
		// TODO:
		assert(0);

//		if(!((OfOnlineService *)GEngine->OnlineService)->GetNotificationHandler()->IsDashBoardUsed())
//		{
//			if(GetTaskType() < EGSTaskType_ShowXUI_ShowKeybord_Max) // want to string verify:
//			{
//#if PS3	//++GengYong
//				if (((OfOnlineService *)GEngine->OnlineService)->GetVirtualKeyboard()->ProbeKeyboardData())
//				{
//					//TaskStatus = ERROR_SUCCESS;
//#endif	//--GengYnog
//					GS_UINT user_index;
//					FString result_str;
//					((OfOnlineService *)GEngine->OnlineService)->GetVirtualKeyboard()->RetrieveKeyboardData(user_index,result_str);
//					if(FALSE == ((OfOnlineService *)GEngine->OnlineService)->VerifyStringSyc(result_str))
//					{
//						SEND_UI_EVENT(UIEventType_ONLINE_COMMON,EOnline2UIEvent_Common_StringVerifyFailed);
//						m_TaskFinished = TRUE;
//						return TRUE;
//					}
//#if PS3	//++GengYong
//				}
//				else
//				{
//					//TaskStatus = ~ERROR_SUCCESS;
//					return FALSE;
//				}
//#endif	//--GengYnog
//			}
//
//			if (m_TaskRecipient)
//				message = OnlineMessage::Create(EOnlineMessage_ShowXUIDone);
//			//message->AddPayload(CTaskID);
//			//OnlineMessageMgr::Get()->Send(message);
//			//m_TaskFinished = TRUE;
//			//return TRUE;
//		}
//
//		//return FALSE;
	}
	else
	{
		if (m_TaskRecipient)
			message = Message::Create(EMessage_OnlineTaskDone);
		//message->AddPayload(CTaskID);
		//OnlineMessageMgr::Get()->Send(message);
		//m_TaskFinished = TRUE;
		//return TRUE;
	}

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
		Delete<Task>(this);
	}
#endif
#ifdef _XBOX
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

#ifdef _XBOX
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
	m_TasksArray[pos] = new(GSOPType) Task(NewId);

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
		assert(0);
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
	m_TasksArray[pos] = new(GSOPType) Task(NewId, ctxId);

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
		Delete<Task>(m_TasksArray[pos]);
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

#ifdef _XBOX
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
