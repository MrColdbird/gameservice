// ======================================================================================
// File         : ContentManager.cpp
// Author       : Li Chen 
// Last Change  : 01/07/2011 | 14:21:43 PM | Friday,January
// Description  : 
// ======================================================================================

#include "stdafx.h"
#include "SignIn.h"
#include "Master.h"
#include "Task.h"
#include "ContentManager.h"

namespace GameService
{

#define MAX_ENUMERATION_RESULTS 10

ContentManager::ContentManager(MessageMgr* msgMgr)
: m_hEnumeration( INVALID_HANDLE_VALUE ),
m_dwBufferSize( 0 ),
m_pBuffer( NULL )
{
	if (msgMgr)
	{
		msgMgr->Register(EMessage_SignInChanged,this);
		msgMgr->Register(EMessage_OnlineTaskDone,this);
	}
}

// ======================================================== 
// Initialize
// ======================================================== 
GS_BOOL ContentManager::Initialize()
{
    if (!SignIn::AreUsersSignedIn())
        return FALSE;

    // Enumerate at most MAX_ENUMERATION_RESULTS items
    GS_DWORD dwError = XContentCreateEnumerator( SignIn::GetActiveUserIndex()
                                            , XCONTENTDEVICE_ANY
                                            , XCONTENTTYPE_MARKETPLACE
                                            , NULL
                                            , MAX_ENUMERATION_RESULTS
                                            , &m_dwBufferSize
                                            , &m_hEnumeration );

    if ( FAILED( HRESULT_FROM_WIN32(dwError) ) )
    {
        Cleanup();

        return FALSE;
    }

    m_pBuffer = GS_NEW BYTE[m_dwBufferSize];
    if ( !m_pBuffer )
    {
        Master::G()->Log("ContentManager::Initialize - Out of Memory!");
    }

    return TRUE;
}

// ======================================================== 
// Finalize
// ======================================================== 
GS_VOID ContentManager::Finalize()
{
    Cleanup();
}

// ======================================================== 
// Enumerate
// ======================================================== 
GS_BOOL ContentManager::Enumerate()
{
	CTaskID id = 0;
    GS_DWORD dwStatus = XEnumerate( m_hEnumeration, m_pBuffer, m_dwBufferSize, NULL,
                    Master::G()->GetTaskMgr()->AddTask(EGSTaskType_ContentMgrEnumeration,this,&id) );
	Master::G()->GetTaskMgr()->StartTask(id,dwStatus);

    if( dwStatus != ERROR_IO_PENDING )
    {
		Master::G()->Log("ContentManager::Enumerate() failed with error %d", dwStatus);
        return FALSE;
    }

    return TRUE;
}

// ======================================================== 
// Cleanup
// ======================================================== 
GS_VOID ContentManager::Cleanup()
{
    // Free the handle
    if ( m_hEnumeration != INVALID_HANDLE_VALUE )
    {
        XCloseHandle( m_hEnumeration );
        m_hEnumeration = INVALID_HANDLE_VALUE;
    }

    // Delete the buffer
    if (m_pBuffer)
    {
        delete [] m_pBuffer;
        m_pBuffer = NULL;
    }
    m_dwBufferSize = 0;

     // Clear out the collection prior to repopulating with more enumeration results
    m_aContentData.clear();

}

//--------------------------------------------------------------------------------------
// Name: Update
// Desc: Checks to see if the enumerator is ready and if so updates the enumerator
//--------------------------------------------------------------------------------------
GS_VOID
ContentManager::Update()
{
}


//--------------------------------------------------------------------------------------
// Name: OnContentInstalled
// Desc: This method is called in response to XN_LIVE_CONTENT_INSTALLED
//       Kicks off another enumeration to discover new content packages
//--------------------------------------------------------------------------------------
GS_BOOL
ContentManager::OnContentInstalled()
{
    Cleanup();

	Initialize();

    return Enumerate();
}

GS_VOID ContentManager::MessageResponse(Message* message)
{
    if (EMessage_SignInChanged == message->GetMessageID())
    {
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
        case EGSTaskType_ContentMgrEnumeration:
            {
                PXCONTENT_DATA pTempContentData = ( PXCONTENT_DATA )m_pBuffer;
                for ( GS_DWORD dw = 0; dw < taskDetailedResult; ++dw )
                {
                    m_aContentData.push_back( pTempContentData[dw] );
                }
            }
            break;
        }
    }
}

} // namespace
