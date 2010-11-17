//
// Copyright (c) Ubisoft Entertainment.
// All rights reserved.
//

#ifndef _TRACKING_MANAGER_H_
#define _TRACKING_MANAGER_H_

#include <RendezVous.h>
#include <Services/Tracking.h>
#include <OnlineCore/src/Platform/Core/PlatformDecl.h>
#include <OnlineCore/src/Platform/Core/String.h>

namespace Quazal
{
    class OnlineConfigClient;
}

namespace GameService
{

class TrackingManager
{
public:
    TrackingManager();
    virtual ~TrackingManager();

	/// Initialize the manager.
	GS_BOOL Initialize();
    GS_VOID FetchConfig();
    GS_BOOL IsInitialzed() 
    {
        return m_bInitialized;
    }
    GS_VOID SetVersionID(int versionId)
    {
        m_iVersionID = versionId;
    }
    GS_VOID SetLicenseType(int type)
    {
        m_iLicenseType = type;
    }

    // For PS3 getting np ticket
    GS_VOID Update();

	/// Shutdown this manager.
	GS_VOID Shutdown();

	/// Send a tag.
	GS_BOOL SendTag(const String& tagName, const String& attributes);
    GS_BOOL SendTag_GameStart();

#if defined(_PS3)
    GS_VOID SetNpTicketSize(GS_INT size)
    {
        m_npTicketSize = size;
    }
#endif

private:
    Quazal::String   m_onlineConfigID;
    Quazal::String   m_sandboxAccessKey;
#if defined(_PS3)
    static Quazal::qByte*   m_npTicket;
    static size_t           m_npTicketSize;
    static char*            m_npServiceID;
    static SceNpTicketVersion m_npTicketVersion;
#endif
    Quazal::OnlineConfigClient* m_onlineConfigClient;
    GS_BOOL m_bInitialized;
    GS_INT m_iVersionID;
    GS_INT m_iLicenseType;

};

typedef TrackingManager * TrackingManagerPtr;

} // namespace GameService

#endif //_TRACKING_MANAGER_H_
