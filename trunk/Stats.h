// ======================================================================================
// File         : Stats.h
// Author       : Li Chen 
// Last Change  : 07/29/2010 | 15:48:32 PM | Thursday,July
// Description  : 
// ======================================================================================

#pragma once
#ifndef STATS_H
#define STATS_H

#include "LeaderBoardDefinition.h"
#include "Message.h"
#include "TaskType.h"

namespace GameService
{

#if defined(_PS3)
#define NP_SCORE_MAX_RANK_DATA	(99)
#endif

class StatsSrv : public MessageRecipient
{
public:
	StatsSrv() : m_pStats(NULL) { }
	StatsSrv(MessageMgr* msgMgr);

	virtual ~StatsSrv();

    GS_BOOL Initialize();
    void Finalize();
    void Update();

    // Read/write stats
    GS_BOOL				RetrieveLocalUserStats(GS_BOOL bImmediately = FALSE);
	GS_BOOL				CanWriteStats() { return !m_bWritingLeaderboard; }
	GS_BOOL				FlushLeaderboard();
    GS_BOOL                WriteLeaderboard(GS_INT userIndex);
    GS_BOOL                ReadLeaderboard(GS_INT idx, GS_INT userIndex = 0, GS_INT maxNum = 8, GS_INT myScoreOffset = 0);
    GS_BOOL                ReadFriendsLeaderboard(GS_INT idx, GS_INT userIndex, GS_INT maxNum);

    GS_INT                 GetRetrievedCount();
    GS_CHAR*               GetRetrievedName(GS_INT index);
    GS_INT                 GetRetrievedRank(GS_INT index);
    GS_INT64				GetRetrievedScore(GS_INT index);
	GS_INT					GetErrorCode() { return m_iError; }

    void                SortFriendsRanking();
#if defined(_XBOX) || defined(_XENON)
    XUID                
#elif defined(_PS3)
    SceNpId*
#else
	GS_INT
#endif
                        GetRetrievedIDByIndex(GS_INT index);

	void				DebugOutputLeaderboard();
	GS_INT64			GetLocalStats_Key(GS_INT userIndex, GS_UINT lbIndex);
	GS_BOOL				IsReadingLeaderboard() { return m_bReadingLeaderboard; }

	LeaderBoardDef&		GetLBDef() { return m_LBDef; }

	class LeaderboardInfo 
	{
	public:
		LeaderboardInfo() : m_iLBNum(0)
#if defined(_XBOX) || defined(_XENON)
			, m_pViewProp(NULL) 
#endif
		{}
		virtual ~LeaderboardInfo();

#if defined(_XBOX) || defined(_XENON)
		LeaderboardInfo(GS_INT lbNum, XSESSION_VIEW_PROPERTIES* pViews);
		XSESSION_VIEW_PROPERTIES*	m_pViewProp; // Session properties when writing stats
#endif
		GS_INT m_iLBNum;
	};

#if defined(_XBOX) || defined(_XENON)
    void                PreWriteLeaderboard(GS_INT lbNum, XSESSION_VIEW_PROPERTIES* pViews);
#endif

	// inherit from MessageHandler
	void MessageResponse(Message* message);
	void SendMessageCallback(GS_INT taskType, GS_BOOL result);

private:
	GS_BOOL				        m_bUserStatsRetrieved;
	GS_BOOL				        m_bReadingLeaderboard;
	GS_BOOL				        m_bWritingLeaderboard;
	LeaderBoardDef		        m_LBDef;
	TArray<LeaderboardInfo*>	m_pLBInfoArray;
	GS_BYTE*				        m_pStats;

    GS_BOOL                        m_bIsReadingMyScore;
    GS_INT                         m_iMyScoreUserIndex;
    GS_INT                         m_iMyScoreMaxNum;
    GS_INT                         m_iMyScoreOffset;
    GS_INT                         m_iFriendsIndexOffset;

	GS_INT							m_iError;
    GS_BOOL                     m_bReadRequestDuringMyScore;
    GS_BOOL                     m_bReadRequestDuringMyScore_idx;
    GS_BOOL                     m_bReadRequestDuringMyScore_userIndex;
    GS_BOOL                     m_bReadRequestDuringMyScore_maxNum;

#if defined(_XBOX) || defined(_XENON)
	XUSER_STATS_SPEC	        m_Spec;                  // Stats specification
    GS_BOOL                     m_bFlushStatsWaiting;

	// TODO: support multi local player
    //GS_UINT               m_nWritingStatsUser;           // User currently writing stats for
#elif defined(_PS3)
    GS_INT                         m_ScoreCtxId;
	SceNpScorePlayerRankData    m_pMyPlayerRankData[1];
	SceNpScoreRankData          m_pRankData[NP_SCORE_MAX_RANK_DATA];
    SceNpScorePlayerRankData*   m_pFriendsRankData;
    GS_UINT                        m_iRetrieveStartIndex;
    GS_UINT                        m_iRetrievedNum;
	SceNpScoreRankNumber        m_TotalRecord;
	GS_INT                         m_TransactionId;
	CellRtcTick                 m_last_sort_date;

#endif

};

} // namespace GameService

#endif // STATS_H
