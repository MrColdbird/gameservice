// ======================================================================================
// File         : SignIn.cpp
// Author       : Li Chen 
// Last Change  : 07/29/2010 | 15:48:13 PM | Thursday,July
// Description  : 
// ======================================================================================

#include "stdafx.h"
#include "SignIn.h"
#include "Master.h"
#include "Achievement.h"
#include "Message.h"
#if defined(_PS3)
#include <netex/net.h>
#include <sys/timer.h>
#ifdef INGAMEBROWSING
#include <InGameBrowsing.h>
#else
#include <StoreBrowsing.h>
#endif
#include "../../Main/MainXenon/Sources/FinalVersionDef.h"
extern bool gi_IsFullVersion;
#endif

#if defined(_XBOX) || defined(_XENON)
extern "C" void xeINO_StorageDeviceReset(void);
#endif

namespace GameService
{

//--------------------------------------------------------------------------------------
// Static members
//--------------------------------------------------------------------------------------
GS_BOOL   SignIn::m_bBeforePressStart            = TRUE; 
GS_DWORD	SignIn::m_dwActiveUserIndex           = 0;
#if defined(_XBOX) || defined(_XENON)
GS_DWORD  SignIn::m_dwMinUsers                   = 0;     
GS_DWORD  SignIn::m_dwMaxUsers                   = 4;     
GS_BOOL   SignIn::m_bRequireOnlineUsers          = FALSE; 
GS_DWORD  SignIn::m_dwSignInPanes                = 4;     
HANDLE SignIn::m_hNotification                = NULL;  
GS_DWORD  SignIn::m_dwSignedInUserMask           = 0;     
GS_DWORD  SignIn::m_dwNumSignedInUsers           = 0;     
GS_DWORD  SignIn::m_dwOnlineUserMask             = 0;     
GS_DWORD  SignIn::m_dwFirstSignedInUser          = (GS_DWORD)-1;
GS_BOOL   SignIn::m_bSystemUIShowing             = FALSE; 
GS_BOOL   SignIn::m_bNeedToShowSignInUI          = FALSE; 
GS_BOOL   SignIn::m_bMessageBoxShowing           = FALSE;
GS_BOOL   SignIn::m_bSigninUIWasShown            = FALSE;
XOVERLAPPED SignIn::m_Overlapped              = {0};
LPCWSTR SignIn::m_pwstrButtons[2]             = { L"Sign In", L"Back" };
MESSAGEBOX_RESULT SignIn::m_MessageBoxResult  = {0};
GS_DWORD	SignIn::m_LastSignInStateChange		  = 0;
GS_BOOL	SignIn::m_SignInChanged				  = FALSE;
XUID	SignIn::m_InvalidID						= INVALID_XUID;
XUID	SignIn::m_Xuids   [ XUSER_MAX_COUNT ]	={0,0,0,0};					// Local XUIDs
GS_BOOL	SignIn::m_bPrivate[ XUSER_MAX_COUNT ]	={false,false,false,false};	// Users consuming private slots
GS_CHAR	SignIn::m_cUserName[XUSER_MAX_COUNT][MAX_USER_NAME] = 
{
    0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,
    0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,
    0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,
};
bool	SignIn::m_bCanNotifyNoProfile = false;
#elif defined(_PS3)
GS_INT                 SignIn::m_sceNPStatus       = SCE_NP_ERROR_NOT_INITIALIZED;
SceNpId				SignIn::m_sceNpID			= 
{
    {
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    // data 
            0,                                  // term
        {0,0,0}                               // dummy
    },
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0}
};
SceNpOnlineName     SignIn::m_sceOnlineName     = 
{
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // data
    0, // term
    {0,0,0} // padding
};
SceNpAvatarUrl      SignIn::m_sceAvatarUrl      =
{
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    0, // term
};
SceNpMyLanguages    SignIn::m_sceMyLang         =
{
    0,0, // language1/2/3 
    0, // term
    {0,0,0,0} // padding
};
SceNpCountryCode    SignIn::m_sceCountryCode    =
{
	{0,0}, // coutry code  
    0, // term
	{0}
};
GS_INT                 SignIn::m_sceLangCode       = 0;
int                 SignIn::m_UserAge           = 0;

