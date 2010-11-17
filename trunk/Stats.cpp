// ======================================================================================
// File         : Stats.cpp
// Author       : Li Chen 
// Last Change  : 07/29/2010 | 15:48:27 PM | Thursday,July
// Description  : 
// ======================================================================================

#include "stdafx.h"
#include "Customize/CusMemory.h"
#include "SignIn.h"
#include "stats.h"
#include "Task.h"
#include "Master.h"
#include "InterfaceMgr.h"
#include "Session.h"
#include "Friends.h"

namespace GameService
{

StatsSrv::StatsSrv(MessageMgr* msgMgr)
: m_pStats(NULL), m_iError(0)
#if defined(_PS3)
, m_ScoreCtxId(-1), m_TransactionId(-1), m_iRetrievedNum(0)
#endif
{
	if (!Initialize())
    {
        Finalize();
    }

	if (msgMgr)
	{
		msgMgr->Register(EMessage_OnlineTaskDone,this);
		msgMgr->Register(EMessage_SignInChanged,this);
	}
}

StatsSrv::~StatsSrv()
{
	Finalize();

	if (m_pStats)
	{
		Free(m_pStats);
		m_pStats = NULL;
	}

    for (GS_INT i=0;i<m_pLBInfoArray.Num();i++)
    {
        if (m_pLBInfoArray(i))
        {
            Delete<LeaderboardInfo>(m_pLBInfoArray(i));
        }
    }
    m_pLBInfoArray.Empty();

}

GS_BOOL StatsSrv::Initialize()
{
	m_bReadingLeaderboard = m_bWritingLeaderboard = FALSE;
	m_bUserStatsRetrieved = m_bIsReadingMyScore = FALSE;

#if defined(_PS3)
    int ret = -1;
    ret = sceNpScoreInit();
    if (ret < 0)
    {
        Master::G()->Log("StatsSrv - sceNpScoreInit failed: %d", ret);
        return FALSE;
    }

	ret = sceNpScoreCreateTitleCtx(
        SignIn::GetNpCommID(),
	    SignIn::GetNpCommPass(),
        &(SignIn::GetNpID()));
	if (ret < 0) 
    {
		Master::G()->Log("StatsSrv - sceNpScoreCreateTitleCtx() failed. ret = 0x%x", ret);
		return FALSE;
	}
	m_ScoreCtxId = ret;
#endif

	return TRUE;
}

void StatsSrv::Finalize()
{
#if defined(_PS3)
    if (m_ScoreCtxId >= 0)
    {
        sceNpScoreDestroyTitleCtx(m_ScoreCtxId);
    }

    sceNpScoreTerm();
#endif
}

//--------------------------------------------------------------------------------------
// Name: RetrieveLocalUserStats()
// Desc: Get stats for the local users so we can do dependent updates later
//--------------------------------------------------------------------------------------
GS_BOOL StatsSrv::RetrieveLocalUserStats(GS_BOOL bImmediately)
{
    if (m_bReadingLeaderboard)
        return FALSE;

#if defined(_XBOX) || defined(_XENON)
	if (-1 == SignIn::GetActiveUserIndex())
		return FALSE;

#elif defined(_PS3)
    GS_INT status = -1;
	sceNpManagerGetStatus(&status);
    if (status != SCE_NP_MANAGER_STATUS_ONLINE)
    {
        return FALSE;
    }
#endif 

    m_iFriendsIndexOffset = 0;

	if (m_pStats)
	{
		Free(m_pStats);
		m_pStats = NULL;
	}


#if defined(_XBOX) || defined(_XENON)
    // Retrieve the necessary buffer size
    GS_DWORD cbResults = 0;

    GS_DWORD ret = XUserReadStats(
        0,						// Current title ID
		1,//SignIn::GetUserNum(),	                    // Number of users
		&(SignIn::GetXUID(SignIn::GetActiveUserIndex())),	// XUIDs of users
        1,						// Number of stats specs
        m_LBDef.GetDefinition(),				// StatsSrv spec(s)
        &cbResults,				// Size of buffer
        NULL,					// Buffer
        NULL );					// Perform synchronously

    if( ret != ERROR_INSUFFICIENT_BUFFER )
    {
		Master::G()->Log("Failed to retrieve the stats buffer size with error %d", ret);
		return FALSE;
    }

    // Allocate the buffer
    m_pStats = new(GSOPType) GS_BYTE[ cbResults ];

    // Retrieve the stats
    if (bImmediately)
    {
        ret = XUserReadStats(
            0,                   // Current title ID
            1,//SignIn::GetUserNum(),	                    // Number of users
            &SignIn::GetXUID(SignIn::GetActiveUserIndex()),	// XUIDs of users
            1,                   // Number of stats specs
            m_LBDef.GetDefinition(),             // StatsSrv spec(s)
            &cbResults,          // Size of buffer
            (PXUSER_STATS_READ_RESULTS)m_pStats,            // Buffer
            NULL );     // Overlapped

		if (ret == ERROR_SUCCESS)
		{
			m_bUserStatsRetrieved = TRUE;
		}

		// Message to Interface
		Message* msg = Message::Create(EMessage_CallBackInterface);
		if (msg)
		{
			msg->AddPayload(EGSTaskType_StatsRetrieveLocal);
			bool result = (ERROR_SUCCESS == ret) ? true : false;
			msg->AddPayload(result);

			msg->AddTarget(Master::G()->GetInterfaceMgr());

			Master::G()->GetMessageMgr()->Send(msg);
		}
    }
    else
    {
        CTaskID id = 0;
        ret = XUserReadStats(
            0,                   // Current title ID
            1,//SignIn::GetUserNum(),	                    // Number of users
            &SignIn::GetXUID(SignIn::GetActiveUserIndex()),	// XUIDs of users
            1,                   // Number of stats specs
            m_LBDef.GetDefinition(),             // StatsSrv spec(s)
            &cbResults,          // Size of buffer
            (PXUSER_STATS_READ_RESULTS)m_pStats,            // Buffer
            Master::G()->GetTaskMgr()->AddTask(EGSTaskType_StatsRetrieveLocal,this,&id) );     // Overlapped
        Master::G()->GetTaskMgr()->StartTask(id,ret);

        if( ret != ERROR_IO_PENDING )
        {
            Master::G()->Log("XUserReadStats() failed with error %d", ret);
            return FALSE;
        }
    }
#elif defined(_PS3)
    // current dont support sync score functions
    bImmediately = FALSE;

    int ret = -1;
	ret = sceNpScoreCreateTransactionCtx(m_ScoreCtxId);
	if (ret < 0) 
    {
		Master::G()->Log("sceNpScoreCreateTransactionCtx() failed. ret = 0x%x", ret);
		return FALSE;
	}
	m_TransactionId = ret;
    
    if (bImmediately)
    {
        ret = sceNpScoreGetRankingByNpId(
            m_TransactionId,
            m_LBDef.GetBoardID(),
            &(SignIn::GetNpID()),
            sizeof(SceNpId) * 1,
            m_pMyPlayerRankData,
            sizeof(m_pMyPlayerRankData),
            NULL,
            0,
            NULL,
            0,
            1,
            &m_last_sort_date,
            &m_TotalRecord,
            NULL);
        if (ret < 0)
        {
            Master::G()->Log("sceNpScoreGetRankingByNpId() failed. ret = 0x%x", ret);
        }
		else
		{
			m_bUserStatsRetrieved = TRUE;
		}

        // destroy transaction context
        ret = sceNpScoreDestroyTransactionCtx(m_TransactionId);
        if (ret < 0) {
            Master::G()->Log("sceNpScoreDestroyTransactionCtx() failed. ret = 0x%x", ret);
        }
        m_TransactionId = -1;
        return TRUE;
    }
    else
    {
        // retrieve my own ranking
        ret = sceNpScoreGetRankingByNpIdAsync(
            m_TransactionId,
            m_LBDef.GetBoardID(),
            &(SignIn::GetNpID()),
            sizeof(SceNpId) * 1,
            m_pMyPlayerRankData,
            sizeof(m_pMyPlayerRankData),
            NULL,
            0,
            NULL,
            0,
            1,
            &m_last_sort_date,
            &m_TotalRecord,
            0, // internal asychronization
            NULL);
        if (ret < 0)
        {
            Master::G()->Log("sceNpScoreGetRankingByNpId() failed. ret = 0x%x", ret);
            // destroy transaction context
            ret = sceNpScoreDestroyTransactionCtx(m_TransactionId);
            if (ret < 0) {
                Master::G()->Log("sceNpScoreDestroyTransactionCtx() failed. ret = 0x%x", ret);
            }
            m_TransactionId = -1;
            return FALSE;
        }

        CTaskID id = 0;
        Master::G()->GetTaskMgr()->AddTask(EGSTaskType_StatsRetrieveLocal,m_TransactionId,this,&id);
        Master::G()->GetTaskMgr()->StartTask(id);
    }
#endif

    return TRUE;
}

#if defined(_XBOX)
StatsSrv::LeaderboardInfo::LeaderboardInfo(GS_INT lbNum, XSESSION_VIEW_PROPERTIES* pViews)
{
	m_iLBNum = lbNum;

    // index counter is for add customized properties
    GS_INT index = 0;
    m_pViewProp = new(GSOPType) XSESSION_VIEW_PROPERTIES[m_iLBNum];

	// init all leaderboards
	for (GS_INT i=0;i<lbNum;i++,index++)
	{
		m_pViewProp[index].dwViewId = pViews[i].dwViewId;
		m_pViewProp[index].dwNumProperties = pViews[i].dwNumProperties;
		// init all properties in one leaderboard
		m_pViewProp[index].pProperties = new(GSOPType) XUSER_PROPERTY[m_pViewProp[index].dwNumProperties];
		for (GS_UINT j=0;j<m_pViewProp[index].dwNumProperties;j++)
		{
			m_pViewProp[index].pProperties[j].dwPropertyId = pViews[i].pProperties[j].dwPropertyId;
			memcpy(&(m_pViewProp[index].pProperties[j].value), &(pViews[i].pProperties[j].value), sizeof(XUSER_DATA));
		}
	}
}
StatsSrv::LeaderboardInfo::~LeaderboardInfo()
{
	if (m_pViewProp && m_iLBNum > 0)
	{
		for (GS_INT i=0;i<m_iLBNum;i++)
		{
			if (m_pViewProp[i].pProperties)
			{
				Free(m_pViewProp[i].pProperties);
			}
		}

		Free(m_pViewProp);
		m_pViewProp = NULL;
		m_iLBNum = 0;
	}
}
#else
StatsSrv::LeaderboardInfo::~LeaderboardInfo()
{}
#endif

//--------------------------------------------------------------------------------------
// Name: SetLBInfo()
// Desc: Set stats for write leaderboard
//--------------------------------------------------------------------------------------
#if defined(_XBOX) || defined(_XENON)
void StatsSrv::PreWriteLeaderboard(GS_INT lbNum, XSESSION_VIEW_PROPERTIES* pViews)
{
	if (lbNum <= 0 || !pViews)
		return;

    if (m_pLBInfoArray.Num() > 0)
    {
        Master::G()->Log("GameService SetLBInfo: Already have unfinished LBinfo: %i", m_pLBInfoArray.Num());
    }

	LeaderboardInfo* pLBInfo = new(GSOPType) LeaderboardInfo(lbNum, pViews);
    m_pLBInfoArray.AddItem(pLBInfo);
}
#endif

//--------------------------------------------------------------------------------------
// Name: GetLocalStats_Key()
// Desc: Get Key Column value
//--------------------------------------------------------------------------------------
GS_INT64 StatsSrv::GetLocalStats_Key(GS_INT userIndex, GS_UINT lbIndex)
{
	// TODO: support multi local players
	if (!m_bUserStatsRetrieved || m_bReadingLeaderboard)
		return 0;

#if defined(_XBOX) || defined(_XENON)
    if (!m_pStats)
        return NULL;

	XUSER_STATS_READ_RESULTS* local_stats = (XUSER_STATS_READ_RESULTS*)m_pStats;
	if (local_stats->dwNumViews <= lbIndex)
		return NULL;
	return (local_stats->pViews[lbIndex].pRows->i64Rating);
#elif defined(_PS3)
    if (!m_pMyPlayerRankData || !m_pMyPlayerRankData->hasData)
        return 0;

    return m_pMyPlayerRankData->rankData.scoreValue;
#else
    return 0;
#endif
}

//--------------------------------------------------------------------------------------
// Name: WriteStats()
// Desc: Write stats for the local users 
//--------------------------------------------------------------------------------------
GS_BOOL StatsSrv::WriteLeaderboard(GS_INT userIndex)
{
	if (m_bWritingLeaderboard 
#if defined(_XBOX) || defined(_XENON)
        || !Master::G()->GetSessionSrv()->IsStarted()
#endif
        )
		return FALSE;
	
#if defined(_XBOX) || defined(_XENON)
	if (m_pLBInfoArray.Num() ==0 || m_pLBInfoArray(0) == NULL)
        return FALSE;

	CTaskID id = 0;
    GS_DWORD ret = XSessionWriteStats(
		Master::G()->GetSessionSrv()->GetCurrentHandle(),
		// TODO: for support multi local players
		SignIn::GetXUID(userIndex),
        m_pLBInfoArray(0)->m_iLBNum,
        m_pLBInfoArray(0)->m_pViewProp,
        Master::G()->GetTaskMgr()->AddTask(EGSTaskType_StatsWrite,this,&id) );
	Master::G()->GetTaskMgr()->StartTask(id,ret);

    if( ret != ERROR_IO_PENDING )
    {
		Master::G()->Log("XSessionWriteStats() failed with error %d", ret);
        return FALSE;
    }
#elif defined(_PS3)
	static SceNpScoreComment comment;
	static SceNpScoreGameInfo game_info;
	static SceNpScoreRankNumber tmp_rank;
	int ret=-1;

	ret = sceNpScoreCreateTransactionCtx(m_ScoreCtxId);
	if (ret < 0) {
		Master::G()->Log("sceNpScoreCreateTransactionCtx() failed. ret = 0x%x", ret);
        return FALSE;
	}
	m_TransactionId = ret;

	memset(&comment, 0x00, sizeof(comment));
	memset(&game_info, 0x00, sizeof(game_info));
	ret = sceNpScoreRecordScoreAsync(
	    m_TransactionId,
	    m_LBDef.GetBoardID(),
	    m_LBDef.GetScore(),
	    &comment,
	    &game_info,
	    &tmp_rank,
        0, // async
	    NULL);
	if (ret < 0) {
		Master::G()->Log("sceNpScoreRecordScore() failed. ret = 0x%x", ret);
        sceNpScoreDestroyTransactionCtx(m_TransactionId);
        m_TransactionId = -1;
        return FALSE;
	}

    CTaskID id = 0;
    Master::G()->GetTaskMgr()->AddTask(EGSTaskType_StatsWrite,m_TransactionId,this,&id);
    Master::G()->GetTaskMgr()->StartTask(id);
#endif

    m_bWritingLeaderboard = TRUE;

	return TRUE;
}


// ======================================================== 
// Update
// WriteLeaderboard that pended by uncreated session
// ======================================================== 
void StatsSrv::Update()
{   
#if defined(_XBOX) || (_XENON)
    if (m_pLBInfoArray.Num() > 0)
    {
        WriteLeaderboard(SignIn::GetSignedInUser());
    }
#endif
    // +LCTEST
    static GS_INT s_testwriting = 0;
    if (s_testwriting)
    {
        GS_INT score = 3;
#if defined(_XBOX) || (_XENON)
#elif defined(_PS3)
        m_LBDef.Set(0, score);
#endif
        WriteLeaderboard(0);
    }
	static GS_INT s_testreading = 0;
	if (s_testreading)
	{
#if defined(_XBOX) || (_XENON)
		m_LBDef.Set(8,0,NULL);
        ReadLeaderboard(1, 0, 20);
#endif
	}
    // -LCTEST
}

//--------------------------------------------------------------------------------------
// Name: FlushLeaderboard()
// Desc: Force submite leaderboard to server
//--------------------------------------------------------------------------------------
GS_BOOL StatsSrv::FlushLeaderboard()
{
#if defined(_XBOX) || (_XENON)
	CTaskID id = 0;
    GS_DWORD ret = XSessionFlushStats(
		Master::G()->GetSessionSrv()->GetCurrentHandle(),
        Master::G()->GetTaskMgr()->AddTask(EGSTaskType_StatsFlush,this,&id) );
	Master::G()->GetTaskMgr()->StartTask(id,ret);

    if( ret != ERROR_IO_PENDING )
    {
		Master::G()->Log("XSessionWriteStats() failed with error %d", ret);
        return FALSE;
    }
#endif
	return TRUE;
}

//--------------------------------------------------------------------------------------
// Name: ReadLeaderboard()
// Desc: Read data from the leaderboard
//--------------------------------------------------------------------------------------
GS_BOOL StatsSrv::ReadLeaderboard(GS_INT idx, GS_INT userIndex, GS_INT maxNum, GS_INT myScoreOffset)
{
    if (m_bReadingLeaderboard && !m_bIsReadingMyScore)
        return FALSE;

    m_iFriendsIndexOffset = 0;

    if (0 == idx)
    {
        if (FALSE == m_bIsReadingMyScore)
        {
            // my score view:
            if (!RetrieveLocalUserStats(0/*async*/))
                return FALSE;

            m_bReadingLeaderboard = TRUE;
            m_bIsReadingMyScore = TRUE;
            m_iMyScoreUserIndex = userIndex;
            m_iMyScoreMaxNum = maxNum;
			m_iMyScoreOffset = myScoreOffset;

            return TRUE;
        }

        idx = GetRetrievedRank(0);
        if (0 == idx)
        {
            // i have not registered in leaderboard
            // so no rows to show
#if defined(_XBOX) || defined(_XENON)
            if(m_pStats)
            {
                Free(m_pStats);
                m_pStats = NULL;
            }
#elif defined(_PS3)
            m_iRetrievedNum = 0;
#endif
			m_bReadingLeaderboard = FALSE;
            return TRUE;
        }

        idx -= myScoreOffset;
        if (idx < 1)
        {
            idx = 1;
        }
    }

#if defined(_XBOX) || defined(_XENON)
    // Throw away old results
    if(m_pStats)
    {
        Free(m_pStats);
        m_pStats = NULL;
    }
#elif defined(_PS3)
	m_iRetrievedNum = 0;
#endif

#if defined(_XBOX) || defined(_XENON)
    HANDLE hEnumerator;
    GS_DWORD cbResults = 0;
    GS_DWORD ret;
    CTaskID id = 0;

    // Calculate the required buffer size
    // Nonzero index means enumerate by rank
    ret = XUserCreateStatsEnumeratorByRank(
        0,                       // Current title ID
        idx,                     // Index to start enumerating from
        maxNum,					 // Number of rows to retrieve
        1,                       // One stats spec
        m_LBDef.GetDefinition(),// StatsSrv spec,
        &cbResults,              // Size of buffer
        &hEnumerator );          // Enumeration handle

    if( ret != ERROR_SUCCESS )
    {
        Master::G()->Log("XUserCreateStatsEnumerator...() failed with error %d", ret);
        return FALSE;
    }

    // Allocate the buffer
    m_pStats = new(GSOPType) GS_BYTE[ cbResults ];

    // Enumerate
    ret = XEnumerate(
        hEnumerator,           // Enumeration handle
        ( PXUSER_STATS_READ_RESULTS )m_pStats,              // Buffer
        cbResults,             // Size of buffer
        NULL,                  // Number of rows returned; not used for asynch
        Master::G()->GetTaskMgr()->AddTask(EGSTaskType_StatsRead,this,&id) );       // Overlapped structure
    Master::G()->GetTaskMgr()->StartTask(id,ret);

    if( ret != ERROR_IO_PENDING )
    {
        Master::G()->Log("XEnumerate() failed with error %d", ret);
        return FALSE;
    }

#elif defined(_PS3)
    if (maxNum > NP_SCORE_MAX_RANK_DATA)
    {
        Master::G()->Log("sceNpScoreGetRankingByRange() Input error: maxNum > NP_SCORE_MAX_RANK_DATA");
        return FALSE;
    }

	int ret = -1;

    // normal process
	ret = sceNpScoreCreateTransactionCtx(m_ScoreCtxId);
	if (ret < 0) {
		Master::G()->Log("sceNpScoreCreateTransactionCtx() failed. ret = 0x%x", ret);
		return FALSE;
	}
	m_TransactionId = ret;

    ret = sceNpScoreGetRankingByRangeAsync(
        m_TransactionId,
        m_LBDef.GetBoardID(),
        idx,
        m_pRankData,
        maxNum*sizeof(SceNpScoreRankData),
        NULL,
        0,
        NULL,
        0,
        maxNum,
        &m_last_sort_date,
        &m_TotalRecord,
        0, // async
        NULL);
    if (ret < 0) {
        Master::G()->Log("sceNpScoreGetRankingByRange() failed. ret = 0x%x", ret);
        sceNpScoreDestroyTransactionCtx(m_TransactionId);
        m_TransactionId = -1;
        return FALSE;
    }

    m_iRetrievedNum = maxNum;
    m_iRetrieveStartIndex = idx;

    CTaskID id = 0;
    Master::G()->GetTaskMgr()->AddTask(EGSTaskType_StatsRead,m_TransactionId,this,&id);
    Master::G()->GetTaskMgr()->StartTask(id);
#endif

    m_bReadingLeaderboard = TRUE;

	return TRUE;
}

GS_BOOL StatsSrv::ReadFriendsLeaderboard(GS_INT idx, GS_INT userIndex, GS_INT maxNum)
{
    if (m_bReadingLeaderboard || m_bIsReadingMyScore)
        return FALSE;

#if defined(_XBOX) || defined(_XENON)
	if(m_pStats)
	{
		Free(m_pStats);
		m_pStats = NULL;
	}
#elif defined(_PS3)
	m_iRetrievedNum=0;
#endif	
	
    m_iFriendsIndexOffset = 0;

	if(FALSE == Master::G()->GetFriendsSrv()->RetrieveFriendsList(userIndex, TRUE))
		return FALSE;

    GS_INT friend_num = Master::G()->GetFriendsSrv()->GetFriendsCount() + 1; // always add my score
    if (0 == idx)
    {
        // means all
        idx = 1;
    }
    if (maxNum > friend_num)
    {
        maxNum = friend_num;
    }

#if defined(_XBOX) || defined(_XENON)
    GS_DWORD cbResults = 0;
    GS_DWORD ret = XUserReadStats(
        0,						// Current title ID
		maxNum,	// Number of users
		Master::G()->GetFriendsSrv()->GetFriendsXUID(),	// XUIDs of users
        1,						// Number of stats specs
        m_LBDef.GetDefinition(),				// StatsSrv spec(s)
        &cbResults,				// Size of buffer
        NULL,					// Buffer
        NULL );					// Perform synchronously

    if( ret != ERROR_INSUFFICIENT_BUFFER )
    {
		Master::G()->Log("Failed to retrieve the stats buffer size with error %d", ret);
		return FALSE;
    }

    // Allocate the buffer
    m_pStats = new(GSOPType) GS_BYTE[ cbResults ];

    // Retrieve the stats
    CTaskID id = 0;
    ret = XUserReadStats(
        0,                   // Current title ID
        maxNum,	// Number of users
        Master::G()->GetFriendsSrv()->GetFriendsXUID(),	// XUIDs of users
        1,                   // Number of stats specs
        m_LBDef.GetDefinition(),             // StatsSrv spec(s)
        &cbResults,          // Size of buffer
        (PXUSER_STATS_READ_RESULTS)m_pStats,            // Buffer
        Master::G()->GetTaskMgr()->AddTask(EGSTaskType_StatsReadFriend,this,&id) );     // Overlapped
    Master::G()->GetTaskMgr()->StartTask(id,ret);

    if( ret != ERROR_IO_PENDING )
    {
        Master::G()->Log("XUserReadStats() failed with error %d", ret);
        return FALSE;
    }

#elif defined(_PS3)
    int ret = -1;
    ret = sceNpScoreCreateTransactionCtx(m_ScoreCtxId);
    if (ret < 0) {
        Master::G()->Log("sceNpScoreCreateTransactionCtx() failed. ret = 0x%x", ret);
        return FALSE;
    }
    m_TransactionId = ret;

    GS_INT size = sizeof(SceNpScorePlayerRankData) * maxNum;
    m_pFriendsRankData = (SceNpScorePlayerRankData*)Alloc(size);
    if (m_pFriendsRankData == NULL) {
        sceNpScoreDestroyTransactionCtx(m_TransactionId);
        return FALSE;
    }

    SceNpId* start_ptr = Master::G()->GetFriendsSrv()->GetFriendsList();
    m_iFriendsIndexOffset += idx-1;
    ret = sceNpScoreGetRankingByNpIdAsync(
        m_TransactionId,
        m_LBDef.GetBoardID(),
        start_ptr,
        sizeof(SceNpId) * maxNum,
        m_pFriendsRankData,
        size,
        NULL,
        0,
        NULL,
        0,
        maxNum,
        &m_last_sort_date,
        &m_TotalRecord,
        0, // async
        NULL);
    if (ret < 0) {
        Master::G()->Log("sceNpScoreGetRankingByNpId() failed. ret = 0x%x", ret);
        Free((GS_BYTE*)m_pFriendsRankData);
        m_pFriendsRankData = NULL;
        sceNpScoreDestroyTransactionCtx(m_TransactionId);
        return FALSE;
    }

    CTaskID id = 0;
    Master::G()->GetTaskMgr()->AddTask(EGSTaskType_StatsReadFriend,m_TransactionId,this,&id);
    Master::G()->GetTaskMgr()->StartTask(id);

    m_iRetrievedNum = maxNum;
#endif

    m_bReadingLeaderboard = TRUE;

	return TRUE;
}

GS_INT StatsSrv::GetRetrievedCount()
{
    if (m_bReadingLeaderboard)
        return 0;

#if defined(_XBOX) || defined(_XENON)
    if (!m_pStats)
        return 0;

	XUSER_STATS_READ_RESULTS* pStats = (XUSER_STATS_READ_RESULTS*)m_pStats;
    if (pStats->dwNumViews > 1)
    {
        Master::G()->Log("[GameService] - More than 1 leaderboard retrieved: %d", pStats->dwNumViews);
        return 0;
    }
    if (pStats->dwNumViews == 0)
    {
        Master::G()->Log("[GameService] - No stats retrieved!");
        return 0;
    }	

    return pStats->pViews[0].dwNumRows - m_iFriendsIndexOffset;

#elif defined(_PS3)

    return m_iRetrievedNum - m_iFriendsIndexOffset;
#endif
}

GS_CHAR* StatsSrv::GetRetrievedName(GS_INT index)
{
    if (m_bReadingLeaderboard)
        return 0;

    index += m_iFriendsIndexOffset;

#if defined(_XBOX) || defined(_XENON)
    if (!m_pStats)
        return 0;

	XUSER_STATS_READ_RESULTS* pStats = (XUSER_STATS_READ_RESULTS*)m_pStats;
    return pStats->pViews[0].pRows[index].szGamertag;

#elif defined(_PS3)
    return m_pRankData[index].onlineName.data;
#endif
}

GS_INT StatsSrv::GetRetrievedRank(GS_INT index)
{
    if (m_bReadingLeaderboard && !m_bIsReadingMyScore)
        return 0;

    index += m_iFriendsIndexOffset;

#if defined(_XBOX) || defined(_XENON)
    if (!m_pStats)
        return 0;

	XUSER_STATS_READ_RESULTS* pStats = (XUSER_STATS_READ_RESULTS*)m_pStats;
    return pStats->pViews[0].pRows[index].dwRank;

#elif defined(_PS3)
    return m_pRankData[index].serialRank;
#endif
}

GS_INT64 StatsSrv::GetRetrievedScore(GS_INT index)
{
    if (m_bReadingLeaderboard)
        return 0;

    index += m_iFriendsIndexOffset;

#if defined(_XBOX) || defined(_XENON)
    if (!m_pStats)
        return 0;

	XUSER_STATS_READ_RESULTS* pStats = (XUSER_STATS_READ_RESULTS*)m_pStats;
    return pStats->pViews[0].pRows[index].i64Rating;

#elif defined(_PS3)
    return m_pRankData[index].scoreValue;
#endif
}

void StatsSrv::DebugOutputLeaderboard()
{
    if (m_bReadingLeaderboard)
        return;

#if defined(_XBOX) || defined(_XENON)
    if (!m_pStats)
        return;

	Master::G()->Log("Debug Leaderboard:");
	XUSER_STATS_READ_RESULTS* pStats = (XUSER_STATS_READ_RESULTS*)m_pStats;
	for (GS_UINT index_view = 0; index_view < pStats->dwNumViews; index_view++)
	{
		Master::G()->Log("==Index: %d\t", pStats->pViews[index_view].dwViewId);
		Master::G()->Log("TotoalRow: %d", pStats->pViews[index_view].dwTotalViewRows);
		for (GS_UINT index_row = 0; index_row < pStats->pViews[index_view].dwNumRows; index_row++)
		{
			Master::G()->Log("====BeginRow");
			Master::G()->Log("%s - Rank:%d - Rating:%ld", 
				pStats->pViews[index_view].pRows[index_row].szGamertag, 
				pStats->pViews[index_view].pRows[index_row].dwRank,
				pStats->pViews[index_view].pRows[index_row].i64Rating);
			Master::G()->Log("====EndRow");
		}
	}
#elif defined(_PS3)
    if (0 == m_TotalRecord)
        return;

    Master::G()->Log("[GameService] - DebugOutputLeaderboard");
    for (GS_INT i=0;i<m_TotalRecord;i++)
    {
        Master::G()->Log("Rank: %i, Name: %s, Score: %i", i, m_pRankData[i].onlineName.data, m_pRankData[i].scoreValue);
    }
#endif
}

#if defined(_XBOX) || defined(_XENON)
XUID 
#elif defined(_PS3)
SceNpId*
#endif
StatsSrv::GetRetrievedIDByIndex(GS_INT index)
{
    index += m_iFriendsIndexOffset;

#if defined(_XBOX) || defined(_XENON)
    if (m_pStats)
    {
        XUSER_STATS_READ_RESULTS* pStats = (XUSER_STATS_READ_RESULTS*)m_pStats;
        // always the first leaderboard
        return pStats->pViews[0].pRows[index].xuid;
    }

	return 0;
#elif defined(_PS3)
    return &(m_pRankData[index].npId);
#endif
}

int CompareRankingFunc(const void *data1, const void *data2)
{
#if defined(_XBOX) || defined(_XENON)
	//XUSER_STATS_READ_RESULTS* pStats = (XUSER_STATS_READ_RESULTS*)m_pStats;
    const XUSER_STATS_ROW *p1 = static_cast<const XUSER_STATS_ROW*>(data1);
    const XUSER_STATS_ROW *p2 = static_cast<const XUSER_STATS_ROW*>(data2);

    if (p1->dwRank < p2->dwRank)
    {
        return -1;
    }
    if (p1->dwRank == p2->dwRank)
    {
        return 0;
    }
    return 1;

#elif defined(_PS3)
	const SceNpScoreRankData *p1 = static_cast<const SceNpScoreRankData *>(data1);
	const SceNpScoreRankData *p2 = static_cast<const SceNpScoreRankData *>(data2);

	if (p1->serialRank < p2->serialRank) {
		return (-1);
	}
	if (p1->serialRank == p2->serialRank) {
		return (0);
	}
	return (1);
#endif
}

void StatsSrv::SortFriendsRanking()
{
#if defined(_XBOX) || defined(_XENON)
	XUSER_STATS_READ_RESULTS* pStats = (XUSER_STATS_READ_RESULTS*)m_pStats;
    for (GS_UINT i=0; i<pStats->pViews->dwNumRows; i++)
    {
        if (0 == pStats->pViews[0].pRows[i].dwRank)
        {
            m_iFriendsIndexOffset ++;
        }
    }
    qsort(pStats->pViews[0].pRows, pStats->pViews->dwNumRows, sizeof(XUSER_STATS_ROW), CompareRankingFunc);

#elif defined(_PS3)
	GS_INT num = m_iRetrievedNum;
    m_iRetrievedNum = 0;
	for (GS_INT i = 0; i < num; i++) 
    {
		if (m_pFriendsRankData[i].hasData) 
        {
			memcpy(&m_pRankData[m_iRetrievedNum], &m_pFriendsRankData[i].rankData, sizeof(SceNpScoreRankData));
			m_iRetrievedNum += 1;
		}
	}
	std::qsort(m_pRankData, m_iRetrievedNum, sizeof(SceNpScoreRankData), CompareRankingFunc);
#endif
}

void StatsSrv::MessageResponse(Message* message)
{
	if (message->GetMessageID() == EMessage_SignInChanged)
		return;

	CTaskID task_id = *(CTaskID*)message->ReadPayload(0);
	GS_TaskType taskType = (GS_TaskType)(*(GS_INT*)message->ReadPayload(1));
	GS_DWORD taskResult = *(GS_DWORD*)message->ReadPayload(2);
	GS_DWORD taskDetailedResult = *(GS_DWORD*)message->ReadPayload(3);
#if defined(_XBOX) || defined(_XENON)
	GS_DWORD taskExtendedResult = *(GS_DWORD*)message->ReadPayload(4);
#endif

	m_iError = taskResult;

    switch(taskType)
    {
    case EGSTaskType_StatsRetrieveLocal:
        {
#if defined(_XBOX) || defined(_XENON)
            if (taskResult == ERROR_SUCCESS)
#elif defined(_PS3)
            if (taskResult == 0 && taskDetailedResult >= 0)
#endif
            {
                m_bUserStatsRetrieved = TRUE;
            }
        }
        break;
    case EGSTaskType_StatsReadFriend:
        {
#if defined(_XBOX) || defined(_XENON)
            if (taskResult == ERROR_SUCCESS)
#elif defined(_PS3)
            if (taskResult == 0 && taskDetailedResult >= 0)
#endif
            {
                SortFriendsRanking();
            }
        }
        m_bReadingLeaderboard = FALSE;
        break;
    case EGSTaskType_StatsRead:
#if defined(_PS3)
        {
            if (m_iRetrieveStartIndex > m_TotalRecord)
            {
                m_iRetrievedNum = 0;
            }
            else if (m_iRetrievedNum + m_iRetrieveStartIndex > m_TotalRecord)
            {
                m_iRetrievedNum = m_TotalRecord - m_iRetrieveStartIndex + 1;
            }
        }
#endif
		m_bIsReadingMyScore = FALSE;
        m_bReadingLeaderboard = FALSE;
        break;
    case EGSTaskType_StatsWrite:
        {
            m_bWritingLeaderboard = FALSE;
#if defined(_XBOX) || defined(_XENON)
            if (m_pLBInfoArray(0))
            {
                Delete<LeaderboardInfo>(m_pLBInfoArray(0));
                m_pLBInfoArray.Remove(0);
            }
#endif
        }
        break;
	}

#if defined(_PS3)
    if (m_TransactionId > 0)
    {
        GS_INT ret = sceNpScoreDestroyTransactionCtx(m_TransactionId);
        if (ret < 0) {
            Master::G()->Log("sceNpScoreDestroyTransactionCtx() failed. ret = 0x%x", ret);
        }

        m_TransactionId = -1;
    }
#endif

	// Message to Interface
	Message* msg = Message::Create(EMessage_CallBackInterface);
	if (msg)
	{
		msg->AddPayload(taskType);
#if defined(_XBOX) || defined(_XENON)
		bool result = (ERROR_SUCCESS == taskResult) ? true : false;
#elif defined(_PS3)
		bool result = (0 == taskResult && taskDetailedResult >= 0) ? true : false;
#endif
		msg->AddPayload(result);

		msg->AddTarget(Master::G()->GetInterfaceMgr());

		Master::G()->GetMessageMgr()->Send(msg);
	}

    // handle RetrieveMyScore after RetrieveLocal
    if (EGSTaskType_StatsRetrieveLocal == taskType && m_bIsReadingMyScore)
    {
#if defined(_XBOX) || defined(_XENON)
        if (taskResult == ERROR_SUCCESS)
        {
#elif defined(_PS3)
        if (taskResult == 0 && taskDetailedResult >= 0)
        {
            // copy hasData PlayerRankData to RankData
            if (m_pMyPlayerRankData[0].hasData)
            {
                m_pRankData[0] = m_pMyPlayerRankData[0].rankData;
            }
            else
            {
                m_pRankData[0].serialRank = m_pRankData[0].rank = m_pRankData[0].highestRank = 0;
            }
#endif
            ReadLeaderboard(0, m_iMyScoreUserIndex, m_iMyScoreMaxNum, m_iMyScoreOffset);
        }
        else
        {
            m_bIsReadingMyScore = FALSE;
			m_bReadingLeaderboard = FALSE;
        }
    }

}

} // namespace GameService
