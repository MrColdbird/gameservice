// ======================================================================================
// File         : Master.h
// Author       : Li Chen 
// Last Change  : 07/29/2010 | 15:47:47 PM | Thursday,July
// Description  : 
// ======================================================================================

#pragma once
#ifndef GAMESERVICE_SINGLETON_H
#define GAMESERVICE_SINGLETON_H


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
class InGameMarketplace;
class SysMsgBoxManager;
class LogFile;
class HttpSrv;


class Master
{
public:
    static Master* G();

    GS_VOID Initialize(GS_BOOL requireOnlineUser, GS_INT signInPanes=1, GS_INT achieveCount=6, GS_INT versionId=0, GS_BOOL isTrial=0, GS_BOOL isDumpLog=0
#if defined(_PS3)
                       , GS_INT freeSpaceAvail=0, GS_BOOL enableInGameMarketplace=0
#endif
                       );
    GS_VOID Finalize();
    GS_VOID Destroy();

    GS_VOID InitServices();

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

    InGameMarketplace* GetInGameMarketplaceSrv()
    {
        return m_pInGameMarketplace;
    }

    HttpSrv* GetHttpSrv()
    {
        return m_pHttpSrv;
    }

    GS_BOOL IsTrialVersion()
    {
        return m_bIsTrial;
    }

#if defined(_PS3)
    GS_BOOL IsEnableInGameMarketplace()
    {
        return m_bEnableInGameMarketplace;
    }
#endif

    // Tracking
    GS_BOOL            SendTrackingTag(GS_CHAR* cName, GS_CHAR* cAttribute);
#if defined(_PS3)
    GS_VOID            SetTrackingNpTicketSize(GS_INT size);
    GS_VOID             SetForceQuit() { m_bForceQuit = TRUE; }
    GS_BOOL             IsForceQuit() { return m_bForceQuit; }
    // urgly interface:
    GS_VOID         SetFreeSpaceAvailable(GS_INT space)
    {
        m_iFreeSpaceAvailable = space;
    }
#endif

    SysMsgBoxManager* GetSysMsgBoxManager() { return m_pSysMsgBoxManager; }

	// for internal use
	TaskMgr*		GetTaskMgr()		{ return m_pTaskMgr; }
	MessageMgr*		GetMessageMgr()		{ return m_pMessageMgr; }
	InterfaceMgr*	GetInterfaceMgr()	{ return m_pInterfaceMgr; }

    GS_VOID         Log(const GS_CHAR* strFormat, ...);
    GS_BOOL         WriteLocalFile(GS_INT fileIndex, GS_BYTE* data, GS_DWORD size);
    const GS_CHAR*  GetLocalFileName(GS_INT fileIndex);


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
	InGameMarketplace*	m_pInGameMarketplace;
    HttpSrv*            m_pHttpSrv;

    TrackingManager*    m_pTrackingMgr;
    SysMsgBoxManager*   m_pSysMsgBoxManager;
    LogFile*            m_pLogFile;

    GS_BOOL             m_bIsDumpLog;

#if defined(_PS3)
    GS_INT              m_iFreeSpaceAvailable;
    GS_BOOL             m_bForceQuit;
    GS_BOOL             m_bEnableInGameMarketplace;
#endif

    Master(); //private constructor
};

} // namespace GameService

#endif // GAMESERVICE_SINGLETON_H
