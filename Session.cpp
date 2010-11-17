// ======================================================================================
// File         : Session.cpp
// Author       : Li Chen 
// Last Change  : 07/29/2010 | 15:48:02 PM | Thursday,July
// Description  : 
// ======================================================================================

#include "stdafx.h"
#include "InterfaceMgr.h"
#include "SignIn.h"
#include "Task.h"
#include "Session.h"
#include "Master.h"
#include "Stats.h"

namespace GameService
{

//--------------------------------------------------------------------------------------
// Name: Init()
// Desc: Initialize all variables.
//--------------------------------------------------------------------------------------
SessionSrv::SessionSrv()
{
	m_IsCreated = m_IsStarted = FALSE;
	// Initialize variables
#if defined(_XBOX) || defined(_XENON)
    m_hSession = INVALID_HANDLE_VALUE;
#endif
}

SessionSrv::SessionSrv(MessageMgr* msgMgr)
{
	m_IsCreated = m_IsStarted = FALSE;
	// Initialize variables
#if defined(_XBOX) || defined(_XENON)
	m_hSession = INVALID_HANDLE_VALUE;
#endif

	if (msgMgr)
	{
		msgMgr->Register(EMessage_OnlineTaskDone,this);
	}
}

//--------------------------------------------------------------------------------------
// Name: BeginSession()
// Desc: Create a new(GSOPType) session for writing stats
//--------------------------------------------------------------------------------------
GS_BOOL SessionSrv::BeginSession()
{
#if defined(_XBOX) || defined(_XENON)
	if (m_hSession != INVALID_HANDLE_VALUE)
		return FALSE;

    GS_DWORD dwUser = SignIn::GetSignedInUser();
	if (-1 == dwUser)
		return FALSE;

    // Set the context to standard (not ranked)
    XUserSetContext( dwUser, X_CONTEXT_GAME_TYPE, X_CONTEXT_GAME_TYPE_STANDARD );

    // Create a (non-matchmaking) session
	CTaskID id = 0;
    GS_DWORD ret = XSessionCreate(
        XSESSION_CREATE_USES_STATS,
        dwUser,
        m_dwPublicSlots,
        m_dwPrivateSlots,
        &m_SessionNonce,
        &m_SessionInfo,
		Master::G()->GetTaskMgr()->AddTask(EGSTaskType_SessionCreate,this,&id),
        &m_hSession );
	Master::G()->GetTaskMgr()->StartTask(id,ret);

    if( ret != ERROR_IO_PENDING )
    {
		Master::G()->Log("XSessionCreate failed with error %d", ret);
        return FALSE;
    }

#endif
	return TRUE;
}


//--------------------------------------------------------------------------------------
// Name: JoinSession()
// Desc: Join the session
//--------------------------------------------------------------------------------------
GS_BOOL SessionSrv::JoinSession()
{
#if defined(_XBOX) || defined(_XENON)
 	if (m_hSession == INVALID_HANDLE_VALUE)
		return FALSE;

    // Join the session
	CTaskID id = 0;
    GS_DWORD ret = XSessionJoinRemote(
        m_hSession,
		SignIn::GetUserNum(),
		SignIn::GetXUIDArray(),
		SignIn::GetIsPrivateArray(),
        Master::G()->GetTaskMgr()->AddTask(EGSTaskType_SessionJoin,this,&id) );
	Master::G()->GetTaskMgr()->StartTask(id,ret);

    if( ret != ERROR_IO_PENDING )
    {
        Master::G()->Log( "XSessionJoinRemote failed with error %d", ret );
        
		return FALSE;
    }

#endif
    return TRUE;
}


//--------------------------------------------------------------------------------------
// Name: StartSession()
// Desc: Start the session
//--------------------------------------------------------------------------------------
GS_BOOL SessionSrv::StartSession()
{
#if defined(_XBOX) || defined(_XENON)
 	if (m_hSession == INVALID_HANDLE_VALUE)
		return FALSE;

   // Start the session
	CTaskID id = 0;
    GS_DWORD ret = XSessionStart(
        m_hSession,
        0,
        Master::G()->GetTaskMgr()->AddTask(EGSTaskType_SessionStart,this,&id) );
	Master::G()->GetTaskMgr()->StartTask(id,ret);

    if( ret != ERROR_IO_PENDING )
    {
        Master::G()->Log( "XSessionStart failed with error %d", ret );
        
		return FALSE;
    }

    m_iLastWriteStatsInSession = 0;

#endif
    return TRUE;
}

// ======================================================== 
// Record last write stats time
// ======================================================== 
#define WRITE_STATS_INTERVAL 300000 // 5m
GS_BOOL SessionSrv::CanWriteStats()
{
#if defined(_XBOX) || defined(_XENON)
    if (0 == m_iLastWriteStatsInSession || GetTickCount() - m_iLastWriteStatsInSession > WRITE_STATS_INTERVAL)
    {
        m_iLastWriteStatsInSession = GetTickCount();
        return TRUE;
    }

    return FALSE;
#endif
}


//--------------------------------------------------------------------------------------
// Name: EndSession()
// Desc: End the current session (and implicitly write stats) 
//--------------------------------------------------------------------------------------
GS_BOOL SessionSrv::EndSession()
{
    // Master::G()->GetStatsSrv()->Finalize();

#if defined(_XBOX) || defined(_XENON)
    // Don't end the session if it was never created
    if( m_hSession == INVALID_HANDLE_VALUE )
    {
        return FALSE;
    }

    // End the session
	CTaskID id = 0;
    GS_DWORD ret = XSessionEnd(
        m_hSession,
        Master::G()->GetTaskMgr()->AddTask(EGSTaskType_SessionEnd,this,&id) );
	Master::G()->GetTaskMgr()->StartTask(id,ret);

    if( ret != ERROR_IO_PENDING )
    {
        Master::G()->Log( "XSessionEnd failed with error %d", ret );
        
		return FALSE;
    }
#endif
	return TRUE;
}


//--------------------------------------------------------------------------------------
// Name: LeaveSession()
// Desc: Remove local users from the session preparatory to deleting it
//--------------------------------------------------------------------------------------
GS_BOOL SessionSrv::LeaveSession()
{
#if defined(_XBOX) || defined(_XENON)
    // Don't leave the session if it was never created
    if( m_hSession == INVALID_HANDLE_VALUE )
    {
        return FALSE;
    }

    // End the session
	CTaskID id = 0;
    GS_DWORD ret = XSessionLeaveRemote(
        m_hSession,
		SignIn::GetUserNum(),
		SignIn::GetXUIDArray(),
        Master::G()->GetTaskMgr()->AddTask(EGSTaskType_SessionLeave,this,&id) );
	Master::G()->GetTaskMgr()->StartTask(id,ret);

    if( ret != ERROR_IO_PENDING )
    {
        Master::G()->Log( "XSessionLeaveRemote failed with error %d", ret );
		return FALSE;
    }
#endif
	return TRUE;
}


//--------------------------------------------------------------------------------------
// Name: DeleteSession()
// Desc: Remove the session
//--------------------------------------------------------------------------------------
GS_BOOL SessionSrv::DeleteSession()
{
#if defined(_XBOX) || defined(_XENON)
    // Don't delete the session if it was never created
    if( m_hSession == INVALID_HANDLE_VALUE )
    {
        return FALSE;
    }

	// End the session
	CTaskID id = 0;
    GS_DWORD ret = XSessionDelete(
        m_hSession,
        Master::G()->GetTaskMgr()->AddTask(EGSTaskType_SessionDelete,this,&id) );
	Master::G()->GetTaskMgr()->StartTask(id,ret);

    if( ret != ERROR_IO_PENDING )
    {
        Master::G()->Log( "XSessionDelete failed with error %d", ret );
		return FALSE;
    }
#endif
	return TRUE;
}

void SessionSrv::MessageResponse(Message* message)
{
	CTaskID task_id = *(CTaskID*)message->ReadPayload(0);
	GS_TaskType taskType = (GS_TaskType)(*(GS_INT*)message->ReadPayload(1));
	GS_DWORD taskResult = *(GS_DWORD*)message->ReadPayload(2);
	GS_DWORD taskDetailedResult = *(GS_DWORD*)message->ReadPayload(3);
#if defined(_XBOX) || defined(_XENON)
	GS_DWORD taskExtendedResult = *(GS_DWORD*)message->ReadPayload(4);
#endif

#if defined(_XBOX) || defined(_XENON)
	if (ERROR_SUCCESS == taskResult)
#elif defined(_PS3)
	if (0 == taskResult)
#endif
	{
		switch (taskType)
		{
		case EGSTaskType_SessionCreate:
		case EGSTaskType_SessionJoin:
			m_IsCreated = TRUE;
			break;
		case EGSTaskType_SessionStart:
			m_IsStarted = TRUE;
			break;
		case EGSTaskType_SessionEnd:
			m_IsStarted = FALSE;
			break;
		case EGSTaskType_SessionDelete:
		case EGSTaskType_SessionLeave:
			m_IsCreated = FALSE;
#if defined(_XBOX) || defined(_XENON)
			m_hSession = INVALID_HANDLE_VALUE;
#endif
			break;
		default:
			break;
		}
	}

	// Message to Interface
	Message* msg = Message::Create(EMessage_CallBackInterface);
	if (msg)
	{
		msg->AddPayload(taskType);
#if defined(_XBOX) || defined(_XENON)
		GS_BOOL result = (ERROR_SUCCESS == taskResult) ? TRUE : FALSE;
#elif defined(_PS3)
		GS_BOOL result = (0 == taskResult) ? TRUE : FALSE;
#endif
		msg->AddPayload(result);

		msg->AddTarget(Master::G()->GetInterfaceMgr());

		Master::G()->GetMessageMgr()->Send(msg);
	}

}

} // namespace GameService
