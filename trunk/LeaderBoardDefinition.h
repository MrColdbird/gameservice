// ======================================================================================
// File         : LeaderBoardDefinition.h
// Author       : Li Chen 
// Last Change  : 07/29/2010 | 15:47:37 PM | Thursday,July
// Description  : 
// ======================================================================================

#ifndef GAMESERVICE_LEADERBOARDDEFINITION_H
#define GAMESERVICE_LEADERBOARDDEFINITION_H

namespace GameService
{

#if defined(_XBOX) || defined(_XENON)
#define LB_COLUMN_MAX XUSER_STATS_ATTRS_IN_SPEC
#else
#define LB_COLUMN_MAX 100 // temp
#endif

class LeaderBoardDef
{
public:
	LeaderBoardDef() : m_iBoardID(-1)
#if defined(_XBOX) || defined(_XENON)
		, m_iColumnNum(0)
#endif
	{}

#if defined(_XBOX) || defined(_XENON)
	void Set(GS_DWORD bid, GS_INT num, GS_INT* values) 
#elif defined(_PS3)
    void Set(GS_DWORD bid, GS_INT score)
#endif
	{ 
		m_iBoardID = bid; 
#if defined(_XBOX) || defined(_XENON)
		m_Spec.dwViewId = bid;
		m_iColumnNum = num; 
		m_Spec.dwNumColumnIds = num;
		for (GS_UINT i=0;i<m_Spec.dwNumColumnIds;i++)
		{
			m_Spec.rgwColumnIds[i] = values[i];
		}
#elif defined(_PS3)
        m_iScore = score;
#endif
	}

#if defined(_XBOX) || defined(_XENON)
	PXUSER_STATS_SPEC GetDefinition() { return &m_Spec; }
#elif defined(_PS3)
    GS_INT GetScore() { return m_iScore; }
#endif

    GS_DWORD GetBoardID() { return m_iBoardID; }

private:
	GS_DWORD m_iBoardID;
#if defined(_XBOX) || defined(_XENON)
	GS_INT m_iColumnNum;
	GS_INT m_piColumnIDs[LB_COLUMN_MAX];
	XUSER_STATS_SPEC   m_Spec;                        // Stats specification
#elif defined(_PS3)
    GS_INT m_iScore;
#endif
};

} // namespace GameService

#endif // GAMESERVICE_LEADERBOARDDEFINITION_H