// communication id
const SceNpCommunicationId SignIn::m_NpCommID = 
{
    {'N','P','W','R','0','1','5','7','6'},
    '\0',
    0,
    0
};
// signature
const SceNpCommunicationSignature SignIn::m_NpCommSig = 
{
    {
        0xb9,0xdd,0xe1,0x3b,0x01,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x66,0x92,0xcf,0xde,
        0xe7,0x78,0x90,0xec,0xb5,0x81,0xe4,0xc0,
        0x7b,0xab,0x2b,0x1d,0x84,0x08,0xdf,0xb4,
        0x0f,0x65,0x24,0x2c,0x5c,0xeb,0x30,0x33,
        0x49,0xdb,0xa8,0x90,0x2d,0x13,0x1e,0x52,
        0x15,0xfa,0xb3,0x0a,0x54,0xb1,0x48,0x42,
        0x9d,0x16,0x04,0x1d,0x79,0xa3,0x46,0xf3,
        0x9f,0xc3,0x17,0x0b,0xcb,0xad,0x43,0xb2,
        0x03,0x3d,0xf0,0xcf,0x7a,0x6d,0x56,0x4b,
        0xa0,0x9b,0x4c,0x72,0x06,0x7a,0xb4,0x19,
        0x5f,0xff,0xc0,0xda,0xf0,0x36,0xec,0xab,
        0x23,0xae,0x0a,0xf3,0x77,0xa0,0xf6,0xfb,
        0x31,0xe2,0x4d,0x24,0x55,0xed,0xaa,0xe7,
        0x10,0x24,0x78,0x87,0x3d,0xbb,0x19,0x80,
        0x12,0x79,0x65,0x39,0x82,0x44,0xf8,0xd5,
        0x6b,0x96,0x13,0x92,0x94,0x0c,0xa7,0xf4,
        0x8f,0x8e,0x05,0x7b,0x75,0xdc,0x0f,0xca,
        0x64,0x01,0x4d,0xae,0x1d,0x47,0xd2,0xec,
        0x60,0x1e,0x3c,0x4e,0x42,0x4c,0x2a,0x3d
    }
};
// passphrase
const SceNpCommunicationPassphrase SignIn::m_NpPassPhrase = 
{
    {
        0xd5,0xbf,0xeb,0xcc,0x24,0xfd,0xec,0x28,
        0xb6,0x48,0xf1,0x94,0x47,0x57,0x8d,0xf7,
        0x6e,0xe8,0x44,0x08,0x5f,0x07,0xcc,0xc9,
        0xe0,0x69,0x06,0x04,0xb6,0x2d,0x97,0x42,
        0x84,0xd0,0x0d,0x7e,0x6f,0x23,0x72,0x3f,
        0x3c,0x56,0xfd,0xbf,0x95,0x8d,0xc9,0xaf,
        0xd6,0x6e,0x4b,0x68,0x2c,0x70,0xab,0x9b,
        0x2b,0x02,0xe0,0xd2,0xb5,0xec,0xd4,0x26,
        0x21,0x07,0xd3,0x93,0x3e,0x3d,0x5a,0x3b,
        0xf3,0x17,0xd8,0x53,0x44,0xa9,0xd6,0xbd,
        0x06,0x68,0x46,0xb5,0x32,0x1a,0x8a,0x00,
        0x7d,0xeb,0x98,0x1a,0x86,0xa5,0x9c,0xf2,
        0xf0,0xc1,0x27,0x85,0x93,0x96,0x87,0x7f,
        0xb8,0xe0,0x22,0xcf,0x05,0xb8,0x71,0x81,
        0xb8,0x6b,0xad,0xcf,0x63,0xcc,0xe1,0x5e,
        0x1f,0x86,0x8b,0x86,0xd6,0x0e,0x90,0x91
    }
};
#if SCEA_SUBMISSION
const char SignIn::m_NpServiceID[] = "UP0001-NPUB30394_00";
#elif SCEE_SUBMISSION
const char SignIn::m_NpServiceID[] = "EP0001-NPEB00435_00";
#else
const char SignIn::m_NpServiceID[] = "UP0001-NPXX00865_00";
#endif


