// ======================================================================================
// File         : InGameBrowsing.h
// Author       : Li Chen 
// Last Change  : 01/17/2011 | 14:40:48 PM | Monday,January
// Description  : 
// ======================================================================================


#pragma once
#ifndef INGAMEBROWSING_H
#define INGAMEBROWSING_H

#ifdef _PS3
#include "Interface.h"

namespace GameService
{

enum EReqType {
    REQ_NONE,
    REQ_CreateSession_Finished,
	REQ_GET_CAT_CON,
	REQ_GET_PRODUCT,
	REQ_MAX
};

typedef struct CReqInfo {
	EReqType reqType;
	char *buf;
	size_t bufLen;
	bool done;
	int errorCode;
	bool canceled; 

	const char *const *caArg1;

	const char *cArg1;
	const char *cArg2;
	const char *cArg3;

	unsigned int iArg1;
	unsigned int iArg2;
	unsigned int iArg3;

	bool Barg1;

	void *userArg;

    GS_BOOL m_bNeedFinishSession;

	uint32_t reqId; 
} CReqInfo;

#define CONTENT_NUM_MAX 18
#define PRODUCT_NUM_MAX 100
#define CATEGORY_NUM_MAX 10 // including sub category
#define CATEGORYID_CHARMAX 64
#define PRODUCTID_CHARMAX 64
#define PRODUCTNAME_CHARMAX 64
#define PRODUCTDESC_CHARMAX 128
#define IMAGEURL_CHARMAX 128

class InGameBrowsing
{
public:
    enum EBrowseMode
    {
        MODE_NONE,
        MODE_CREATE_SESSION,
        MODE_CATEGORY,
        MODE_CONTENT_INFO,
        MODE_SKU_LIST,
        MODE_PRODUCT_INFO,
        MODE_VIEW_CART,
        MODE_CHECKOUT,
        MODE_DL_LIST,
    };
	enum EBrowsePhase
    {
        PHASE_NONE,
        PHASE_SessionCreating,
        PHASE_SessionCreated,
        PHASE_CategoryRetrieved,
        PHASE_ContentInfoRetrieved,
        PHASE_ProductDetailRetrieved,
        PHASE_SKUInfoRetrieved,
		PHASE_CHECKOUT_Processing,
	};
    enum 
    {
        ECATLEVEL_TOP=0,
        ECATLEVEL_SUB,
        ECATLEVEL_MAX // currently only 2 level sub category supported
    };
	typedef struct ProductInfoSruct
	{
        char m_cProductID[PRODUCTID_CHARMAX];
        char m_cProductName[PRODUCTNAME_CHARMAX];
        char m_cProductDesc[PRODUCTDESC_CHARMAX];
        char m_cImageURL[IMAGEURL_CHARMAX];
		char m_cCategoryId[CATEGORYID_CHARMAX];
	} CProductInfo;

	InGameBrowsing();
	~InGameBrowsing();

	GS_BOOL Init();//called after np signed in
	GS_VOID Term();

	GS_VOID Update();

    GS_UINT GetContextID() { return m_iContextID; }
    SceNpCommerce2SessionInfo& GetSessionInfo()  { return m_SessionInfo; }
	sys_event_queue_t GetEventQueueID() { return m_EventQueueID; }
    EBrowseMode GetMode() { return m_eMode; }
    EBrowsePhase GetPhase() { return m_ePhase; }
    GS_VOID SetPhase(GS_INT phase) { m_ePhase = (EBrowsePhase)phase; }
    GS_VOID SetLastUnfinishedRequest(EReqType req) { m_eLastUnfinishedReq = req; }
    EReqType GetLastUnfinishedRequest() { return m_eLastUnfinishedReq; }
    GS_UINT GetSeletedProductIndex() { return m_iSelectedProductIndex; }

    // functionality:
    GS_BOOL CreateSession(GS_BOOL force = FALSE);
    GS_BOOL SendFinishSessionEvent();
    GS_BOOL SendGetCategoryInfoEvent(GS_BOOL needFinishSession);
    GS_INT  GetCategoryContentsEvent(CReqInfo* rp);
    GS_INT ReqCategoryContents(CReqInfo* rp, size_t* fillSize);
    GS_BOOL GetContentInfo(SceNpCommerce2GetCategoryContentsResult& result, CReqInfo* rp);
	GS_BOOL IsCurrentCatMatched(const char* currentCat);
    GS_INT GetProductInfoEvent(CReqInfo* rp);
    GS_INT ReqProductInfo(CReqInfo* rp, size_t* fillSize);
    GS_VOID CheckoutProductDone();
    GS_VOID CheckoutProductFinished();
	GS_VOID DownloadListProductDone();

    // user functions:
    GS_BOOL GetCategory(GS_INT catType);
    GS_UINT GetProductCount() { return m_iProductCount; }
    GS_BOOL GetProductDetail(GS_INT index, GS_BOOL needFinishSession);
    GS_BOOL BuyProduct( GS_INT index, ContentInstalledCallBack callBack );

    GS_BOOL IsGetCategoryFinished(TArray<CMarketplaceItem>& productList);
    GS_BOOL IsGetProductFinished(CMarketplaceDetail& productDetail);
	GS_BOOL GetProductID(GS_INT index, GS_CHAR* productId);

	static GS_VOID CommerceCallback(uint32_t ctx_id, uint32_t subject_id,
									int event, 	int error_code,	void *arg);
	GS_VOID HandleEvent( CReqInfo * r );
	GS_VOID OnDownloadContentFinished();
private:
    GS_UINT m_iContextID;
    SceNpCommerce2SessionInfo m_SessionInfo;
    sys_ppu_thread_t m_ThreadID;
    sys_event_queue_t m_EventQueueID;
    sys_event_port_t  m_EventPortID;
    sys_memory_container_t m_MemoryCID;

    CReqInfo m_ReqInfo;
    EReqType m_eLastUnfinishedReq;

    EBrowseMode m_eMode;
    EBrowsePhase m_ePhase;

    GS_INT m_iCurrentCatType;
    GS_UINT m_iProductCount;
    SceNpCommerce2CategoryInfo m_CategoryInfo;
    char m_CatReqBuf[SCE_NP_COMMERCE2_RECV_BUF_SIZE];

    // copy data locally for tricky and f**ky sce API
    char m_CatCandidateID[CATEGORY_NUM_MAX][CATEGORYID_CHARMAX];
    GS_UINT m_iCurrentCatCandidateIDIndex;
	TArray<GS_UINT> m_vecCatCandidateIDIndex;

    SceNpCommerce2ContentInfo m_ContentInfo[CONTENT_NUM_MAX];

    GS_UINT m_iSelectedProductIndex;

    SceNpCommerce2GameProductInfo m_ProdInfo;
	CProductInfo m_GameProductInfo[PRODUCT_NUM_MAX];
    char m_ProdReqBuf[SCE_NP_COMMERCE2_RECV_BUF_SIZE];

	SceNpCommerce2GameSkuInfo m_GameSkuInfo[PRODUCT_NUM_MAX];

	ContentInstalledCallBack m_ContentInstalledCallBack;
};

// void dl_list_event_handler(uint32_t, uint32_t, int, int, void *);
} // namespace GameService
#endif

#endif // INGAMEBROWSING_H
