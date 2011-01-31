// ======================================================================================
// File         : Session.h
// Author       : Li Chen 
// Last Change  : 07/29/2010 | 15:48:08 PM | Thursday,July
// Description  : 
// ======================================================================================


#pragma once
#ifndef GAMESERVICE_SESSION_H
#define GAMESERVICE_SESSION_H

#include "Message.h"

namespace GameService
{

class SessionSrv : public MessageRecipient
{
public:
	SessionSrv();
	SessionSrv(MessageMgr* msgMgr);
	virtual ~SessionSrv() {}

    GS_BOOL BeginSession();
    GS_BOOL JoinSession();
    GS_BOOL StartSession();
    GS_BOOL CanWriteStats();
    GS_BOOL EndSession();
    GS_BOOL LeaveSession();
    GS_BOOL DeleteSession();

	GS_BOOL IsCreated() { return m_IsCreated; }
	GS_BOOL IsStarted() 
    { 
        if (m_IsStarted
#if defined(_XBOX) || defined(_XENON)
			&& m_hSession != INVALID_HANDLE_VALUE
#endif
			)
        {
            return TRUE;
        }
        return FALSE; 
    }

#if defined(_XBOX) || defined(_XENON)
	HANDLE GetCurrentHandle() { return m_hSession; }
#endif
	// inherit from MessageHandler
	void MessageResponse(Message* message);

private:
    static const GS_DWORD m_dwPublicSlots       = 8;
    static const GS_DWORD m_dwPrivateSlots      = 8;

	GS_BOOL			m_IsCreated;
	GS_BOOL			m_IsStarted;

#if defined(_XBOX) || defined(_XENON)
	// SessionSrv data
    XSESSION_INFO      m_SessionInfo;      // SessionSrv info
    ULONGLONG          m_SessionNonce;     // Nonce
    HANDLE             m_hSession;         // Handle
    GS_DWORD                    m_iLastWriteStatsInSession;
#endif

};

} // namespace GameService

#endif // SESSION_H