// NP POOL
#define NP_POOL_SIZE (128*1024)
uint8_t np_pool[NP_POOL_SIZE];
#endif

GS_UINT	SignIn::m_nNumUsers						= 0;							// Local users

#if defined(_PS3)
GS_INT     SignIn::m_iCheckConnectionSlot          = 2;
GS_BOOL    SignIn::m_bIsOnline                     = FALSE;

// callback for signing-in to network
void SignIn::NetSysUtilCbInternal(uint64_t status, uint64_t param, void * userdata)
{
	switch (status) {
	case CELL_SYSUTIL_NET_CTL_NETSTART_LOADED:
		Master::G()->Log("CELL_SYSUTIL_NET_CTL_NETSTART_LOADED");
		break;
	case CELL_SYSUTIL_NET_CTL_NETSTART_FINISHED:
		Master::G()->Log("CELL_SYSUTIL_NET_CTL_NETSTART_FINISHED");
        {
            struct CellNetCtlNetStartDialogResult result;
            int ret=-1;

            Master::G()->Log("[GameService] - Network connected");

            memset(&result, 0, sizeof(result));
            result.size = sizeof(result);
            ret = cellNetCtlNetStartDialogUnloadAsync(&result);
            if (ret < 0) {
                Master::G()->Log("cellNetCtlNetStartDialogUnloadAsync() failed. ret = 0x%x", ret);
            }
            switch (result.result) {
            case 0:
                StartNP(TRUE);
                break;
            case CELL_NET_CTL_ERROR_NET_NOT_CONNECTED:
                Master::G()->Log("CELL_NET_CTL_ERROR_NET_NOT_CONNECTED");
                StartNP(FALSE);
                break;
            default:
                Master::G()->Log("result error = 0x%x", result.result);
                StartNP(FALSE);
                break;
            }
        }
		break;
	case CELL_SYSUTIL_NET_CTL_NETSTART_UNLOADED:
		Master::G()->Log("CELL_SYSUTIL_NET_CTL_NETSTART_UNLOADED");
        cellSysutilUnregisterCallback(m_iCheckConnectionSlot);
		break;
	default:
		break;
	}
}

// callback for NpManager
void SignIn::NpManagerCallBack(int event, int result, void *arg)
{
	(void)result;
	switch (event) {
	case SCE_NP_MANAGER_STATUS_ONLINE:
		Master::G()->Log("[GameService] - <MANAGER CB> online");
        {
            StartNP(TRUE);
        }
		break;
	case SCE_NP_MANAGER_STATUS_OFFLINE:
		Master::G()->Log("[GameService] - <MANAGER CB> offline");
        {
            StartNP(FALSE);
        }
		break;
    case SCE_NP_MANAGER_EVENT_GOT_TICKET:
        {
            if (result > 0)
            {
				if (Master::G())
					Master::G()->SetTrackingNpTicketSize(result);
            }
            else
            {
                Master::G()->Log("SCE_NP_MANAGER_EVENT_GOT_TICKET failed: %x !!!", result);
            }
        }
        break;
	default:
		Master::G()->Log("<MANAGER CB> event:%d, result:0x%x", event, result);
		break;
	}
}
// void SignIn::NpBasicSendInitPresence()
// {
//     int status = SCE_NP_MANAGER_STATUS_OFFLINE;
//     int ret=-1;

//     ret = sceNpManagerGetStatus(&status);
//     if (ret < 0) {
//         Master::G()->Log("sceNpManagerGetStatus() failed. ret = 0x%x", ret);
//     }
//     switch (status) {
//     case SCE_NP_MANAGER_STATUS_ONLINE:
//         Master::G()->Log("[basic] send initial presence");
//         ret = sceNpBasicSetPresence(NULL, 0);
//         if (ret < 0) {
//             Master::G()->Log("sceNpBasicSetPresence() failed. ret = 0x%x", ret);
//         }
//         break;
//     default:
//         break;
//     }
// }

// callback for NpBasic Presence
int SignIn::NpBasicCallBack(int event, int retCode, uint32_t reqId, void *arg)
{
    // TODO:
    // send instant message
    return 0;
}

