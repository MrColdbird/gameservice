// ======================================================================================
// File         : Achievement.h
// Author       : Li Chen 
// Last Change  : 07/29/2010 | 15:44:52 PM | Thursday,July
// Description  : 
// ======================================================================================

#ifndef GAMESERVIE_ACHIEVEMENT_H
#define GAMESERVIE_ACHIEVEMENT_H

#include "Message.h"

namespace GameService
{

class AchievementSrv : public MessageRecipient
{
public:
	AchievementSrv() : m_dwCountMax(0), m_bHasRead(FALSE) {}
    AchievementSrv(MessageMgr* msgMgr, GS_INT count
#if defined(_PS3)
                   , GS_INT freespace
#endif
                   );
	virtual ~AchievementSrv();

    GS_BOOL Initialize();
    void Finalize();

    void ReadAchievements( GS_UINT userIndex, GS_INT startIndex, GS_INT count);
	void Write(GS_INT num, GS_INT* ids);
	void ShowSystemUI(GS_INT userIndex);

    GS_BOOL HasRead() { return m_bHasRead; }
	GS_BOOL IsEarned(GS_INT achieveIndex);

	// inherit from MessageHandler
	void MessageResponse(Message* message);

    GS_DWORD GetCountMax() { return m_dwCountMax; }

#if defined(_PS3)
    static void PS3_WriteAchievement_Thread(uint64_t instance);
    void WriteAchievment_PS3();
#endif

private:
	// for enumerate only:
    GS_DWORD   m_dwCountMax;
    GS_BOOL    m_bHasRead;
#if defined(_XBOX) || defined(_XENON)
    GS_BYTE*   m_Achievements;
	HANDLE	m_hReadHandle;
#endif
#if defined(_PS3)
	SceNpTrophyContext  m_TrophyCtx;
	SceNpTrophyHandle   m_TrophyHandle;
    SceNpTrophyDetails* m_TrophyDetails;
    SceNpTrophyData*    m_TrophyData;
    GS_INT*             m_iWantUnlockIDs;
    GS_INT              m_iWantUnlockIDNum;
    GS_UINT64           m_iTrophySpace;
    GS_INT              m_iFreeSpaceAvailable;
#endif
};

} // namespace GameService

#endif // GAMESERVIE_ACHIEVEMENT_H
