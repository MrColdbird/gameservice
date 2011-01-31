// ======================================================================================
// File         : Master.cpp
// Author       : Li Chen 
// Last Change  : 11/17/2010 | 11:36:26 AM | Wednesday,November
// Description  : 
// ======================================================================================

#include "stdafx.h"
#include "SignIn.h"
#include "Task.h"
#include "Session.h"
#include "Master.h"
#include "Interface.h"
#include "InterfaceMgr.h"
#include "Stats.h"
#include "Achievement.h"
#include "Friends.h"
#if defined(_XBOX) || defined(_XENON) || defined(_PS3)
#include "InGameMarketplace.h"
#endif
#include "SysMsgBox.h"
#include "LogFile.h"
#include "HttpSrv.h"
#ifdef GAMESERVIE_TRACKING_ENABLE
#include "OSDK/Tracking/TrackingManager.h"
#include <RendezVous.h>
#endif

#if defined(_WINDOWS)
#include <stdarg.h>
#endif

namespace GameService
{

GS_BOOL Master::m_bInstanceFlag = FALSE;
Master* Master::m_pSingle = NULL;

Master* Master::G()
{
    if(!m_bInstanceFlag)
    {
        m_bInstanceFlag = TRUE;
        m_pSingle = GS_NEW Master();
        return m_pSingle;
    }

	if (!m_pSingle)
        GS_Assert(0);
        // FatalError( "No GameService Master instanced! \n" );

	return m_pSingle;
}

Master::Master()
: m_pMessageMgr(NULL)
, m_pTaskMgr(NULL)
, m_pSessionSrv(NULL)
, m_pInterfaceMgr(NULL)
, m_pStatsSrv(NULL)
, m_pAchievementSrv(NULL)
, m_pFriendsSrv(NULL)
, m_pInGameMarketplace(NULL)
, m_pTrackingMgr(NULL)
, m_pSysMsgBoxManager(NULL)
, m_pHttpSrv(NULL)
, m_pLogFile(NULL)
, m_iAchieveCount(0)
, m_iVersionID(0)
, m_bIsTrial(FALSE)
, m_bInitialized(FALSE)
#if defined(_PS3)
, m_iFreeSpaceAvailable(0)
, m_bForceQuit(FALSE)
, m_bEnableInGameMarketplace(0)
#endif
{
}

GS_VOID Master::Initialize(GS_BOOL requireOnlineUser, GS_INT signInPanes, GS_INT achieveCount, GS_INT versionId, GS_BOOL isTrial, GS_BOOL isDumpLog
#if defined(_PS3)
                       , GS_INT freeSpaceAvail, GS_BOOL enableInGameMarketplace
#endif
                           )
{
	// engine specific code
    m_bIsDumpLog = isDumpLog;
    m_pLogFile = GS_NEW LogFile(m_bIsDumpLog);

    Log("GameService Master Waiting for initialization.");

    m_iAchieveCount = achieveCount;
    m_iVersionID = versionId;
    m_bIsTrial = isTrial;
#if defined(_PS3)
    m_iFreeSpaceAvailable = freeSpaceAvail;
    m_bEnableInGameMarketplace = enableInGameMarketplace;
#endif

    m_bInitialized = FALSE;

	if (m_pSysMsgBoxManager == NULL)
	{
		m_pSysMsgBoxManager = GS_NEW SysMsgBoxManager();
	}

	
	if (m_pMessageMgr == NULL)
	{
		m_pMessageMgr = GS_NEW MessageMgr();
	}

	if (m_pTaskMgr == NULL)
	{
		m_pTaskMgr = GS_NEW TaskMgr();
	}

	if (m_pInterfaceMgr == NULL)
	{
		m_pInterfaceMgr = GS_NEW InterfaceMgr(m_pMessageMgr);
	}

	// services:
	if (m_pSessionSrv == NULL)
	{
		m_pSessionSrv = GS_NEW SessionSrv(m_pMessageMgr);
	}

	if (m_pStatsSrv == NULL)
	{
		m_pStatsSrv = GS_NEW StatsSrv(m_pMessageMgr);
	}

	if (m_pAchievementSrv == NULL)
	{
		m_pAchievementSrv = GS_NEW AchievementSrv(m_pMessageMgr, m_iAchieveCount
#if defined(_PS3)
                                                         , m_iFreeSpaceAvailable
#endif
                                                         );
	}

	if (m_pFriendsSrv == NULL)
	{
		m_pFriendsSrv = GS_NEW FriendsSrv();
	}

    // Tracking:
#ifdef GAMESERVIE_TRACKING_ENABLE
	if (m_pTrackingMgr == NULL)
	{
		m_pTrackingMgr = GS_NEW TrackingManager();
	}
#endif

    // InGameMarketplace
#if defined(_XBOX) || defined(_XENON) || defined(_PS3)
#if defined(_PS3)
    if (m_bEnableInGameMarketplace)
#endif
	{
		if (m_pInGameMarketplace == NULL)
		{
			m_pInGameMarketplace = GS_NEW InGameMarketplace(m_pMessageMgr);
		}
	}
#endif

    if (NULL == m_pHttpSrv)
		m_pHttpSrv = GS_NEW HttpSrv();


    // Initialize autologin
    SignIn::Initialize( 1, 4, requireOnlineUser, signInPanes );
}

GS_VOID Master::InitServices()
{
    if (m_bInitialized)
    {
        Finalize();
    }

	if (m_pStatsSrv)
	{
		m_pStatsSrv->Initialize();
	}

	if (m_pAchievementSrv)
    {
		m_pAchievementSrv->Initialize();
    } 

	if (m_pFriendsSrv)
	{
		m_pFriendsSrv->Initialize();
	}

#if defined(_XBOX) || defined(_XENON) || defined(_PS3)
    if (m_pInGameMarketplace)
	{
		m_pInGameMarketplace->Initialize();
	}
#endif

    if (m_pHttpSrv)
        m_pHttpSrv->Initialize();

    // Tracking:
#ifdef GAMESERVIE_TRACKING_ENABLE
	if (m_pTrackingMgr)
	{   
		m_pTrackingMgr->SetVersionID(m_iVersionID);
		m_pTrackingMgr->SetLicenseType(!m_bIsTrial);
		m_pTrackingMgr->Initialize();
	}
#endif

	m_bInitialized = TRUE;
    Log("GameService Master Initialized.");

}

GS_VOID Master::Finalize()
{
    // ignore Session service for now
	if (m_pStatsSrv) 
		m_pStatsSrv->Finalize();

	if (m_pAchievementSrv)
		m_pAchievementSrv->Finalize();

	if (m_pFriendsSrv)
		m_pFriendsSrv->Finalize();

#if defined(_XBOX) || defined(_XENON) || defined(_PS3)
    if (m_pInGameMarketplace)
        m_pInGameMarketplace->Finalize();
#endif

    if (m_pHttpSrv)
        m_pHttpSrv->Finalize();

    // Tracking:
#ifdef GAMESERVIE_TRACKING_ENABLE
	if (m_pTrackingMgr)
		m_pTrackingMgr->Finalize();
#endif

	m_bInitialized = FALSE;
}


GS_VOID Master::Destroy()
{
	Finalize();
    GS_DELETE this;
}

Master::~Master()
{
	if (m_pMessageMgr)
	{
		GS_DELETE m_pMessageMgr;
		m_pMessageMgr = NULL;
	}
	if (m_pTaskMgr)
	{
		GS_DELETE m_pTaskMgr;
		m_pTaskMgr = NULL;
	}
	if (m_pInterfaceMgr)
	{
		GS_DELETE m_pInterfaceMgr;
		m_pInterfaceMgr = NULL;
	}
	if (m_pStatsSrv)
	{
		GS_DELETE m_pStatsSrv;
		m_pStatsSrv = NULL;
	}
	if (m_pSessionSrv)
	{
		GS_DELETE m_pSessionSrv;
		m_pSessionSrv = NULL;
	}
	if (m_pAchievementSrv)
	{
		GS_DELETE m_pAchievementSrv;
		m_pAchievementSrv = NULL;
	}
	if (m_pFriendsSrv)
	{
		GS_DELETE m_pFriendsSrv;
		m_pFriendsSrv = NULL;
	}

#ifdef GAMESERVIE_TRACKING_ENABLE
    if (m_pTrackingMgr)
    {
        GS_DELETE m_pTrackingMgr;
		m_pTrackingMgr = NULL;
    }
#endif
    if (m_pSysMsgBoxManager)
    {
        GS_DELETE m_pSysMsgBoxManager;
		m_pSysMsgBoxManager = NULL;
    }
    if (m_pFriendsSrv)
    {
        GS_DELETE m_pFriendsSrv;
		m_pFriendsSrv = NULL;
    }

#if defined(_XBOX) || defined(_XENON) || defined(_PS3)
    if (m_pInGameMarketplace)
    {
        GS_DELETE m_pInGameMarketplace;
        m_pInGameMarketplace = NULL;
    }
#endif

    if (m_pHttpSrv)
    {
        GS_DELETE m_pHttpSrv;
        m_pHttpSrv = NULL;
    }

#if defined(_PS3)
    SignIn::TermNP();
    SignIn::TermNet();
#endif

    if (m_pLogFile)
    {
        GS_DELETE m_pLogFile;
		m_pLogFile = NULL;
    }

 	m_bInstanceFlag = FALSE;
	m_pSingle = NULL;
}

GS_VOID Master::Update()
{
	SignIn::Update();

    if (m_pStatsSrv)
        m_pStatsSrv->Update();

#ifdef GAMESERVIE_TRACKING_ENABLE
    if (m_pTrackingMgr)
        m_pTrackingMgr->Update();
#endif

    if (m_pTaskMgr)
        m_pTaskMgr->UpdateAll();

    if (m_pSysMsgBoxManager)
        m_pSysMsgBoxManager->Update();

#if defined(_XBOX) || defined(_XENON) || defined(_PS3)
    if (m_pInGameMarketplace)
        m_pInGameMarketplace->Update();
#endif

}

GS_BOOL Master::SendTrackingTag(GS_CHAR* cName, GS_CHAR* cAttribute)
{
    if (!m_pTrackingMgr)
        return FALSE;

#ifdef GAMESERVIE_TRACKING_ENABLE
    return m_pTrackingMgr->SendTag(String(cName), String(cAttribute));
#else
	return FALSE;
#endif
}

#if defined(_PS3)
GS_VOID Master::SetTrackingNpTicketSize(GS_INT size)
{
#ifdef GAMESERVIE_TRACKING_ENABLE
    if (m_pTrackingMgr)
        m_pTrackingMgr->SetNpTicketSize(size);
#endif
}
#endif

GS_VOID Master::Log(const GS_CHAR* strFormat, ...)
{
    if (m_pLogFile)
    {
        va_list pArgList;
        va_start( pArgList, strFormat );

        GS_CHAR str[512];
#if defined(_XBOX) || defined(_XENON)
		_vsnprintf_s( str, _TRUNCATE, strFormat, pArgList );
#elif defined(_PS3)
		vsnprintf( str, 512, strFormat, pArgList );
#endif
        m_pLogFile->DumpLog(str);

        va_end( pArgList );
    }
}

GS_BOOL Master::WriteLocalFile(GS_INT fileIndex, GS_BYTE* data, GS_DWORD size)
{
    if (m_pLogFile)
    {
        return m_pLogFile->WriteLocalFile(fileIndex, data, size);
    }

    return FALSE;
}

const GS_CHAR* Master::GetLocalFileName(GS_INT fileIndex)
{
    if (m_pLogFile)
    {
        return m_pLogFile->GetFileName(fileIndex);
    }

    return NULL;
}

} // namespace GameService