void SignIn::SignInNP()
{
    if (!CheckConnection())
    {
        StartNP(FALSE);
    }
}

GS_BOOL SignIn::CheckConnection()
{
#if !defined(_FINAL_RELEASE_)
	TimeCheck t = TimeCheck("CheckConnection");
#endif
	struct CellNetCtlNetStartDialogParam param;
	GS_INT ret = -1;
	ret = cellSysutilRegisterCallback(m_iCheckConnectionSlot, SignIn::NetSysUtilCbInternal, NULL);
    if (ret != CELL_OK) {
        Master::G()->Log("error: cellSysutilRegisterCallback() = 0x%x", ret);
		return FALSE;
    }
	memset(&param, 0, sizeof(param));
	param.size = sizeof(param);
	param.type = CELL_NET_CTL_NETSTART_TYPE_NP;
	ret = cellNetCtlNetStartDialogLoadAsync(&param);
	if (ret < 0) {
		Master::G()->Log("cellNetCtlNetStartDialogLoadAsync() failed. ret = 0x%x", ret);
        cellSysutilUnregisterCallback(m_iCheckConnectionSlot);
		return FALSE;
	}
	Master::G()->Log("[net start dialog] start...");

    return TRUE;
}
#endif

//--------------------------------------------------------------------------------------
// Name: Initialize()
// Desc: Sets up variables and creates notification listener
//--------------------------------------------------------------------------------------
GS_VOID SignIn::Initialize( GS_DWORD dwMinUsers,
    GS_DWORD dwMaxUsers,
    GS_BOOL bRequireOnlineUsers,
    GS_DWORD dwSignInPanes )
{
#if defined(_XBOX) || defined(_XENON)
    // Sanity check inputs
    assert( dwMaxUsers <= 4 && dwMinUsers <= dwMaxUsers );
    assert( dwSignInPanes <= 4 && dwSignInPanes != 3 );

    // Assign variables
    m_dwMinUsers = dwMinUsers;
    m_dwMaxUsers = dwMaxUsers;
    m_bRequireOnlineUsers = bRequireOnlineUsers;
    m_dwSignInPanes = dwSignInPanes;

    m_bCanNotifyNoProfile = false;

    // Register our notification listener
    m_hNotification = XNotifyCreateListener( XNOTIFY_SYSTEM | XNOTIFY_LIVE );
    if( m_hNotification == NULL || m_hNotification == INVALID_HANDLE_VALUE )
    {
        FatalError( "Failed to create state notification listener." );
    }

    QuerySigninStatus();

#elif defined(_PS3)
    if (!StartNet())
    {
        StartNP(FALSE);
        return;
    }

    if (bRequireOnlineUsers)
    {
        SignInNP();
    }
    else
    {
        StartNP(FALSE);
    }
#endif
}

#if defined(_PS3)
GS_BOOL SignIn::StartNet()
{
	GS_INT ret = -1;
	ret = cellSysmoduleLoadModule(CELL_SYSMODULE_NET);
    if (ret >= 0)
    {
        ret = sys_net_initialize_network();
        if (ret != CELL_OK) 
        {
            FatalError( "sys_net_initialize_network failed (0x%x)", ret );

            ret = sys_net_finalize_network();
            if (ret < 0) 
            {
                Master::G()->Log("sys_net_finalize_network() failed (0x%x)", ret);
            }

            ret = cellSysmoduleUnloadModule(CELL_SYSMODULE_NET);
            if (ret < 0)
            {
                Master::G()->Log("cellSysmoduleUnloadModule() failed (0x%x)", ret);
            }

            return FALSE;
        }
    }
    else
    {
        return FALSE;
    }

    // init NetCtl
	ret = cellSysmoduleLoadModule(CELL_SYSMODULE_NETCTL);
	if (ret < 0) {
		Master::G()->Log("CELL_SYSMODULE_SYSUTIL_NETCTL failed. ret = 0x%x", ret);
        return FALSE;
	}
	ret = cellNetCtlInit();
	if (ret < 0) {
		Master::G()->Log("cellNetCtlInit() failed. ret = 0x%x", ret);
        cellSysmoduleUnloadModule(CELL_SYSMODULE_NETCTL);
        return FALSE;
	}

    return TRUE;
}

