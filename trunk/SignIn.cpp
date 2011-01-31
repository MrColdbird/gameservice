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
#endif

namespace GameService
{

//--------------------------------------------------------------------------------------
// Static members
//--------------------------------------------------------------------------------------
GS_BOOL   SignIn::m_bBeforePressStart            = TRUE;
#if !defined(_WINDOWS)
GS_DWORD	SignIn::m_dwActiveUserIndex           = 0;
#endif
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
GS_BOOL	SignIn::m_bPrivate[ XUSER_MAX_COUNT ]	={0,0,0,0};	// Users consuming private slots
GS_CHAR	SignIn::m_cUserName[XUSER_MAX_COUNT][MAX_USER_NAME] = 
{
    0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,
    0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,
    0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,
};
GS_BOOL	SignIn::m_bCanNotifyNoProfile = FALSE;
#elif defined(_PS3)
GS_BOOL SignIn::m_bStarNPInProgress = FALSE;
GS_BOOL SignIn::m_bIsStarNPRequestPending = FALSE;
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

#include "Customize/PS3_IDGarden.txt"

// NP POOL
uint8_t G_NPPOOL[SCE_NP_MIN_POOL_SIZE];
#define HTTP_POOL_SIZE      (512 * 1024)
#define HTTP_COOKIE_POOL_SIZE      (128 * 1024)
#define SSL_POOL_SIZE       (300 * 1024)
static CellHttpsData G_caList;
static GS_BYTE* G_HttpPool;
static GS_BYTE* G_HttpCookiePool;
static GS_BYTE* G_SslPool;
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
                break;
            case CELL_NET_CTL_ERROR_NET_NOT_CONNECTED:
                Master::G()->Log("CELL_SYSUTIL_NET_CTL_NETSTART_FINISHED result: CELL_NET_CTL_ERROR_NET_NOT_CONNECTED");
                break;
            default:
                Master::G()->Log("CELL_SYSUTIL_NET_CTL_NETSTART_FINISHED result error = 0x%x", result.result);
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
            StartNP();
        }
		break;
	case SCE_NP_MANAGER_STATUS_OFFLINE:
		Master::G()->Log("[GameService] - <MANAGER CB> offline");
        {
            StartNP();
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
        StartNP();
    }
}

GS_BOOL SignIn::CheckConnection()
{
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
    GS_Assert( dwMaxUsers <= 4 && dwMinUsers <= dwMaxUsers );
    GS_Assert( dwSignInPanes <= 4 && dwSignInPanes != 3 );

    // Assign variables
    m_dwMinUsers = dwMinUsers;
    m_dwMaxUsers = dwMaxUsers;
    m_bRequireOnlineUsers = bRequireOnlineUsers;
    m_dwSignInPanes = dwSignInPanes;

    m_bCanNotifyNoProfile = FALSE;

    // Register our notification listener
    m_hNotification = XNotifyCreateListener( XNOTIFY_SYSTEM | XNOTIFY_LIVE );
    if( m_hNotification == NULL || m_hNotification == INVALID_HANDLE_VALUE )
    {
        Master::G()->Log( "Failed to create state notification listener.\n" );
    }

    QuerySigninStatus();

#elif defined(_PS3)
    if (!StartNet())
    {
        StartNP();
        return;
    }

    if (bRequireOnlineUsers)
    {
        SignInNP();
    }
    else
    {
        StartNP();
    }
#endif
}

#if defined(_PS3)
static void
freeCerts(void)
{
	if(G_caList.ptr != NULL)
		GS_DELETE [] G_caList.ptr;
}
static int
loadCerts(void)
{
	char *buf = NULL;
	size_t size = 0;
	int ret = 0;

	ret = cellSslCertificateLoader(CELL_SSL_LOAD_CERT_ALL, NULL, 0, &size);
	if(ret < 0){
		Master::G()->Log("cellSslCertificateLoader() failed. ret = 0x%x\n", ret);
		goto error;
	}

	memset(&G_caList, 0, sizeof(G_caList));
	G_caList.ptr = (char*)GS_NEW GS_BYTE[size];
	if(NULL == G_caList.ptr){
		Master::G()->Log("malloc failed for cert pinter... \n");
		ret = -1;
		goto error;
	}
	buf = (char*)(G_caList.ptr);
	G_caList.size = size;

	ret = cellSslCertificateLoader(CELL_SSL_LOAD_CERT_ALL, buf, size, NULL);
	if(ret < 0){
		Master::G()->Log("cellSslCertificateLoader() failed. ret = 0x%x\n", ret);
		goto error;
	}

	return 0;

error:
	freeCerts();
	return ret;
}

