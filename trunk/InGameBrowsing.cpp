// ======================================================================================
// File         : InGameBrowsing.cpp
// Author       : Li Chen 
// Last Change  : 01/17/2011 | 14:40:53 PM | Monday,January
// Description  : 
// ======================================================================================

#include "stdafx.h"
#ifdef _PS3
#include "SignIn.h"
#include "Master.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <cell/ssl.h>
#include <np/commerce2.h>
#include <netex/net.h>
#include <netex/libnetctl.h>
#include <cell/sysmodule.h>
#include <sys/ppu_thread.h>
#include <sys/event.h>
#include <np.h>
#include "InGameBrowsing.h"


#define HTTP_POOL_SIZE      (512 * 1024)
#define HTTP_COOKIE_POOL_SIZE      (128 * 1024)
#define SSL_POOL_SIZE       (300 * 1024)
#define COMMERCE2_TPL 1001
#define COMMERCE2_STACKSIZE (1024 * 16)
#define EVENT_QUEUE_SIZE    (16)

namespace GameService
{
static enum {
	E_PHASE_NONE,
	E_PHASE_NETSTART_DONE,
	E_PHASE_NETSTART_UNLOADED,
	E_PHASE_CREATESESSION_DONE,
	E_PHASE_CHECKOUT_FINISHING,
	E_PHASE_CHECKOUT_FINISHED,
	E_PHASE_REDOWNLOAD_FINISHED
} phase = E_PHASE_NONE;

// ======================================================== 
// InGameBrowsing Routine functions:
// Constructor, destructor, Init, Term, ThreadInit, ThreadTerm
// ======================================================== 
InGameBrowsing::InGameBrowsing()
: m_eMode(MODE_NONE), m_iContextID(0), m_ePhase(PHASE_NONE), m_eLastUnfinishedReq(REQ_NONE), m_ContentInstalledCallBack( NULL )
{
    // m_MemoryCID = SYS_MEMORY_CONTAINER_ID_INVALID;
}

InGameBrowsing::~InGameBrowsing()
{
    Term();
}

GS_BOOL InGameBrowsing::Init()
{
	int ret;

    ret = cellSysmoduleLoadModule(CELL_SYSMODULE_SYSUTIL_NP_COMMERCE2);
    if (ret < 0)
    {
        Master::G()->Log("cellSysmoduleLoadModule(CELL_SYSMODULE_SYSUTIL_NP_COMMERCE2) failed. ret = 0x%x", ret);
        return FALSE;
    }

    ret = sceNpCommerce2Init();
    if(ret < 0 && ret != SCE_NP_COMMERCE2_ERROR_ALREADY_INITIALIZED){
        Master::G()->Log("sceNpCommerce2Init() failed. ret = 0x%x", ret);
        return FALSE;
    }

	ret = sceNpCommerce2CreateCtx(SCE_NP_COMMERCE2_VERSION, &SignIn::GetNpID(),
	    CommerceCallback, NULL, &m_iContextID);
	if(ret < 0){
		Master::G()->Log("sceNpCommerce2CreateCtx() failed. ret = 0x%x", ret);
        return FALSE;
	}

    return TRUE;
}

void InGameBrowsing::Term()
{
	int ret;

    if (m_iContextID != 0)
    {
        ret = sceNpCommerce2DestroyCtx(m_iContextID);
        if (ret < 0) {
            Master::G()->Log("sceNpCommerce2DestroyCtx() failed (0x%x)", ret);
        }
        m_iContextID = 0;
    }

	ret = sceNpCommerce2Term();
	if (ret < 0) {
		Master::G()->Log("sceNpCommerce2Term() failed (0x%x)", ret);
	}

	ret = cellSysmoduleUnloadModule(CELL_SYSMODULE_SYSUTIL_NP_COMMERCE2);
	if (ret < 0) {
		Master::G()->Log("cellSysmoduleUnloadModule(CELL_SYSMODULE_SYSUTIL_NP_COMMERCE2) failed (0x%x)", ret);
	}

    // if(m_MemoryCID != SYS_MEMORY_CONTAINER_ID_INVALID)
    //     sys_memory_container_destroy(m_MemoryCID);
    // m_MemoryCID = SYS_MEMORY_CONTAINER_ID_INVALID;

}

// ======================================================== 
// CreateSession
// ======================================================== 
GS_BOOL InGameBrowsing::CreateSession(GS_BOOL force)
{
    if (!force && m_ePhase > PHASE_NONE)
        return FALSE;

    m_ePhase = PHASE_NONE;
    m_eMode = MODE_CREATE_SESSION;
    m_iSelectedProductIndex = 0;

	int ret = 0;

	ret = sceNpCommerce2CreateSessionStart(m_iContextID);
	if(ret < 0)
    {
		Master::G()->Log("sceNpCommerce2CreateSessionStart() failed. ret = 0x%x", ret);
        return FALSE;
	}

	return TRUE;
}

// ======================================================== 
// Notify session finished
// ======================================================== 
GS_BOOL InGameBrowsing::SendFinishSessionEvent()
{
	int ret = 0;

	memset(&m_ReqInfo, 0, sizeof(m_ReqInfo));
	m_ReqInfo.reqType = REQ_NONE;
	m_ReqInfo.buf = NULL;
	m_ReqInfo.bufLen = 0;
    m_ReqInfo.iArg1 = 0; // startPosition
    m_ReqInfo.iArg2 = 0; // maxCountOfResults
    m_ReqInfo.cArg1 = SignIn::GetNpServiceID(); // service ID
    // m_ReqInfo.userArg = (void *)d;
    m_ReqInfo.m_bNeedFinishSession = TRUE;
    m_ReqInfo.errorCode = 0;
    m_ReqInfo.canceled = false;

    // ask commerce2_thread to run synchronized sceNpCommerce2CreateSessionFinish
	//ret = sys_event_port_send(m_EventPortID, (uint64_t)(uintptr_t)&m_ReqInfo, NULL, NULL);
	//if(ret < 0){
 //       Master::G()->Log("sys_event_port_send() failed. ret = 0x%x", ret);
 //       return FALSE;
	//}
	HandleEvent( &m_ReqInfo );
    return TRUE;
}

// ======================================================== 
// Category retrieve functions:
// SendGetCategoryInfoEvent
// For send event to commerce2_thread
// ======================================================== 
GS_BOOL InGameBrowsing::SendGetCategoryInfoEvent(GS_BOOL needFinishSession)
{
	int ret = 0;

	memset(m_CatReqBuf, 0, sizeof(m_CatReqBuf));

	memset(&m_ReqInfo, 0, sizeof(m_ReqInfo));
	m_ReqInfo.reqType = REQ_GET_CAT_CON;
	m_ReqInfo.buf = m_CatReqBuf;
	m_ReqInfo.bufLen = sizeof(m_CatReqBuf);
    m_ReqInfo.iArg1 = 0; // startPosition
    m_ReqInfo.iArg2 = CONTENT_NUM_MAX; // maxCountOfResults
    m_ReqInfo.cArg1 = SignIn::GetNpServiceID(); // service ID
    // m_ReqInfo.userArg = (void *)d;
    m_ReqInfo.m_bNeedFinishSession = needFinishSession;
    m_ReqInfo.errorCode = 0;
    m_ReqInfo.canceled = false;

    // ask commerce2_thread to run synchronized GetCategoryContentsEvent
	//ret = sys_event_port_send(m_EventPortID, (uint64_t)(uintptr_t)&m_ReqInfo, NULL, NULL);
	//if(ret < 0){
 //       Master::G()->Log("sys_event_port_send() failed. ret = 0x%x", ret);
 //       return FALSE;
	//}
	HandleEvent( &m_ReqInfo );
    return TRUE;
}

// ======================================================== 
// Category retrieve functions:
// GetCategoryContentsEvent
// ReqCategoryContents
// Getting Category info
// ======================================================== 
GS_INT InGameBrowsing::GetCategoryContentsEvent(CReqInfo* rp)
{
    m_eMode = MODE_CATEGORY;

    SceNpCommerce2GetCategoryContentsResult result;
	size_t recvdSize;
	int ret = 0;

	ret = ReqCategoryContents(rp, &recvdSize);
    sceNpCommerce2DestroyReq(rp->reqId);
    rp->reqId = 0;
	if(ret < 0)
		return ret;

	ret = sceNpCommerce2InitGetCategoryContentsResult(&result, rp->buf, recvdSize);
	if(ret < 0){
		Master::G()->Log("sceNpCommerce2InitGetCategoryContentsResult() failed. ret = 0x%x", ret);
		return ret;
	}

	ret = sceNpCommerce2GetCategoryInfo(&result, &m_CategoryInfo);
	if(ret < 0){
		Master::G()->Log("sceNpCommerce2GetCategoryInfo() failed. ret = 0x%x", ret);
		return -1;
	}

    m_ePhase = PHASE_CategoryRetrieved;

    // get all contents info
    m_eMode = MODE_CONTENT_INFO;

    if(FALSE == GetContentInfo(result,rp))
	{
		return -1;
	}

    int _ret = 0;
    while(!m_vecCatCandidateIDIndex.IsEmpty() && _ret >= 0)
    {
        rp->cArg1 = m_CatCandidateID[m_vecCatCandidateIDIndex(0)];
        m_vecCatCandidateIDIndex.Remove(0);

        _ret = GetCategoryContentsEvent(rp);
    }

    m_ePhase = PHASE_ContentInfoRetrieved;

	return _ret < 0 ? _ret : 0;
}
GS_INT InGameBrowsing::ReqCategoryContents(CReqInfo* rp, size_t* fillSize)
{
	int ret = 0;

	ret = sceNpCommerce2GetCategoryContentsCreateReq(m_iContextID, &rp->reqId);
	if(ret < 0){
		Master::G()->Log("sceNpCommerce2GetCategoryContentsCreateReq() failed. ret = 0x%x", ret);
        return ret;
	}
	Master::G()->Log("%s: reqId = %u", __FUNCTION__, rp->reqId);

	ret = sceNpCommerce2GetCategoryContentsStart(rp->reqId, rp->cArg1, rp->iArg1, rp->iArg2);
	if(ret < 0){
		Master::G()->Log("sceNpCommerce2GetCategoryContentsStart() failed. ret = 0x%x", ret);
        return ret;
	}

	ret = sceNpCommerce2GetCategoryContentsGetResult(rp->reqId, rp->buf, rp->bufLen, fillSize);
	if(ret < 0){
		Master::G()->Log("sceNpCommerce2GetCategoryContentsGetResult() failed. ret = 0x%x", ret);
        return ret;
	}
	Master::G()->Log("%s: GetCategoryContents Result len = %u", __FUNCTION__,
	    *fillSize);

    return ret;
}

// ======================================================== 
// GetContentInfo
// Get content info from category
// ======================================================== 
GS_BOOL InGameBrowsing::IsCurrentCatMatched(const char* currentCat)
{
    for (GS_INT i=0; i<32; i++)
    {
        GS_INT cat = (1<<i);
        if ((m_iCurrentCatType & cat) != 0)
        {
            char category_name[32];
            switch(cat)
            {
            case GS_EDLCCAT_Song:
                strcpy(category_name,"Songs");
                break;
            case GS_EDLCCAT_Gear:
                strcpy(category_name,"Gears");
				break;
			case GS_EDLCCAT_DemoSong:
				strcpy( category_name, "DLCDemos" ); 
                break;
            }

            if(strncmp(currentCat, category_name, 32) == 0)
            {
                return TRUE;
            }
        }
    }

    return FALSE;
}
GS_BOOL InGameBrowsing::GetContentInfo(SceNpCommerce2GetCategoryContentsResult& result, CReqInfo* rp)
{
    for (GS_INT index=0; index<result.rangeOfContents.count; index++)
    {
        int ret = 0;

        ret = sceNpCommerce2GetContentInfo(&result, index, &m_ContentInfo[index]);
        if(ret < 0){
            Master::G()->Log("sceNpCommerce2GetContentInfo() failed. ret = 0x%x", ret);
            return FALSE;
        }

        switch(m_ContentInfo[index].contentType)
        {
        case SCE_NP_COMMERCE2_CONTENT_TYPE_CATEGORY:
            // Breadth-First-Search 
            ret = sceNpCommerce2GetCategoryInfoFromContentInfo(&m_ContentInfo[index], &m_CategoryInfo);
            if(ret < 0){
                Master::G()->Log("sceNpCommerce2GetCategoryInfoFromContentInfo() failed. ret = 0x%x", ret);
                return FALSE;
            }

            if (IsCurrentCatMatched(m_CategoryInfo.categoryName))
            {
                strcpy(m_CatCandidateID[m_iCurrentCatCandidateIDIndex],m_CategoryInfo.categoryId);
                m_vecCatCandidateIDIndex.AddItem(m_iCurrentCatCandidateIDIndex);

                m_iCurrentCatCandidateIDIndex++;
            }

            break;

        case SCE_NP_COMMERCE2_CONTENT_TYPE_PRODUCT:
            if (m_iProductCount >= PRODUCT_NUM_MAX)
            {
                // TODO:

                Master::G()->Log("Product num over max: %d!", m_iProductCount);
                return FALSE;
            }

            strcpy(m_GameProductInfo[m_iProductCount].m_cCategoryId, m_CategoryInfo.categoryId);
            ret = sceNpCommerce2GetGameProductInfoFromContentInfo(&m_ContentInfo[index], &m_ProdInfo);
            if(ret < 0){
                Master::G()->Log("sceNpCommerce2GetGameProductInfoFromContentInfo() failed. ret = 0x%x", ret);
                return FALSE;
            }

            strcpy(m_GameProductInfo[m_iProductCount].m_cProductID, m_ProdInfo.productId);
            strcpy(m_GameProductInfo[m_iProductCount].m_cProductName, m_ProdInfo.productName);
            strcpy(m_GameProductInfo[m_iProductCount].m_cProductDesc, m_ProdInfo.productShortDescription);
            strcpy(m_GameProductInfo[m_iProductCount].m_cImageURL, m_ProdInfo.imageUrl);
            ret = sceNpCommerce2GetGameSkuInfoFromGameProductInfo(&m_ProdInfo, 0, &m_GameSkuInfo[m_iProductCount]);
            if(ret < 0){
                Master::G()->Log("sceNpCommerce2GetGameSkuInfoFromGameProductInfo() failed. ret = 0x%x", ret);
                return FALSE;
            }

            //if(m_ProdInfo.countOfSku != 1)
            //{
            //    m_GameSkuInfo[m_iProductCount].purchasabilityFlag = SCE_NP_COMMERCE2_SKU_PURCHASABILITY_FLAG_ON;
            //}

            m_iProductCount++;
            break;
        }

    }
    return TRUE;
}

// ======================================================== 
// ProductInfo retrive functions:
// GetProductInfoEvent, ReqProductInfo
// For retrieve Product detail and SKU info
// ======================================================== 
GS_INT InGameBrowsing::GetProductInfoEvent(CReqInfo* rp)
{
	size_t recvdSize;
	int ret = 0;
    SceNpCommerce2GetProductInfoResult result;

	ret = ReqProductInfo(rp, &recvdSize);
	sceNpCommerce2DestroyReq(rp->reqId);
	rp->reqId = 0;
	if(ret < 0)
		return ret;

	ret = sceNpCommerce2InitGetProductInfoResult(&result, rp->buf, recvdSize);
	if(ret < 0){
		Master::G()->Log("sceNpCommerce2InitGetProductInfoResult() failed. ret = 0x%x", ret);
		return ret;
	}

	ret = sceNpCommerce2GetGameProductInfo(&result, &m_ProdInfo);
	if(ret < 0){
		Master::G()->Log("sceNpCommerce2GetProductInfo() failed. ret = 0x%x",
		    ret);
		return ret;
	}

    strcpy(m_GameProductInfo[m_iSelectedProductIndex].m_cProductID, m_ProdInfo.productId);
    strcpy(m_GameProductInfo[m_iSelectedProductIndex].m_cProductName, m_ProdInfo.productName);
    strcpy(m_GameProductInfo[m_iSelectedProductIndex].m_cProductDesc, m_ProdInfo.productLongDescription);
    strcpy(m_GameProductInfo[m_iSelectedProductIndex].m_cImageURL, m_ProdInfo.imageUrl);

    m_ePhase = PHASE_ProductDetailRetrieved;

    // get SKU info
    for (GS_INT i=0; i<m_ProdInfo.countOfSku; i++)
    {
        int ret = 0;

        ret = sceNpCommerce2GetGameSkuInfoFromGameProductInfo(
            &m_ProdInfo, i, &m_GameSkuInfo[m_iSelectedProductIndex]);
        if(ret < 0){
            Master::G()->Log("sceNpCommerce2GetGameSkuInfoFromGameProductInfo() failed. ret = 0x%x", ret);
            return ret;
        }

        // if(name != NULL)
        //     *name = m_GameSkuInfo[m_iSelectedProductIndex].skuName;
        // if(targetIdPtr != NULL)
        //     *targetIdPtr = m_GameSkuInfo[m_iSelectedProductIndex].skuId;
        // if(price != NULL)
        //     *price = m_GameSkuInfo[m_iSelectedProductIndex].price;
        // if(purchasabilityFlag != NULL){
        //     *purchasabilityFlag = m_GameSkuInfo[m_iSelectedProductIndex].purchasabilityFlag;
        // }
    }

    m_ePhase = PHASE_SKUInfoRetrieved;
}
GS_INT InGameBrowsing::ReqProductInfo(CReqInfo* rp, size_t* fillSize)
{
	int ret = 0;

	ret = sceNpCommerce2GetProductInfoCreateReq(m_iContextID, &rp->reqId);
	if(ret < 0){
		Master::G()->Log("sceNpCommerce2GetProductInfoCreateReq() failed. ret = 0x%x", ret);
		return ret;
	}
	Master::G()->Log("%s: reqId = %u", __FUNCTION__, rp->reqId);

	ret = sceNpCommerce2GetProductInfoStart(rp->reqId, rp->cArg1, rp->cArg2);
	if(ret < 0){
		Master::G()->Log("sceNpCommerce2GetProductInfoStart() failed. ret = 0x%x", ret);
		return ret;
	}

	ret = sceNpCommerce2GetProductInfoGetResult(rp->reqId, rp->buf, rp->bufLen, fillSize);
	if(ret < 0){
		Master::G()->Log("sceNpCommerce2GetProductInfoGetResult() failed. ret = 0x%x", ret);
		return ret;
	}
	Master::G()->Log("%s: GetProductInfo Result len = %u", __FUNCTION__,
	    *fillSize);

    return ret;
}


// ======================================================== 
// Checkout product functions 
// ======================================================== 
GS_VOID InGameBrowsing::CheckoutProductDone()
{
    sceNpCommerce2DoCheckoutFinishAsync(m_iContextID);
}
GS_VOID InGameBrowsing::DownloadListProductDone()
{
    sceNpCommerce2DoDlListFinishAsync(m_iContextID);
}
GS_VOID InGameBrowsing::CheckoutProductFinished()
{
    if(m_MemoryCID != SYS_MEMORY_CONTAINER_ID_INVALID)
        sys_memory_container_destroy(m_MemoryCID);
    m_MemoryCID = SYS_MEMORY_CONTAINER_ID_INVALID;
}

// ======================================================== 
// User function:
// GetCategory()
// First step to get top level category
// ======================================================== 
GS_BOOL InGameBrowsing::GetCategory(GS_INT catType)
{
    m_iCurrentCatType = catType;
    m_iProductCount = 0;
    m_vecCatCandidateIDIndex.Empty();	
    m_iCurrentCatCandidateIDIndex = 0;

    if (PHASE_NONE == m_ePhase)
    {
        SetLastUnfinishedRequest(REQ_GET_CAT_CON);
        CreateSession();
        return TRUE;
    }

    // a session is creating
    if (m_ePhase < PHASE_SessionCreated)
        return FALSE;

    return SendGetCategoryInfoEvent(FALSE);
}

// ======================================================== 
// User function:
// GetProductDetail()
// Second step to get one product detail
// ======================================================== 
GS_BOOL InGameBrowsing::GetProductDetail(GS_INT index, GS_BOOL needFinishSession)
{
    // TODO: handle sub category case if the m_ContentInfo[index] was Category
    // GS_Assert(m_ContentInfo[index].contentType == SCE_NP_COMMERCE2_CONTENT_TYPE_PRODUCT);

    // make sure m_ContentInfo has been retrieved
    if (m_ePhase < PHASE_ContentInfoRetrieved)
        return FALSE;

	memset(&m_ReqInfo, 0, sizeof(m_ReqInfo));
	m_ReqInfo.reqType = REQ_GET_PRODUCT;
	m_ReqInfo.buf = m_ProdReqBuf;
	m_ReqInfo.bufLen = sizeof(m_ProdReqBuf);
	m_ReqInfo.cArg1 = m_GameProductInfo[index].m_cCategoryId;
	m_ReqInfo.cArg2 = m_GameProductInfo[index].m_cProductID;
    m_ReqInfo.m_bNeedFinishSession = needFinishSession;
    m_ReqInfo.errorCode = 0;
    m_ReqInfo.canceled = false;

    m_iSelectedProductIndex = index;

	//int ret = sys_event_port_send(m_EventPortID, (uint64_t)(uintptr_t)&m_ReqInfo, NULL, NULL);
	//if(ret < 0){
	//	Master::G()->Log("sys_event_port_send() failed. ret = 0x%x", ret);
	//	return FALSE;
	//}
	HandleEvent( &m_ReqInfo );

    return TRUE;
}

// ======================================================== 
// UI interface to check async tasks finished
// ======================================================== 
GS_BOOL InGameBrowsing::IsGetCategoryFinished(TArray<CMarketplaceItem>& productList)
{
    if (m_ePhase >= PHASE_ContentInfoRetrieved)
    {
        for (GS_INT i=0; i<m_iProductCount; i++)
        {
            CMarketplaceItem tmp;
            tmp.m_iIndex = i;
            strcpy(tmp.m_strName, m_GameProductInfo[i].m_cProductName);
            productList.AddItem(tmp);
        }
        return TRUE;
    }
    return FALSE;
}
GS_BOOL InGameBrowsing::IsGetProductFinished(CMarketplaceDetail& productDetail)
{
    if (m_ePhase >= PHASE_ProductDetailRetrieved)
    {
        strcpy(productDetail.m_strName, m_GameProductInfo[m_iSelectedProductIndex].m_cProductName);
        strcpy(productDetail.m_strDesc, m_GameProductInfo[m_iSelectedProductIndex].m_cProductDesc);
        strcpy(productDetail.m_strImagePath, m_GameProductInfo[m_iSelectedProductIndex].m_cImageURL);
        return TRUE;
    }

    return FALSE;
}
// for wishlist:
GS_BOOL InGameBrowsing::GetProductID(GS_INT index, GS_CHAR* productId)
{
    if (m_ePhase >= PHASE_ProductDetailRetrieved)
    {
        strcpy(productId, m_GameProductInfo[index].m_cProductID);
        return TRUE;
    }

    return FALSE;
}

// ======================================================== 
// User function:
// BuyProduct()
// Final step to buy one product
// ======================================================== 
GS_BOOL InGameBrowsing::BuyProduct( GS_INT index, ContentInstalledCallBack callBack )
{
	int ret = 0;

	m_ContentInstalledCallBack = callBack;

	if(m_MemoryCID != SYS_MEMORY_CONTAINER_ID_INVALID)
		sys_memory_container_destroy(m_MemoryCID);
	m_MemoryCID = SYS_MEMORY_CONTAINER_ID_INVALID;

	if ( m_GameSkuInfo[index].purchasabilityFlag == SCE_NP_COMMERCE2_SKU_PURCHASABILITY_FLAG_ON )
	{
		ret = sys_memory_container_create(&m_MemoryCID,
			SCE_NP_COMMERCE2_DO_CHECKOUT_MEMORY_CONTAINER_SIZE);
		if(ret < 0){
			Master::G()->Log("sys_memory_container_create() failed. ret = 0x%x\n", ret);
			if(m_MemoryCID != SYS_MEMORY_CONTAINER_ID_INVALID)
				sys_memory_container_destroy(m_MemoryCID);
			return FALSE;
		}

		// currently support buy 1 product at one time
		//ret = sceNpCommerce2CreateSessionStart( m_iContextID );

		ret = sceNpCommerce2DoCheckoutStartAsync(m_iContextID, &m_GameSkuInfo[index].skuId, 1, m_MemoryCID);
		if(ret < 0){
			Master::G()->Log("sceNpCommerce2DoCheckoutStartAsync() failed. ret = 0x%x\n", ret);
			if(m_MemoryCID != SYS_MEMORY_CONTAINER_ID_INVALID)
				sys_memory_container_destroy(m_MemoryCID);
			return FALSE;
		}
	}
	else 
	{
		ret = sys_memory_container_create(&m_MemoryCID,
			SCE_NP_COMMERCE2_DO_DL_LIST_MEMORY_CONTAINER_SIZE);
		if(ret < 0){
			Master::G()->Log("sys_memory_container_create() failed. ret = 0x%x\n", ret);
			if(m_MemoryCID != SYS_MEMORY_CONTAINER_ID_INVALID)
				sys_memory_container_destroy(m_MemoryCID);
			return FALSE;
		}

		//start redownload
		ret = sceNpCommerce2DoDlListStartAsync(m_iContextID, "UP0001-NPXX00804_00", &m_GameSkuInfo[index].skuId, 1, m_MemoryCID);
		if(ret < 0){
			Master::G()->Log("sceNpCommerce2DoDlListStartAsync() failed. ret = 0x%x\n", ret);
			if(m_MemoryCID != SYS_MEMORY_CONTAINER_ID_INVALID)
				sys_memory_container_destroy(m_MemoryCID);
			m_MemoryCID = SYS_MEMORY_CONTAINER_ID_INVALID;
			return FALSE;
		}
	}

	return TRUE;
}


GS_VOID InGameBrowsing::CommerceCallback(uint32_t ctx_id, uint32_t subject_id,
									int event, 	int error_code,	void *arg)
{
	(void)ctx_id;
	(void)subject_id;
	(void)error_code;
	(void)arg;

	switch(event){
	case SCE_NP_COMMERCE2_EVENT_CREATE_SESSION_DONE:
		phase = E_PHASE_CREATESESSION_DONE;
		break;

	case SCE_NP_COMMERCE2_EVENT_REQUEST_ERROR:
	case SCE_NP_COMMERCE2_EVENT_DO_CHECKOUT_SUCCESS:
	case SCE_NP_COMMERCE2_EVENT_DO_CHECKOUT_BACK:
		phase = E_PHASE_CHECKOUT_FINISHING;
		break;
	case SCE_NP_COMMERCE2_EVENT_DO_CHECKOUT_FINISHED:
		phase = E_PHASE_CHECKOUT_FINISHED;
		break;
	case SCE_NP_COMMERCE2_EVENT_DO_DL_LIST_SUCCESS:
		phase = E_PHASE_REDOWNLOAD_FINISHED;
		break;
	case SCE_NP_COMMERCE2_EVENT_DO_CHECKOUT_STARTED:
	default:
		break;
	}
}

GS_VOID InGameBrowsing::Update()
{
	int ret = 0;
	switch (phase)
	{
	case  E_PHASE_NETSTART_DONE:
		//OnFinishNetstartDialog();
		phase = E_PHASE_NONE;
		break;
	case  E_PHASE_NETSTART_UNLOADED:
		//NPInitAndStartSessionCreation();
		phase = E_PHASE_NONE;
		break;
	case  E_PHASE_CREATESESSION_DONE:
		ret = sceNpCommerce2CreateSessionFinish(m_iContextID, &m_SessionInfo);
		if (ret != CELL_OK)	{
			Master::G()->Log( "sceNpCommerce2CreateSessionFinish() failed. ret = 0x%x\n", ret);
		}

		SetPhase(InGameBrowsing::PHASE_SessionCreated);
		{
			switch(GetLastUnfinishedRequest())
			{
			case REQ_GET_CAT_CON:
				SendGetCategoryInfoEvent(TRUE);
				break;
			case REQ_GET_PRODUCT:
				GetProductDetail(m_iSelectedProductIndex, TRUE);
				break;
			}
		}
		phase = E_PHASE_NONE;
		break;
	case E_PHASE_CHECKOUT_FINISHING:
		sceNpCommerce2DoCheckoutFinishAsync(m_iContextID);
		phase = E_PHASE_NONE;
		break;
	case E_PHASE_CHECKOUT_FINISHED:
		if(m_MemoryCID != SYS_MEMORY_CONTAINER_ID_INVALID)		sys_memory_container_destroy(m_MemoryCID);
		m_MemoryCID = SYS_MEMORY_CONTAINER_ID_INVALID;
		OnDownloadContentFinished();
		phase = E_PHASE_NONE;
		break;
	case E_PHASE_REDOWNLOAD_FINISHED:
		sceNpCommerce2DoDlListFinishAsync(m_iContextID);
		if(m_MemoryCID != SYS_MEMORY_CONTAINER_ID_INVALID)		sys_memory_container_destroy(m_MemoryCID);
		m_MemoryCID = SYS_MEMORY_CONTAINER_ID_INVALID;
		OnDownloadContentFinished();
		phase = E_PHASE_NONE;
		break;
	default:
		break;
	}
}

GS_VOID InGameBrowsing::OnDownloadContentFinished()
{
	if ( m_ContentInstalledCallBack )
		m_ContentInstalledCallBack();
}

GS_VOID InGameBrowsing::HandleEvent( CReqInfo * r )
{
	if(true == r->canceled){
		r->done = true;
		return;
	}

	if (TRUE == r->m_bNeedFinishSession)
	{
		sceNpCommerce2CreateSessionFinish(m_iContextID, &m_SessionInfo);
	}

	switch(r->reqType){
		case REQ_GET_CAT_CON:
			r->errorCode = GetCategoryContentsEvent(r);
			break;
		case REQ_GET_PRODUCT:
			r->errorCode = GetProductInfoEvent(r);
			break;
		default:
			Master::G()->Log("%s: invalid reqType (%u)", __FUNCTION__, r->reqType);
			break;
	}

	if (SCE_NP_COMMERCE2_SERVER_ERROR_SESSION_EXPIRED == r->errorCode)
	{
		// force create a new session again
		SetLastUnfinishedRequest(r->reqType);
		CreateSession(TRUE);
	}
	// no error
	else
	{
		SetLastUnfinishedRequest(REQ_NONE);
	}

	r->done = true;
}

}
#endif
