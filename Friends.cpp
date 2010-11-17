// ======================================================================================
// File         : Friends.cpp
// Author       : Li Chen 
// Last Change  : 09/21/2010 | 17:27:28 PM | Tuesday,September
// Description  : 
// ======================================================================================

#include "stdafx.h"
#include "Customize/CusMemory.h"
#include "Friends.h"
#include "SignIn.h"
#include "Master.h"

namespace GameService
{

FriendsSrv::FriendsSrv()
: m_iFriendsCount(0), m_pFriends(NULL)
{
#if defined(_XBOX) || defined(_XENON)
    m_iLastRetrievedTime = GetTickCount();
    m_pXUIDs = NULL;
#elif defined(_PS3)
    // TODO:
    // Add PS3 Tick function
    m_iLastRetrievedTime = 0;
#endif
}

FriendsSrv::~FriendsSrv()
{
    Free((GS_BYTE*)m_pFriends);
    m_pFriends = NULL;

#if defined(_XBOX) || defined(_XENON)
    Free((GS_BYTE*)m_pXUIDs);
    m_pXUIDs = NULL;
#endif
}

GS_BOOL FriendsSrv::RetrieveFriendsList(GS_DWORD userIndex, GS_BOOL includeMe)
{
#if defined(_XBOX) || defined(_XENON)
    GS_DWORD cbBuffer;
    HANDLE hFriendsEnum;

    // Do not enumerate presence information too often to decrease network traffic
    if( (GetTickCount() - m_iLastRetrievedTime) > 1000 )
    {
        m_iLastRetrievedTime = GetTickCount();

        // Enumerate presence strings of friends
        GS_DWORD dwRet;
        dwRet = XFriendsCreateEnumerator(
            userIndex,                    // user of whom to enumerate friends
            0,                              // starting index
            MAX_FRIENDS,                    // max number of friends
            &cbBuffer,                      // size of buffer needed
            &hFriendsEnum );

        // Presence information not yet available
        if( dwRet != ERROR_SUCCESS )
            return FALSE;

        // Re-allocate the Friends buffer so that it is the exact size requested by XFriendsCreateEnumerator
        Free( (GS_BYTE*)m_pFriends );
        m_pFriends = NULL;
        m_pFriends = ( XONLINE_FRIEND* )Alloc( cbBuffer );
        if( m_pFriends == NULL )
        {
            CloseHandle( hFriendsEnum );
            return FALSE;
        }

        dwRet = XEnumerate( hFriendsEnum, m_pFriends, cbBuffer, &m_iFriendsCount, NULL );
        if( dwRet != ERROR_SUCCESS )
        {
            CloseHandle( hFriendsEnum );
            return FALSE;
        }

        CloseHandle( hFriendsEnum );

        // copy xuids to an array
        Free((GS_BYTE*)m_pXUIDs);
        m_pXUIDs = NULL;
        if (includeMe)
        {
            m_pXUIDs = (XUID*)Alloc((m_iFriendsCount+1)*sizeof(XUID));
        }
        else
        {
            m_pXUIDs = (XUID*)Alloc(m_iFriendsCount*sizeof(XUID));
        }

        for (GS_UINT i=0; i<m_iFriendsCount; i++)
        {
            m_pXUIDs[i] = m_pFriends[i].xuid;
        }
        if (includeMe)
        {
            m_pXUIDs[m_iFriendsCount] = SignIn::GetXUID(SignIn::GetActiveUserIndex());
        }
    }
#elif defined(_PS3)

    // TODO:
    // Add PS3 Tick function to limit retrieve frequency

	int ret=-1;
	ret = sceNpBasicGetFriendListEntryCount(&m_iFriendsCount);
	if (ret < 0) {
		Master::G()->Log("sceNpBasicGetFriendListEntryCount() failed. ret = 0x%x", ret);
		return FALSE;
	}

    Free( (GS_BYTE*)m_pFriends );
    m_pFriends = NULL;
    if (includeMe)
    {
        m_pFriends = (SceNpId *)Alloc(sizeof(SceNpId) * (m_iFriendsCount+1));
    }
    else
    {
        m_pFriends = (SceNpId *)Alloc(sizeof(SceNpId) * (m_iFriendsCount));
    }
	if (m_pFriends == NULL) {
        m_iFriendsCount = 0;
		return FALSE;
	}

	/* friend */
	for (GS_UINT i = 0; i < m_iFriendsCount; i++) {
		ret = sceNpBasicGetFriendListEntry(i, m_pFriends + i);
		if (ret < 0) {
			Master::G()->Log("sceNpBasicGetFriendListEntry() failed. ret = 0x%x", ret);
			break;
		}
	}
    if (includeMe)
    {
        m_pFriends[m_iFriendsCount] = SignIn::GetNpID();
    }
#endif
	return TRUE;
}

} // namespace GameService
