// ======================================================================================
// File         : SignIn.h
// Author       : Li Chen 
// Last Change  : 07/29/2010 | 15:48:22 PM | Thursday,July
// Description  : 
// ======================================================================================


#pragma once
#ifndef SIGNIN_H
#define SIGNIN_H

namespace GameService
{

#define MAX_USER_NAME 64

class SignIn
{
public:

	// Flags that can be returned from Update()
	enum SIGNIN_UPDATE_FLAGS
	{
		SIGNIN_USERS_CHANGED    = 0x00000001,
		SYSTEM_UI_CHANGED       = 0x00000002,
		CONNECTION_CHANGED      = 0x00000004
	};

#if defined(_XBOX) || defined(_XENON)
    // Check users that are signed in
    static GS_DWORD    GetSignedInUserCount()
    {
        return m_dwNumSignedInUsers;
    }
    static GS_DWORD    GetSignedInUserMask()
    {
        return m_dwSignedInUserMask;
    }
    static GS_BOOL     IsUserSignedIn( GS_DWORD dwController )
    {
        return ( m_dwSignedInUserMask & ( 1 << dwController ) ) != 0;
    }

    static GS_BOOL     AreUsersSignedIn()
    {		
//        return ( m_dwNumSignedInUsers >= m_dwMinUsers ) &&
//            ( m_dwNumSignedInUsers <= m_dwMaxUsers );
		//jin yu+ change to check if the contorller user is signed in.
		if(XUserGetSigninState( GetActiveUserIndex() ) == eXUserSigninState_NotSignedIn )
			return FALSE;
		else
			return TRUE;
		//jin yu-

    }

    // Get the first signed-in user
    static GS_DWORD    GetSignedInUser()
    {
        return m_dwFirstSignedInUser;
    }

    // Check users that are signed into live
    static GS_DWORD    GetOnlineUserMask()
    {
        return m_dwOnlineUserMask;
    }

    static GS_BOOL     IsUserSignedInOnline( GS_DWORD dwController )
    {
        return ( m_dwOnlineUserMask & ( 1 << dwController ) ) != 0;
    }



	static GS_DWORD GetActiveUserIndex()
	{
		return m_dwActiveUserIndex;
	}

	// Get the first signed-in user
	static void SetActiveUserIndex(GS_DWORD user)
	{
		m_dwActiveUserIndex = user; 
	}

#elif defined(_PS3)
	static GS_DWORD GetActiveUserIndex()
	{
		return m_dwActiveUserIndex;
	}
    static GS_INT GetOnlineStatus()
    {
        return m_sceNPStatus;
    }
    static SceNpId& GetNpID()
    {
        return m_sceNpID;
    }
    static SceNpOnlineName& GetOnlineName()
    {
        return m_sceOnlineName;
    }
    static SceNpAvatarUrl& GetAvatarUrl()
    {
        return m_sceAvatarUrl;
    }
    static SceNpMyLanguages& GetLanguage()
    {
        return m_sceMyLang;
    }
    static SceNpCountryCode& GetCountryCode()
    {
        return m_sceCountryCode;
    }
    static GS_INT GetLangCode()
    {
        return m_sceLangCode;
    }
#endif

	// for local user infos:
	static GS_UINT		GetUserNum()
	{
		return m_nNumUsers;
	}
#if defined(_XBOX) || defined(_XENON)
	static XUID&	GetXUID(GS_UINT iUser)
	{
		if (iUser < 0 || iUser >= XUSER_MAX_COUNT)
			return m_InvalidID;

		return m_Xuids[iUser];
	}
#elif defined(_PS3)
#endif
    static GS_CHAR* GetUserName(GS_UINT iUser);

#if defined(_XBOX) || defined(_XENON)
	static XUID*	GetXUIDArray()
	{
		return m_Xuids;
	}
#elif defined(_PS3)
#endif

#if defined(_XBOX) || defined(_XENON)
	static GS_BOOL		GetIsPrivate(GS_UINT iUser)
	{
		if (iUser < 0 || iUser >= XUSER_MAX_COUNT)
			return 0;

		return m_bPrivate[iUser];
	}
	static GS_BOOL*	GetIsPrivateArray()
	{
		return m_bPrivate;
	}

    static GS_BOOL     IsUserOnline( GS_DWORD dwController )
    {
        return ( m_dwOnlineUserMask & ( 1 << dwController ) ) != 0;
    }

    // Check the presence of system UI
    static GS_BOOL     IsSystemUIShowing()
    {
        return m_bSystemUIShowing || m_bNeedToShowSignInUI;
    }

    // Function to reinvoke signin UI
    static GS_VOID     ShowSignInUI()
    {
        m_bNeedToShowSignInUI = TRUE;
    }    
    
