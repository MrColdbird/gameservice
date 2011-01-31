//
// Copyright (c) Ubisoft Entertainment.
// All rights reserved.
//

#include "stdafx.h"
#include "OSDK/Tracking/TrackingManager.h"
#include <Extensions/OnlineConfig.h>
#if defined(_XBOX) || defined(_XENON)
#include <RendezVous/Core/src/Client/XboxLSP/LSPBackEndServices.h>
#endif
#include "SignIn.h"
#include "Interface.h"
#include "Master.h"

namespace GameService
{

//#define SCE_TICKET_NEWVERSION 1

#if defined(_PS3)
Quazal::qByte* TrackingManager::m_npTicket = NULL;
size_t TrackingManager::m_npTicketSize = 0;

#if defined(SCE_TICKET_NEWVERSION)
SceNpTicketVersion TrackingManager::m_npTicketVersion = {3,0};
#else
SceNpTicketVersion TrackingManager::m_npTicketVersion = {2,1};
#endif

#endif

//------------------------------------------------------------------------
TrackingManager::TrackingManager()
{
    m_bInitialized = FALSE;
    m_iVersionID = 0;
	m_onlineConfigClient = NULL;

    // TODO:
    // Should remove Engine specific code to a unique header file.
#if defined(_XBOX) || defined(_XENON)
    m_onlineConfigID = "21babceda14949d19d1aa27317a0a385";
    m_sandboxAccessKey = "wS4IO73R";
#elif defined(_PS3)
    m_onlineConfigID = "295583213fd84aa6a79291455f558402";
    m_sandboxAccessKey = "4TeVtJ7V";
#endif
}

//------------------------------------------------------------------------
TrackingManager::~TrackingManager()
{
	Shutdown();
}

//------------------------------------------------------------------------
GS_BOOL TrackingManager::Initialize()
{
    if (m_bInitialized)
        return TRUE;

#if defined(_XBOX) || defined(_XENON)
	// Initialize sandbox
	Quazal::Tracking::Create(m_sandboxAccessKey);

	// initialize onlineconfigclient
	m_onlineConfigClient = qNew OnlineConfigClient(m_onlineConfigID);

    FetchConfig();

#elif defined(_PS3)

    if (FALSE == SignIn::GetNPStatus())
    {
        return FALSE;
    }

    // request ticket
	int requestRet = sceNpManagerRequestTicket2(&SignIn::GetNpID(), &m_npTicketVersion, SignIn::GetNpServiceID(), NULL, 0, NULL, 0);
	if (requestRet != 0)
	{
        Master::G()->Log("sceNpManagerRequestTicket failed");
		return FALSE;
	}
#endif

	return TRUE;
}

GS_VOID TrackingManager::FetchConfig()
{
    // will not initialize twice!
    m_bInitialized = TRUE;

    Master::G()->Log("Fetching online config... %s", m_onlineConfigID.CStr());

	CallContext cc;

	// sometimes the FetchConfig will not success when running at the first time
    if (m_onlineConfigClient->FetchConfig(cc) == false)
    {
        Master::G()->Log("Failed to fetch OnlineConfig");
        return;
    }
    cc.Wait();
    if (QFAILED(cc.GetOutcome().GetReturnCode()))
    {
        Master::G()->Log("OnlineConfig result: %d", cc.GetOutcome().GetReturnCode());
    }

#if defined(_XBOX) || defined(_XENON)
	LSPBackEndServices back_service;
	back_service.SetSandboxAccessKey(m_sandboxAccessKey);
#endif

    SendTag_GameStart();
}

// for waiting PS3 got ticket event
GS_VOID TrackingManager::Update()
{
    if (!m_bInitialized)
    {
#if defined(_PS3)
        if (m_npTicketSize > 0)
        {
            Master::G()->Log("Request Ticket complete, ticket size=%d",m_npTicketSize);

            m_bInitialized = TRUE;

            // get np ticket
            m_npTicket = qSpecialNewArray(qByte, m_npTicketSize);
            int ticketRet = sceNpManagerGetTicket(m_npTicket, &m_npTicketSize);

            if (ticketRet != 0)
            {
                Master::G()->Log("sceNpManagerGetTicket failed");
                qSpecialDeleteArray(m_npTicket);
                m_npTicket = NULL;
                m_npTicketSize = 0;
                return;
            }

            Master::G()->Log("sceNpManagerGetTicket returned: %x m_npTicketSize = %d",ticketRet,m_npTicketSize);

            // Initialize sandbox
            Quazal::Tracking::Create(m_sandboxAccessKey);

            // initialize onlineconfigclient
            m_onlineConfigClient = qNew OnlineConfigClient(m_onlineConfigID,m_npTicket,m_npTicketSize);
            
            FetchConfig();
        }
#endif
    }
}

//------------------------------------------------------------------------
GS_VOID TrackingManager::Shutdown()
{
    m_bInitialized = FALSE;

    Quazal::Tracking::Destroy();

    if (m_onlineConfigClient != NULL)
    {
        qDelete m_onlineConfigClient;
        m_onlineConfigClient = NULL;
    }
}

//------------------------------------------------------------------------
GS_BOOL TrackingManager::SendTag(const String& tagName, const String& attributes)
{
	if (!m_bInitialized)
		return FALSE;

	ProtocolCallContext * context = qNew ProtocolCallContext;
	GS_Assert(context != NULL);
	
	if (!Tracking::GetInstance()->SendTag(context, tagName, attributes))
	{
        Master::G()->Log("Send tag failed.");
		return FALSE;
	}

    Master::G()->Log("TrackingManager::SendTag - %s: %s",tagName.CStr(),attributes.CStr());

	return TRUE;
}

GS_BOOL TrackingManager::SendTag_GameStart()
{
    char tracking_attr[128];
	char lang_locale[10];

    GetConsoleLangLocaleAbbr(lang_locale);
#if defined(_XBOX) || defined(_XENON)
    sprintf_s(tracking_attr, 128, "VERSION=%d&LICENSETYPE=%d&LANGUAGE=%s", m_iVersionID, m_iLicenseType, lang_locale);
#elif defined(_PS3)
    sprintf(tracking_attr, "VERSION=%d&LICENSETYPE=%d&LANGUAGE=%s", m_iVersionID, m_iLicenseType, lang_locale);
#endif
    return SendTag("GAME_START", tracking_attr);
}

} // namespace GameService