GS_BOOL SignIn::StartNet()
{
	GS_INT ret = -1;
	ret = cellSysmoduleLoadModule(CELL_SYSMODULE_NET);
    if (ret >= 0)
    {
        ret = sys_net_initialize_network();
        if (ret != CELL_OK) 
        {
            Master::G()->Log( "sys_net_initialize_network failed (0x%x)\n", ret );

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
	
    ret = cellSysmoduleLoadModule(CELL_SYSMODULE_HTTP);
	if (ret < 0) {
		Master::G()->Log("cellSysmoduleLoadModule failed (0x%x)", ret);
		return FALSE;
	}

    // NOTE: 
    // InGameMarketplace init only:
    if (Master::G()->IsEnableInGameMarketplace())
    {
        // init NetCtl
        ret = cellSysmoduleLoadModule(CELL_SYSMODULE_HTTPS);
        if (ret < 0) {
            Master::G()->Log("CELL_SYSMODULE_HTTPS failed. ret = 0x%x", ret);
			return FALSE;
        }

		G_HttpPool = GS_NEW GS_BYTE[HTTP_POOL_SIZE];
		if(G_HttpPool == NULL){
			Master::G()->Log( "SignIn::StartNet - failed to malloc libhttp memory pool\n");
			return FALSE;
		}
		
		ret = cellHttpInit(G_HttpPool, HTTP_POOL_SIZE);
        if(ret < 0){
            Master::G()->Log("cellHttpInit() failed. ret = 0x%x\n", ret);
            return FALSE;
        }

		G_HttpCookiePool = GS_NEW GS_BYTE[HTTP_COOKIE_POOL_SIZE];
		if(G_HttpCookiePool == NULL){
			Master::G()->Log( "SignIn::StartNet - failed to malloc libhttp memory pool\n");
			return FALSE;
		}
		
		ret = cellHttpInitCookie(G_HttpCookiePool, HTTP_COOKIE_POOL_SIZE);
        if(ret < 0){
            Master::G()->Log("cellHttpInitCookie() failed. ret = 0x%x\n", ret);
            return FALSE;
        }

        if (loadCerts() < 0)
            return FALSE;

		G_SslPool = GS_NEW GS_BYTE[SSL_POOL_SIZE];
		if(G_SslPool == NULL){
			return FALSE;
		}
		
		ret = cellSslInit(G_SslPool, SSL_POOL_SIZE);
        if(ret < 0){
            Master::G()->Log("cellSslInit() failed. ret = 0x%x\n", ret);
            return FALSE;
        }

        ret = cellHttpsInit(1, &G_caList);
        if(ret < 0){
            Master::G()->Log("cellHttpsInit() failed. ret = 0x%x\n", ret);
            return FALSE;
        }
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

GS_BOOL SignIn::StartNP()
{
    if (m_bStarNPInProgress)
    {
        m_bIsStarNPRequestPending = TRUE;
        return FALSE;
    }

    sys_ppu_thread_t temp_id;
	int ret = -1;
    ret = sys_ppu_thread_create(
        &temp_id, PS3_StartNP_Thread,
        (uintptr_t)(Master::G()), THREAD_PRIO, STACK_SIZE,
        0, "GameService StarNP Thread");
    if (ret < 0) {
        Master::G()->Log("[GameService] - sys_ppu_thread_create() for StartNP failed (%x)", ret);
        return FALSE;
    }

    m_bStarNPInProgress = TRUE;

    return TRUE;
}

void SignIn::PS3_StartNP_Thread(uint64_t instance)
{
    Master* master = (Master*)instance;

    GS_INT ret = -1;
    ret = cellSysmoduleLoadModule(CELL_SYSMODULE_SYSUTIL_NP);
    if (ret >= 0)
    {
        ret = sceNpInit(SCE_NP_MIN_POOL_SIZE, G_NPPOOL);
        if (ret != CELL_OK && ret != SCE_NP_ERROR_ALREADY_INITIALIZED) 
        {
            master->Log( "sceInit failed (0x%x)\n", ret );

            ret = sceNpTerm();
            if (ret < 0) 
            {
                master->Log("sceNpTerm() failed (0x%x)", ret);
            }

            ret = cellSysmoduleUnloadModule(CELL_SYSMODULE_SYSUTIL_NP);
            if (ret < 0)
            {
                master->Log("cellSysmoduleUnloadModule() failed (0x%x)", ret);
            }
            return;
        }
    }
    else
    {
        return;
    }

	// register notification when connection status changed
	ret = sceNpManagerRegisterCallback(NpManagerCallBack, NULL);
	if (ret < 0) {
		master->Log("sceNpManagerRegisterCallback() failed. ret = 0x%x", ret);
	}

	int npStatus = -2;
	ret = sceNpManagerGetStatus(&npStatus);
    if (ret == 0)
    {
        m_bIsOnline = FALSE;
        if (SCE_NP_MANAGER_STATUS_ONLINE == npStatus)
        {
            m_bIsOnline = TRUE;
        }

        master->Log("[GameService] - sceNpManagerGetStatus: %d", npStatus);
    }

	ret = sceNpManagerGetNpId(&m_sceNpID);
	if (ret < 0) {
		master->Log("sceNpManagerGetNpId() failed. ret = 0x%x", ret);
	}

    Master::G()->Log("[GameService] - GetOnlineId: %s", m_sceNpID.handle.data);

	ret = sceNpManagerGetOnlineName(&m_sceOnlineName);
	if (ret < 0) {
		master->Log("sceNpManagerGetOnlineName() failed. ret = 0x%x", ret);
	}
    master->Log("[GameService] - GetOnlineName: %s", m_sceOnlineName.data);

	ret = sceNpManagerGetAvatarUrl(&m_sceAvatarUrl);
	if (ret < 0) {
		master->Log("sceNpManagerGetAvatarUrl() failed. ret = 0x%x", ret);
	}
    master->Log("[GameService] - GetAvatarUrl: %s", m_sceAvatarUrl.data);

	ret = sceNpManagerGetMyLanguages(&m_sceMyLang);
	if (ret < 0) {
		master->Log("sceNpManagerGetMyLanguages() failed. ret = 0x%x", ret);
	}
    master->Log("[GameService] - GetMyLanguage: %d", m_sceMyLang.language1);

	ret = sceNpManagerGetAccountRegion(&m_sceCountryCode, &m_sceLangCode);
	if (ret < 0) {
		master->Log("sceNpManagerGetAccountRegion() failed. ret = 0x%x", ret);
	}
    master->Log("[GameService] - GetAccountRegion: %s", m_sceCountryCode.data);

    ret = sceNpManagerGetAccountAge(&m_UserAge);
	if (ret < 0) {
		master->Log("sceNpManagerGetAccountAge() failed. ret = 0x%x", ret);
	}
    master->Log("[GameService] - GetAccountAge: %d", m_UserAge);

    // TODO:
    // support sub-signin
    master->InitServices();

 //   if (master->GetAchievementSrv())
 //       master->GetAchievementSrv()->ReadAchievements(0,0,master->GetAchievementSrv()->GetCountMax());

    m_nNumUsers = 1;

	//InGameBrowsing Init for trial version
//     if(Master::G()->IsTrialVersion())
//     {
// #ifdef INGAMEBROWSING
//         master->GetInGameBrowsingSrv()->Init();
// #else
//         master->GetStoreBrowsingSrv()->Init();
// #endif
//     }

    // sign in succeed!

    // ret = sceNpBasicRegisterContextSensitiveHandler(
    //         &m_NpCommID, NpBasicCallBack, NULL);
    // if (ret < 0) {
    //     Master::G()->Log("sceNpBasicRegisterHandler() failed. ret = 0x%x", ret);
    //     sceNpBasicUnregisterHandler();
    // }

    while (m_bIsStarNPRequestPending)
    {
        m_bIsStarNPRequestPending = FALSE;
        PS3_StartNP_Thread(instance);
    }

    // NpBasicSendInitPresence();

    m_bStarNPInProgress = FALSE;

    sys_ppu_thread_exit(0);
}

GS_BOOL SignIn::GetNPStatus()
{
	int npStatus = -2;
	int ret = sceNpManagerGetStatus(&npStatus);
    if (ret == 0 && SCE_NP_MANAGER_STATUS_ONLINE == npStatus)
    {
        return TRUE;
    }

    return FALSE;
}

void SignIn::TermNP()
{
    //sceNpBasicUnregisterHandler();
    sceNpManagerUnregisterCallback();
    sceNpManagerTerm();
    sceNpTerm();
	cellSysmoduleUnloadModule(CELL_SYSMODULE_NET);
}
void SignIn::TermNet()
{
    if (Master::G()->IsEnableInGameMarketplace())
    {
        cellHttpsEnd();

        cellSslEnd();
		if(G_SslPool != NULL){
			GS_DELETE [] G_SslPool;
			G_SslPool = NULL;
		}

        freeCerts();

        cellHttpEndCookie();
		if(G_HttpCookiePool != NULL){
			GS_DELETE [] G_HttpCookiePool;
			G_HttpCookiePool = NULL;
		}

        cellHttpEnd();
		if(G_HttpPool != NULL){
			GS_DELETE [] G_HttpPool;
			G_HttpPool = NULL;
		}
    }

	int ret = cellSysmoduleUnloadModule(CELL_SYSMODULE_HTTP);
	if (ret < 0) {
		Master::G()->Log("cellSysmoduleUnloadModule failed (0x%x)", ret);
	}
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
    GS_Assert( m_hNotification != NULL && m_hNotification != INVALID_HANDLE_VALUE);  // ensure Initialize() was called

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
            m_SignInChanged = TRUE;

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

        case XN_LIVE_CONTENT_INSTALLED:
            {                
                // notify ContentMgr
                Message* msg = Message::Create(EMessage_ContentInstalled);
                if (msg)
                {
                    Master::G()->GetMessageMgr()->Send(msg);
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
                m_bCanNotifyNoProfile = TRUE;
                //jin yu:  return to game start set the flag
                m_bBeforePressStart = TRUE;
                m_bSigninUIWasShown = FALSE;
				//StorageDeviceReset();
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
		// TODO:
		// Add Xbox system msgbox in SysMsgBox class
     
		//GS_DWORD dwResult;

        //ZeroMemory( &m_Overlapped, sizeof( XOVERLAPPED ) );

        //WCHAR strMessage[512];
        //swprintf_s( strMessage, L"No profile signed in. Please sign in a profile to continue");

        //dwResult = XShowMessageBoxUI( XUSER_INDEX_ANY,
        //                              L"Signin Error",   // Message box title
        //                              strMessage,                 // Message
        //                              ARRAYSIZE( m_pwstrButtons ),// Number of buttons
        //                              m_pwstrButtons,             // Button captions
        //                              0,                          // Button that gets focus
        //                              XMB_ERRORICON,              // Icon to display
        //                              &m_MessageBoxResult,        // Button pressed result
        //                              &m_Overlapped );

        //if( dwResult != ERROR_IO_PENDING )
        //    Master::G()->Log( "Failed to invoke message box UI, error %d", dwResult );

        //m_bSystemUIShowing = TRUE;
        //m_bMessageBoxShowing = TRUE;
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
            Master::G()->Log( "Failed to invoke signin UI, error %d", ret );
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

    return 0;
#else
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
GS_CHAR* SignIn::GetUserNameStr(GS_UINT iUser)
{
#if defined(_XBOX) || defined(_XENON)
	ZeroMemory( m_cUserName[iUser], MAX_USER_NAME );

	XUserGetName( iUser, m_cUserName[iUser], MAX_USER_NAME );

    return m_cUserName[iUser];
#elif defined(_PS3)
	return (GS_CHAR*)m_sceOnlineName.data;
#else
	return 0;
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
#else
							 GS_INT
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

GS_VOID SignIn::StartGame(GS_UINT userIndex)
{
#if defined(_XBOX) || defined(_XENON)
	SetActiveUserIndex(userIndex);
	SetBeforePressStart(FALSE);
	QuerySigninStatus();
    // Rich Presence
#endif
}

} // namespace GameService