GS_BOOL SignIn::StartNP(GS_BOOL isOnline)
{
#if !defined(_FINAL_RELEASE_)
	TimeCheck t = TimeCheck("StartNP");
#endif

    m_bIsOnline = isOnline;

    GS_INT ret = -1;
    ret = cellSysmoduleLoadModule(CELL_SYSMODULE_SYSUTIL_NP);
    if (ret >= 0)
    {
        ret = sceNpInit(NP_POOL_SIZE, np_pool);
        if (ret != CELL_OK && ret != SCE_NP_ERROR_ALREADY_INITIALIZED) 
        {
            FatalError( "sceInit failed (0x%x)", ret );

            ret = sceNpTerm();
            if (ret < 0) 
            {
                Master::G()->Log("sceNpTerm() failed (0x%x)", ret);
            }

            ret = cellSysmoduleUnloadModule(CELL_SYSMODULE_SYSUTIL_NP);
            if (ret < 0)
            {
                Master::G()->Log("cellSysmoduleUnloadModule() failed (0x%x)", ret);
            }
            return FALSE;
        }
    }
    else
    {
        return FALSE;
    }

    // sign in succeed!
    // register notification when connection status changed
	ret = sceNpManagerRegisterCallback(NpManagerCallBack, NULL);
	if (ret < 0) {
		Master::G()->Log("sceNpManagerRegisterCallback() failed. ret = 0x%x", ret);
	}

    // ret = sceNpBasicRegisterContextSensitiveHandler(
    //         &m_NpCommID, NpBasicCallBack, NULL);
    // if (ret < 0) {
    //     Master::G()->Log("sceNpBasicRegisterHandler() failed. ret = 0x%x", ret);
    //     sceNpBasicUnregisterHandler();
    // }

	int npStatus = -2;
	ret = sceNpManagerGetStatus(&npStatus);
    if (SCE_NP_MANAGER_STATUS_ONLINE == npStatus)
    {
        m_bIsOnline = TRUE;
    }
    if (ret == 0)
    {
        Master::G()->Log("[GameService] - sceNpManagerGetStatus: %d", npStatus);
    }

	ret = sceNpManagerGetNpId(&m_sceNpID);
	if (ret < 0) {
		Master::G()->Log("sceNpManagerGetNpId() failed. ret = 0x%x", ret);
	}

    Master::G()->Log("[GameService] - GetOnlineId: %s", m_sceNpID.handle.data);

	ret = sceNpManagerGetOnlineName(&m_sceOnlineName);
	if (ret < 0) {
		Master::G()->Log("sceNpManagerGetOnlineName() failed. ret = 0x%x", ret);
	}
    Master::G()->Log("[GameService] - GetOnlineName: %s", m_sceOnlineName.data);

	ret = sceNpManagerGetAvatarUrl(&m_sceAvatarUrl);
	if (ret < 0) {
		Master::G()->Log("sceNpManagerGetAvatarUrl() failed. ret = 0x%x", ret);
	}
    Master::G()->Log("[GameService] - GetAvatarUrl: %s", m_sceAvatarUrl.data);

	ret = sceNpManagerGetMyLanguages(&m_sceMyLang);
	if (ret < 0) {
		Master::G()->Log("sceNpManagerGetMyLanguages() failed. ret = 0x%x", ret);
	}
    Master::G()->Log("[GameService] - GetMyLanguage: %d", m_sceMyLang.language1);

	ret = sceNpManagerGetAccountRegion(&m_sceCountryCode, &m_sceLangCode);
	if (ret < 0) {
		Master::G()->Log("sceNpManagerGetAccountRegion() failed. ret = 0x%x", ret);
	}
    Master::G()->Log("[GameService] - GetAccountRegion: %s", m_sceCountryCode.data);

    ret = sceNpManagerGetAccountAge(&m_UserAge);
	if (ret < 0) {
		Master::G()->Log("sceNpManagerGetAccountAge() failed. ret = 0x%x", ret);
	}
    Master::G()->Log("[GameService] - GetAccountAge: %d", m_UserAge);

    // NpBasicSendInitPresence();

    // TODO:
    // support sub-signin
    Master::G()->InitServices();

    if (Master::G()->GetAchievementSrv())
        Master::G()->GetAchievementSrv()->ReadAchievements(0,0,Master::G()->GetAchievementSrv()->GetCountMax());

    m_nNumUsers = 1;

	//InGameBrowsing Init for trial version
	if(!gi_IsFullVersion)
	{
#ifdef INGAMEBROWSING
		Master::G()->GetInGameBrowsingSrv()->Init();
#else
		Master::G()->GetStoreBrowsingSrv()->Init();
#endif
	}
    return TRUE;

}
void SignIn::TermNP()
{
    // sceNpBasicUnregisterHandler();
    // sceNpManagerUnregisterCallback();
    sceNpManagerTerm();
    sceNpTerm();
	cellSysmoduleUnloadModule(CELL_SYSMODULE_NET);
}
void SignIn::TermNet()
{
    cellNetCtlTerm();
    cellSysmoduleUnloadModule(CELL_SYSMODULE_NETCTL);
    cellSysmoduleUnloadModule(CELL_SYSMODULE_SYSUTIL_NP);
    // sys_net_finalize_network();
}

