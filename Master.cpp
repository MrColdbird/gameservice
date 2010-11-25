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
#include "InterfaceMgr.h"
#include "Stats.h"
#include "Achievement.h"
#include "Friends.h"
#include "SysMsgBox.h"
#include "LogFile.h"
#include "OSDK/Tracking/TrackingManager.h"
#include <RendezVous.h>

#if defined(_PS3)

#ifdef INGAMEBROWSING
#include "InGameBrowsing.h"
#else
#include "StoreBrowsing.h"
#endif

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
        m_pSingle = new(GSOPType) Master();
        return m_pSingle;
    }

	if (!m_pSingle)
		FatalError( "No GameService Master instanced! \n" );

	return m_pSingle;
}

Master::Master()
: m_pMessageMgr(NULL), m_pTaskMgr(NULL), m_pInterfaceMgr(NULL), m_pSessionSrv(NULL), m_pSysMsgBoxManager(NULL),
    m_pAchievementSrv(NULL), m_pTrackingMgr(NULL), m_pStatsSrv(NULL), m_pFriendsSrv(NULL), m_pLogFile(NULL),
    m_iFreeSpaceAvailable(0)
#if defined(_PS3)
    , m_bForceQuit(FALSE)
#ifdef INGAMEBROWSING
,m_pInGameBrowsingSrv(NULL)
#else
,m_pStoreBrowsingSrv(NULL)
#endif
#endif
{
}

GS_VOID Master::Initialize(GS_BOOL requireOnlineUser, GS_INT signInPanes, GS_INT achieveCount, GS_INT versionId, GS_BOOL isTrial, GS_BOOL isDumpLog
#if defined(_PS3)
                       , GS_INT freeSpaceAvail
#endif
                           )
{
    if (isDumpLog)
        m_pLogFile = new(GSOPType) LogFile();

    Log("GameService Master Waiting for initialization.");

    m_iAchieveCount = achieveCount;
    m_iVersionID = versionId;
    m_bIsTrial = isTrial;
#if defined(_PS3)
    m_iFreeSpaceAvailable = freeSpaceAvail;
#endif

    m_bInitialized = FALSE;
    // Initialize autologin
	m_pSysMsgBoxManager = new(GSOPType) SysMsgBoxManager();
    SignIn::Initialize( 1, 4, requireOnlineUser, signInPanes );
}

GS_VOID Master::InitServices()
{
    if (m_bInitialized)
    {
        ResetServices();
        return;
    }

	m_pMessageMgr = new(GSOPType) MessageMgr();
	m_pTaskMgr = new(GSOPType) TaskMgr();
	m_pInterfaceMgr = new(GSOPType) InterfaceMgr(m_pMessageMgr);

	// services:
	m_pSessionSrv = new(GSOPType) SessionSrv(m_pMessageMgr);
    m_pStatsSrv = new(GSOPType) StatsSrv(m_pMessageMgr);

#if defined(_PS3)
    if (m_bIsTrial)
    {
       m_pAchievementSrv = NULL;
    }
    else
#endif
    {
        m_pAchievementSrv = new(GSOPType) AchievementSrv(m_pMessageMgr, m_iAchieveCount
#if defined(_PS3)
                                                         , m_iFreeSpaceAvailable
#endif
                                                         );
    }

#if defined(_PS3)
    // Force Quit by SCE, dont do anything!
    if (m_bForceQuit)
    {
        return;
    }
#endif

    m_pFriendsSrv = new(GSOPType) FriendsSrv();
    m_pFriendsSrv->RetrieveFriendsList(SignIn::GetActiveUserIndex());

    // Tracking:
#ifdef GAMESERVIE_TRACKING_ENABLE
    m_pTrackingMgr = new(GSOPType) TrackingManager();
    m_pTrackingMgr->SetVersionID(m_iVersionID);
    m_pTrackingMgr->SetLicenseType(!m_bIsTrial);
    m_pTrackingMgr->Initialize();
#endif

#if defined(_PS3)
#ifdef INGAMEBROWSING
	m_pInGameBrowsingSrv = new(GSOPType) InGameBrowsing();
#else
	m_pStoreBrowsingSrv = new(GSOPType) StoreBrowsing();
#endif
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
}

GS_VOID Master::ResetServices()
{
    Finalize();

    // ignore Session service for now


    if (m_pStatsSrv) 
        m_pStatsSrv->Initialize();

    if (m_pAchievementSrv)
        m_pAchievementSrv->Initialize();

    if (m_pFriendsSrv)
        m_pFriendsSrv->RetrieveFriendsList(SignIn::GetActiveUserIndex());

    // Tracking:
#ifdef GAMESERVIE_TRACKING_ENABLE
    if (m_pTrackingMgr) 
        m_pTrackingMgr->Initialize();
#endif

}

GS_VOID Master::Destroy()
{
    DeleteThis<Master>(this);
}

Master::~Master()
{
	if (m_pMessageMgr)
	{
		Delete<MessageMgr>(m_pMessageMgr);
	}
	if (m_pTaskMgr)
	{
		Delete<TaskMgr>(m_pTaskMgr);
	}
	if (m_pInterfaceMgr)
	{
		Delete<InterfaceMgr>(m_pInterfaceMgr);
	}
	if (m_pStatsSrv)
	{
		Delete<StatsSrv>(m_pStatsSrv);
	}
	if (m_pSessionSrv)
	{
		Delete<SessionSrv>(m_pSessionSrv);
	}
	if (m_pAchievementSrv)
	{
		Delete<AchievementSrv>(m_pAchievementSrv);
	}
#ifdef GAMESERVIE_TRACKING_ENABLE
    if (m_pTrackingMgr)
    {
        Delete<TrackingManager>(m_pTrackingMgr);
    }
#endif
    if (m_pSysMsgBoxManager)
    {
        Delete<SysMsgBoxManager>(m_pSysMsgBoxManager);
    }
    if (m_pFriendsSrv)
    {
        Delete<FriendsSrv>(m_pFriendsSrv);
    }

#if defined(_PS3)
#ifdef INGAMEBROWSING
	if(m_pInGameBrowsingSrv)
	{
		Delete<InGameBrowsing>(m_pInGameBrowsingSrv);
	}
#else
	if(m_pStoreBrowsingSrv)
	{
		Delete<StoreBrowsing>(m_pStoreBrowsingSrv);
	}
#endif
    SignIn::TermNP();
    SignIn::TermNet();
#endif

    if (m_pLogFile)
    {
        Delete<LogFile>(m_pLogFile);
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
#if defined(_PS3) && defined(INGAMEBROWSING)
	if (m_pInGameBrowsingSrv && m_pInGameBrowsingSrv->IsInited())
        m_pInGameBrowsingSrv->Update();
#endif
    if (m_pSysMsgBoxManager)
        m_pSysMsgBoxManager->Update();

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
        m_pLogFile->Write(str);

        va_end( pArgList );
    }
}

} // namespace GameService
