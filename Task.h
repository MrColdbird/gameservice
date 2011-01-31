// ======================================================================================
// File         : Task.h
// Author       : Li Chen 
// Last Change  : 07/29/2010 | 15:48:44 PM | Thursday,July
// Description  : 
// ======================================================================================

#pragma once
#ifndef GAMESERVICE_TASK_H
#define GAMESERVICE_TASK_H

#include "TaskType.h"

namespace GameService
{

#define ONLINE_TASK_TYPE_NUM 128
#define ONLINE_TASK_ARRAY_NUM 128

typedef GS_INT CTaskID;

class MessageRecipient;
class Task
{
public:
	Task(CTaskID taskId
#if defined(_PS3)
	, GS_INT ctxId
#endif
		);

	GS_BOOL Update();
	void Close();
	void Cancel();
	CTaskID GetTaskID() { return m_TaskID; }
#if defined(_XBOX) || defined(_XENON)
	PXOVERLAPPED GetXOverlapped() { return &m_XOverlapped; }
	XPLAYERLIST_RESULT& GetCustomUIResult() { return m_CustomUI_Result; }
#endif

	GS_TaskType GetTaskType() { return (GS_TaskType)((m_TaskID & 0x0000FF00) >> 8); }
	GS_DWORD GetResult() { return m_TaskResult; }	
	void  Start();
	void  Stop();
	GS_BOOL  IsFinished();

	MessageRecipient* m_TaskRecipient;
	
private:
	CTaskID 	m_TaskID;
	GS_BOOL		m_TaskStarted,m_TaskFinished;
    GS_CHAR		m_cTypeName[128];

	GS_DWORD 		m_TaskResult;

#if defined(_XBOX) || defined(_XENON)
	XPLAYERLIST_RESULT	m_CustomUI_Result;
	XOVERLAPPED			m_XOverlapped;
#elif defined(_PS3)
    GS_INT         m_sceTransId;
#endif

};

class TaskMgr
{
public:
	TaskMgr();
	~TaskMgr();

	void Start();
	void Stop();

	void UpdateAll();

#if defined(_XBOX) || defined(_XENON)
	PXOVERLAPPED AddTask(GS_TaskType taskType, MessageRecipient* taskRecipient, CTaskID* taskId);
	PXOVERLAPPED StartNewTask(GS_TaskType taskType, MessageRecipient* taskRecipient);
	void StartTask(CTaskID taskId, GS_DWORD errorCode);

	XOVERLAPPED* GetTaskXOverlapped(CTaskID taskId);
	XPLAYERLIST_RESULT& GetTaskCustomUIResult(CTaskID taskId);
#elif defined(_PS3)
	void AddTask(GS_TaskType taskType, GS_INT ctxId, MessageRecipient* taskRecipient, CTaskID* taskId);
	void StartTask(CTaskID taskId);
#endif


	void RemoveTask(CTaskID taskId);
	void CancelTask(CTaskID taskId);
	GS_DWORD GetTaskResult(CTaskID taskId);
	GS_BYTE GetPosInTaskArray(CTaskID taskId) { return (taskId & 0x000000FF); }
	GS_TaskType GetTaskType(CTaskID taskId) { return (GS_TaskType)((taskId & 0x0000FF00) >> 8); }
	GS_BOOL IsTaskIDValid(CTaskID taskId) { return taskId != 0 && taskId != GS_INT(0xFFFFFFFF); }	

private:
	CTaskID GenerateTaskID(GS_TaskType taskType, GS_BYTE pos);

private:
	Task*	m_TasksArray[ONLINE_TASK_ARRAY_NUM];
	GS_UINT16	m_Generator;
};

} // namespace GameService

#endif //GAMESERVICE_Task_H