GS_INT SignIn::IsCableConnected()
{
    int state = -1;
    int ret = cellNetCtlGetState(&state);
    if (ret != 0)
    {
        return 0;
    }

    return state == CELL_NET_CTL_STATE_IPObtained ? 1 : 0;
}
#endif 

#if defined(_XBOX) || defined(_XENON)
//--------------------------------------------------------------------------------------
// Name: QuerySigninStatus()
// Desc: Query signed in status of all users.
//--------------------------------------------------------------------------------------
GS_VOID SignIn::QuerySigninStatus()
{
    m_dwSignedInUserMask = 0; m_dwOnlineUserMask = 0;

    // Count the signed-in users
    m_dwNumSignedInUsers = 0;
    m_dwFirstSignedInUser = ( GS_DWORD )-1;

    m_nNumUsers = 0;

    for( GS_UINT nUser = 0;
        nUser < XUSER_MAX_COUNT;
        nUser++ )
    {
        XUSER_SIGNIN_STATE State = XUserGetSigninState( nUser );

        if( State != eXUserSigninState_NotSignedIn )
        {
            // Check whether the user is online
            GS_BOOL bUserOnline =
                State == eXUserSigninState_SignedInToLive;

            m_dwOnlineUserMask |= bUserOnline << nUser;

            // If we want Online users only, only count signed-in users
            if( !m_bRequireOnlineUsers || bUserOnline )
            {
                m_dwSignedInUserMask |= ( 1 << nUser );

                if( m_dwFirstSignedInUser == ( GS_DWORD )-1 )
                {
                    m_dwFirstSignedInUser = nUser;
                }

                ++m_dwNumSignedInUsers;
            }
        }

        // Retrieve local users
        if( IsUserSignedIn( nUser ) )
        {
            XUserGetXUID( nUser, &m_Xuids[ nUser ] );
            m_bPrivate[ nUser ] = FALSE;

            m_nNumUsers++;
        }

    }

    // check to see if we need to invoke the signin UI
    m_bNeedToShowSignInUI = !AreUsersSignedIn() && !m_bBeforePressStart;

    if (m_dwNumSignedInUsers > 0)
    {
        Master::G()->InitServices();
        if (Master::G()->GetAchievementSrv())
            Master::G()->GetAchievementSrv()->ReadAchievements(GetActiveUserIndex(),0,Master::G()->GetAchievementSrv()->GetCountMax());
    }
}
#endif

