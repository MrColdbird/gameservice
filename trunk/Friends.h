// ======================================================================================
// File         : Friends.h
// Author       : Li Chen 
// Last Change  : 09/21/2010 | 17:42:24 PM | Tuesday,September
// Description  : 
// ======================================================================================

#ifndef GAMESERVICE_FRIENDS_H
#define GAMESERVICE_FRIENDS_H

namespace GameService
{

class FriendsSrv
{
public:
    FriendsSrv();
	virtual ~FriendsSrv();

	GS_BOOL	RetrieveFriendsList(GS_DWORD userIndex, GS_BOOL includeMe = FALSE);
    GS_DWORD   GetFriendsCount() { return m_iFriendsCount; }

#if defined(_XBOX) || defined(_XENON)
    XONLINE_FRIEND* GetFriendsList() { return m_pFriends; } 
    XUID*           GetFriendsXUID() { return m_pXUIDs; }
#elif defined(_PS3)
    SceNpId*  GetFriendsList() { return m_pFriends; }
#endif
            

private:
	GS_DWORD				m_iLastRetrievedTime;
#if defined(_XBOX) || defined(_XENON)
    GS_DWORD				m_iFriendsCount;
    XONLINE_FRIEND*     m_pFriends;
    XUID*               m_pXUIDs;
#elif defined(_PS3)
    GS_UINT				m_iFriendsCount;
    SceNpId*            m_pFriends;
#endif
};

}

#endif // GAMESERVICE_FRIENDS_H