	// Check privileges for a signed-in users
    static GS_BOOL     CheckPrivilege( GS_DWORD dwController, XPRIVILEGE_TYPE priv );
	static GS_BOOL NotifyNoProfileSignedIn()
	{
		GS_BOOL ret = m_bCanNotifyNoProfile;
		if (m_bCanNotifyNoProfile)
		{
			m_bCanNotifyNoProfile = FALSE;
		}

		return ret;
	}

#elif defined(_PS3)
    static GS_BOOL StartNP();
    static GS_BOOL GetNPStatus();
    static void PS3_StartNP_Thread(uint64_t instance);
    static void TermNP();
    static void SignInNP();
    static GS_BOOL StartNet();
    static void TermNet();
    static GS_BOOL CheckConnection();
    static void NpStatusNotifyCallBack(int event, int result, void* arg);
    static void NpManagerCallBack(int event, int result, void *arg);
    static int NpBasicCallBack(int event, int retCode, uint32_t reqId, void *arg);
    static void NetSysUtilCbInternal(uint64_t status, uint64_t param, void * userdata);
    static void NpBasicSendInitPresence();
    static const SceNpCommunicationId* GetNpCommID()
    {
        return &m_NpCommID;
    }
    static const SceNpCommunicationSignature* GetNpCommSig()
    {
        return &m_NpCommSig;
    }
    static const SceNpCommunicationPassphrase* GetNpCommPass()
    {
        return &m_NpPassPhrase;
    }
    static const char* GetNpServiceID()
    {
        return m_NpServiceID;
    }
    static GS_BOOL IsUserOnline()
    {
        return m_bIsOnline;
    }
    static int GetUserAge()
    {
        return m_UserAge;
    }
    static GS_INT IsCableConnected();
#endif

	static void ShowGamerCardUI(
#if defined(_XBOX) || defined(_XENON)
                             XUID 
#elif defined(_PS3)
                             SceNpId* 
#endif
                             id);

    static void     SetBeforePressStart(GS_BOOL before)
    {
        m_bBeforePressStart = before;
    }
    static GS_BOOL     GetBeforePressStart()
    {
        return m_bBeforePressStart;
    }

    static void StartGame(GS_UINT userInex);

    // Methods to drive autologin
    static GS_VOID Initialize( 
        GS_DWORD dwMinUsers, 
        GS_DWORD dwMaxUsers,
        GS_BOOL  bRequireOnlineUsers,
        GS_DWORD dwSignInPanes );

    static GS_DWORD    Update();
    static GS_VOID     QuerySigninStatus();              // Query signed in users

private:

    // Private constructor to prevent instantiation
                    SignIn();

    // Parameters
    static GS_DWORD m_dwMinUsers;             // minimum users to accept as signed in
    static GS_DWORD m_dwMaxUsers;             // maximum users to accept as signed in
    static GS_BOOL m_bRequireOnlineUsers;    // online profiles only
    static GS_DWORD m_dwSignInPanes;          // number of panes to show in signin UI
    static GS_BOOL m_bBeforePressStart;

    // Internal variables
#if defined(_XBOX) || defined(_XENON)
    static HANDLE m_hNotification;                // listener to accept notifications
    static GS_DWORD m_dwSignedInUserMask;           // bitfields for signed-in users
    static GS_DWORD m_dwFirstSignedInUser;          // first signed-in user
    static GS_DWORD m_dwNumSignedInUsers;           // number of signed-in users
    static GS_DWORD m_dwOnlineUserMask;             // users who are online
    static GS_BOOL m_bSystemUIShowing;             // system UI present
    static GS_BOOL m_bNeedToShowSignInUI;          // invoking signin UI necessary
    static GS_BOOL m_bMessageBoxShowing;           // is retry signin message box showing?
    static GS_BOOL m_bSigninUIWasShown;            // was the signin ui shown at least once?
    static XOVERLAPPED m_Overlapped;              // message box overlapped struct
    static LPCWSTR  m_pwstrButtons[2];             // message box buttons
    static MESSAGEBOX_RESULT m_MessageBoxResult;  // message box button pressed
    static GS_BOOL        m_bPrivate[ XUSER_MAX_COUNT ]; // Users consuming private slots
	static GS_BOOL m_bCanNotifyNoProfile;
    static GS_CHAR m_cUserName[XUSER_MAX_COUNT][MAX_USER_NAME];
	static XUID m_InvalidID;
	//JinYu+
	static GS_DWORD m_dwActiveUserIndex;
	static GS_DWORD m_LastSignInStateChange;
	static GS_BOOL  m_SignInChanged;
	//JinYu-

#elif defined(_PS3)
	static GS_DWORD            m_dwActiveUserIndex;
    static GS_INT              m_iCheckConnectionSlot;
    static GS_BOOL             m_bIsOnline;
    static GS_INT              m_sceNPStatus;
	static SceNpId          m_sceNpID;
	static SceNpOnlineName  m_sceOnlineName;
	static SceNpAvatarUrl   m_sceAvatarUrl;
	static SceNpMyLanguages m_sceMyLang;
	static SceNpCountryCode m_sceCountryCode;
    static GS_INT              m_sceLangCode;
    static const SceNpCommunicationSignature    m_NpCommSig;
    static const SceNpCommunicationId           m_NpCommID;
    static const SceNpCommunicationPassphrase   m_NpPassPhrase;
    static const char                           m_NpServiceID[64];
    static int              m_UserAge;
    static GS_BOOL          m_bStarNPInProgress;
#endif

	// Latest SignIn infos
	static GS_UINT        m_nNumUsers;                   // Local users
#if defined(_XBOX) || defined(_XENON)
    static XUID        m_Xuids   [ XUSER_MAX_COUNT ]; // Local XUIDs
#elif defined(_PS3)
#endif

};

} // namespace GameService

#endif // SIGNIN_H