//--------------------------------------------------------------------------------------
// Name: Update()
// Desc: Does required per-frame processing for signin
//--------------------------------------------------------------------------------------
GS_DWORD SignIn::Update()
{
#if defined(_XBOX) || defined(_XENON)
    assert( m_hNotification != NULL );  // ensure Initialize() was called

    GS_DWORD dwRet = 0;

    // Check for system notifications
    GS_DWORD dwNotificationID;
    ULONG_PTR ulParam;

    //
    // For XN_SYS_SIGNINCHANGED, handle the case where the system sends a spurious
    // notification. See  the FAQ on 
    // Xbox 360 Central: https://xds.xbox.com/xbox360/nav.aspx?Page=devsupport/sitefaq.htm#misc17
    // for a description of the workaround
    //
    static const GS_DWORD dwTimeBeforeTrustingSignInChangedNotif = 1000;
    static GS_DWORD dwQuestionableSigninChangeNotifReceived = GetTickCount();
    static GS_UINT  cQuestionableSigninChangeNotifReceived = 0;

    if( XNotifyGetNext( m_hNotification, 0, &dwNotificationID, &ulParam ) )
    {
        switch( dwNotificationID )
        {
        case XN_SYS_SIGNINCHANGED:

            m_LastSignInStateChange = GetTickCount();
            m_SignInChanged = true;

            break;

        case XN_SYS_UI:
            dwRet |= SYSTEM_UI_CHANGED;
            m_bSystemUIShowing = static_cast<GS_BOOL>( ulParam );

			if (!m_bSystemUIShowing && !m_bBeforePressStart)
			{
		        if (m_bMessageBoxShowing)
					m_bSigninUIWasShown = FALSE;

				if (!m_bSigninUIWasShown && !m_bNeedToShowSignInUI)
					m_bBeforePressStart = !AreUsersSignedIn();

				m_bMessageBoxShowing = FALSE;
			}

            // check to see if we need to invoke the signin UI
            //m_bNeedToShowSignInUI = !AreUsersSignedIn() && !m_bBeforePressStart;
            break;

        case XN_LIVE_CONNECTIONCHANGED:
            {
                dwRet |= CONNECTION_CHANGED;
                if (ulParam != XONLINE_S_LOGON_CONNECTION_ESTABLISHED)
                {
                    //QuerySigninStatus();
                    // TODO:
                    // may need to inform other services connection lost message

                }
            }
            break;

        } // switch( dwNotificationID )
    } // if( XNotifyGetNext() )

	//Jin Yu+		
    if(m_SignInChanged)
    {
        GS_DWORD curTime = GetTickCount();
        if( (curTime - m_LastSignInStateChange) > 500 )
        {
            if(!AreUsersSignedIn() && !m_bBeforePressStart)
            {
                // for script
                m_bCanNotifyNoProfile = true;
                //jin yu:  return to game start set the flag
                m_bBeforePressStart = true;
                m_bSigninUIWasShown = FALSE;
				StorageDeviceReset();
            }

			// Query who is signed in
            QuerySigninStatus();

            if (m_dwNumSignedInUsers > 0)
            {
                // notify other GameService
                Message* msg = Message::Create(EMessage_SignInChanged);
                if (msg)
                {
                    msg->AddPayload(1);
                    Master::G()->GetMessageMgr()->Send(msg);
                }
            }

            m_SignInChanged = FALSE;
        }
    }		
	//Jin Yu-

    // If there are not enough or too many profiles signed in, display an 
    // error message box prompting the user to either sign in again or exit the sample
    if( !m_bMessageBoxShowing && !m_bSystemUIShowing && m_bSigninUIWasShown && !AreUsersSignedIn() && !m_bBeforePressStart )
    {
        GS_DWORD dwResult;

        ZeroMemory( &m_Overlapped, sizeof( XOVERLAPPED ) );

        WCHAR strMessage[512];
        swprintf_s( strMessage, L"No profile signed in. Please sign in a profile to continue");

        dwResult = XShowMessageBoxUI( XUSER_INDEX_ANY,
                                      L"Signin Error",   // Message box title
                                      strMessage,                 // Message
                                      ARRAYSIZE( m_pwstrButtons ),// Number of buttons
                                      m_pwstrButtons,             // Button captions
                                      0,                          // Button that gets focus
                                      XMB_ERRORICON,              // Icon to display
                                      &m_MessageBoxResult,        // Button pressed result
                                      &m_Overlapped );

        if( dwResult != ERROR_IO_PENDING )
            FatalError( "Failed to invoke message box UI, error %d", dwResult );

        m_bSystemUIShowing = TRUE;
        m_bMessageBoxShowing = TRUE;
    }

    // Wait until the message box is discarded, then either exit or show the signin UI again
    if( m_bMessageBoxShowing && XHasOverlappedIoCompleted( &m_Overlapped ) )
    {
        if( XGetOverlappedResult( &m_Overlapped, NULL, TRUE ) == ERROR_SUCCESS )
        {
            switch( m_MessageBoxResult.dwButtonPressed )
            {
                case 0:     // Show the signin UI again
                    ShowSignInUI();
                    m_bSigninUIWasShown = FALSE;
                    break;

                case 1:     // Reboot to the launcher
                    // XLaunchNewImage( XLAUNCH_KEYWORD_DEFAULT_APP, 0 );
                    break;

            }
        }
    }

    // Check to see if we need to invoke the signin UI
    if( !m_bMessageBoxShowing && m_bNeedToShowSignInUI && !m_bSystemUIShowing )
    {
        m_bNeedToShowSignInUI = FALSE;

        GS_DWORD ret = XShowSigninUI(
            m_dwSignInPanes,
            m_bRequireOnlineUsers ? XSSUI_FLAGS_SHOWONLYONLINEENABLED : 0 );

        if( ret != ERROR_SUCCESS )
        {
            FatalError( "Failed to invoke signin UI, error %d", ret );
        }
        else
        {
            m_bSystemUIShowing = TRUE;
            m_bSigninUIWasShown = TRUE;
        }
    }

    return dwRet;

#elif defined(_PS3)
    // TODO: 
    // if player's online status updated, 
    // handling

    cellSysutilCheckCallback();

    // +LCTEST
    static int s_itest = 0;
    if (s_itest)
    {
        int ret = 0;
        ret = IsCableConnected();
    }
    // -LCTEST

    return 0;
#endif
}

