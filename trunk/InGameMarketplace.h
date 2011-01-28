// ======================================================================================
// File         : InGameMarketplace.h
// Author       : Li Chen 
// Last Change  : 01/17/2011 | 14:40:17 PM | Monday,January
// Description  : 
// ======================================================================================

#ifndef GAMESERVICE_INGAMEMARKETPLACE_H
#define GAMESERVICE_INGAMEMARKETPLACE_H

#include "Message.h"
#include "Interface.h"

namespace GameService
{

#if defined(_XBOX)
class ContentManager;
class DownloadManager;
class OfferManager;
#elif defined(_PS3)
class InGameBrowsing;
#endif
class MessageMgr;

class InGameMarketplace : public MessageRecipient
{
public:
    InGameMarketplace(MessageMgr* msgMgr);
    virtual ~InGameMarketplace();

    GS_BOOL Initialize();
    GS_VOID Finalize();
	GS_VOID Update();

	GS_BOOL RequestContentList(GS_INT category);
    GS_BOOL RequestProductDetail(GS_INT index);
    GS_BOOL CheckContentListFinished(efd::vector<CMarketplaceItem>& productList);
    GS_BOOL CheckProductDetailFinished(CMarketplaceDetail& detail);
	GS_BOOL	RequestDownloadItems( GS_INT index, ContentInstalledCallBack callBack );
    GS_BOOL Add2WishList(GS_INT index);

	// inherit from MessageHandler
	void MessageResponse(Message* message);

private:
    GS_DWORD m_dwSelectedOffer;
    GS_DWORD m_dwOfferCount;
	GS_INT m_iCurrentCategory;
    GS_BOOL m_bIsCatRequestPending;
    GS_BOOL m_bIsProdRequestPending;
    GS_BOOL m_bDownloadImageStarted, m_bDownloadImageEnded;

	ContentInstalledCallBack m_ContentInstalledCallBack;
#if defined(_XBOX)
    GS_FLOAT m_fContentPingTime;

    ContentManager* m_pContentMgr;
    DownloadManager* m_pDownloadMgr;
    OfferManager* m_pOfferMgr;

#elif defined(_PS3)
	InGameBrowsing* m_pInGameBrowsing;

#endif
};

} // namespace

#endif // GAMESERVICE_INGAMEMARKETPLACE_H
