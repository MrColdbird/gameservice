// ======================================================================================
// File         : Master.h
// Author       : Li Chen 
// Last Change  : 07/29/2010 | 15:47:47 PM | Thursday,July
// Description  : 
// ======================================================================================

#pragma once
#ifndef GAMESERVICE_SINGLETON_H
#define GAMESERVICE_SINGLETON_H
//#define INGAMEBROWSING
#define SCEA_SUBMISSION 0
#define SCEE_SUBMISSION 1

namespace GameService
{

class TaskMgr;
class MessageMgr;
class InterfaceMgr;
class SessionSrv;
class StatsSrv;
class AchievementSrv;
class FriendsSrv;
class TrackingManager;
#if defined(_PS3)
#ifdef INGAMEBROWSING
class InGameBrowsing;
#else
class StoreBrowsing;
#endif
#endif
class SysMsgBoxManager;
class LogFile;
#if SCEA_SUBMISSION
#define NP_GUI_PRODUCT_ID "UP0001-NPUB30394_00-KEY0000000000001"
#elif SCEE_SUBMISSION
#define NP_GUI_PRODUCT_ID "EP0001-NPEB00435_00-KEY0000000000001"
#else
#define NP_GUI_PRODUCT_ID "UP0001-NPXX00865_00-KEY0000000000001"
#endif

class Master
{
public:
    static Master* G();

    GS_VOID Initialize(GS_BOOL requireOnlineUser, GS_INT signInPanes=1, GS_INT achieveCount=6, GS_INT versionId=0, GS_BOOL isTrial=0, GS_BOOL isDumpLog=0
#if defined(_PS3)
                       , GS_INT freeSpaceAvail=0
#endif
                       );
    GS_VOID Finalize();
    GS_VOID Destroy();

    GS_VOID InitServices();
    GS_VOID ResetServices();

    GS_VOID Update();
    virtual ~Master();

	SessionSrv*		GetSessionSrv()		
    { 
#if defined(_PS3)
        if (m_bForceQuit) return NULL;
#endif
        return m_pSessionSrv; 
    }
	StatsSrv*		GetStatsSrv()		
    { 
#if defined(_PS3)
        if (m_bForceQuit) return NULL;
#endif
        return m_pStatsSrv; 
    }
	AchievementSrv*	GetAchievementSrv()	
    { 
#if defined(_PS3)
        if (m_bForceQuit) return NULL;
#endif
        return m_pAchievementSrv; 
    }
    FriendsSrv*     GetFriendsSrv()     
    { 
#if defined(_PS3)
        if (m_bForceQuit) return NULL;
#endif
        return m_pFriendsSrv; 
    }
#if defined(_PS3)
#ifdef INGAMEBROWSING
	InGameBrowsing*  GetInGameBrowsingSrv() 
    { 
#if defined(_PS3)
        if (m_bForceQuit) return NULL;
#endif
        return m_pInGameBrowsingSrv; 
    }
#else
	StoreBrowsing*  GetStoreBrowsingSrv() 
    { 
#if defined(_PS3)
        if (m_bForceQuit) return NULL;
#endif
        return m_pStoreBrowsingSrv; 
    }
#endif
#endif
    // Tracking
    GS_BOOL            SendTrackingTag(GS_CHAR* cName, GS_CHAR* cAttribute);
#if defined(_PS3)
    GS_VOID            SetTrackingNpTicketSize(GS_INT size);
    GS_VOID             SetForceQuit() { m_bForceQuit = TRUE; }
    GS_BOOL             IsForceQuit() { return m_bForceQuit; }
#endif
    SysMsgBoxManager* GetSysMsgBoxManager() { return m_pSysMsgBoxManager; }

	// for internal use
	TaskMgr*		GetTaskMgr()		{ return m_pTaskMgr; }
	MessageMgr*		GetMessageMgr()		{ return m_pMessageMgr; }
	InterfaceMgr*	GetInterfaceMgr()	{ return m_pInterfaceMgr; }

    GS_VOID         Log(const GS_CHAR* strFormat, ...);

    // urgly interface:
    GS_VOID         SetFreeSpaceAvailable(GS_INT space)
    {
        m_iFreeSpaceAvailable = space;
    }

private:
    static GS_BOOL		m_bInstanceFlag;
    static Master*	m_pSingle;

    GS_INT                 m_iAchieveCount;
    GS_INT                 m_iVersionID;
    GS_BOOL                m_bIsTrial;
    GS_INT                 m_bInitialized;

	MessageMgr*		    m_pMessageMgr;
	TaskMgr*		    m_pTaskMgr;
	SessionSrv*		    m_pSessionSrv;
	InterfaceMgr*	    m_pInterfaceMgr;
	StatsSrv*		    m_pStatsSrv;
	AchievementSrv*	    m_pAchievementSrv;
    FriendsSrv*         m_pFriendsSrv;

    TrackingManager*    m_pTrackingMgr;
    SysMsgBoxManager*   m_pSysMsgBoxManager;
    LogFile*            m_pLogFile;

    GS_INT              m_iFreeSpaceAvailable;

#if defined(_PS3)
#ifdef INGAMEBROWSING
	InGameBrowsing*		m_pInGameBrowsingSrv;
#else
	StoreBrowsing*		m_pStoreBrowsingSrv;
#endif
    GS_BOOL             m_bForceQuit;
#endif

    Master(); //private constructor
};

} // namespace GameService

#endif // GAMESERVICE_SINGLETON_H