#if defined(_XBOX) || defined(_XENON)
//--------------------------------------------------------------------------------------
// Name: CheckPrivilege()
// Desc: Test to see if a user has a required privilege
//--------------------------------------------------------------------------------------
GS_BOOL SignIn::CheckPrivilege( GS_DWORD dwController, XPRIVILEGE_TYPE priv )
{
    GS_BOOL bResult;

    return
        ( XUserCheckPrivilege( dwController, priv, &bResult ) == ERROR_SUCCESS ) &&
        bResult;
}
#elif defined(_PS3)
#endif

// ======================================================== 
// GetCurrentUserName
// ======================================================== 
GS_CHAR* SignIn::GetUserName(GS_UINT iUser)
{
#if defined(_XBOX) || defined(_XENON)
	ZeroMemory( m_cUserName[iUser], MAX_USER_NAME );

	XUserGetName( iUser, m_cUserName[iUser], MAX_USER_NAME );

    return m_cUserName[iUser];
#elif defined(_PS3)
	return (GS_CHAR*)m_sceOnlineName.data;
#endif
}

// ======================================================== 
// Show GamerCard
// ======================================================== 
int GFunc_handler(int result, void* arg)
{
    Master::G()->Log("[GameService] - sceNpProfileCallGui handle result: %d", result);
    return 0;
}

void SignIn::ShowGamerCardUI(
#if defined(_XBOX) || defined(_XENON)
                             XUID 
#elif defined(_PS3)
                             SceNpId* 
#endif
                             id)
{
#if defined(_XBOX) || defined(_XENON)
    XShowGamerCardUI(GetActiveUserIndex(), id);
#elif defined(_PS3)
    void *arg;                            //User-defined pointer to be passed to

    int ret = -1;
    ret = sceNpProfileCallGui(id, GFunc_handler,arg,0);
    if (ret < 0)
    {
        Master::G()->Log("[GameService] - sceNpProfileCallGui failed: %d", ret);
    }
#endif
}

void SignIn::StartGame(GS_UINT userIndex)
{
#if defined(_XBOX) || defined(_XENON)
	SetActiveUserIndex(userIndex);
	SetBeforePressStart(FALSE);
	QuerySigninStatus();
    // Rich Presence
#endif
}

#if defined(_XBOX) || defined(_XENON)
void SignIn::StorageDeviceReset()
{
	xeINO_StorageDeviceReset();
}
#endif

} // namespace GameService

