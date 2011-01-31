// ======================================================================================
// File         : InGameMarketplace.cpp
// Author       : Li Chen 
// Last Change  : 01/17/2011 | 14:40:23 PM | Monday,January
// Description  : 
// ======================================================================================

#include "stdafx.h"
#include "Master.h"
#include "SignIn.h"
#include "Task.h"
#include "HttpSrv.h"
#include "LogFile.h"
#include "Interface.h"

#if defined(_XBOX)
#include "ContentManager.h"
#include "InGameDownloadManager.h"
#include "InGameOfferManager.h"
#elif defined(_PS3)
#include "InGameBrowsing.h"
#endif

#include "InGameMarketplace.h"

namespace GameService
{

InGameMarketplace::InGameMarketplace(MessageMgr* msgMgr)
: m_iCurrentCategory(GS_EDLCCAT_ALL), m_ContentInstalledCallBack( NULL )
    , m_bDownloadImageStarted(FALSE), m_bDownloadImageEnded(FALSE)
    , m_bIsCatRequestPending(FALSE), m_bIsProdRequestPending(FALSE)
{	
    if (msgMgr)
    {
		msgMgr->Register(EMessage_InternalTaskDone,this);
    }

#if defined(_XBOX)
    if (msgMgr)
	{
		msgMgr->Register(EMessage_ContentInstalled,this);
		msgMgr->Register(EMessage_SignInChanged,this);
	}

    m_pContentMgr = GS_NEW ContentManager(msgMgr);
    m_pDownloadMgr = GS_NEW DownloadManager();
    m_pOfferMgr = GS_NEW OfferManager(msgMgr);

    GS_Assert(m_pContentMgr && m_pDownloadMgr && m_pOfferMgr);

#elif defined(_PS3)
	m_pInGameBrowsing = GS_NEW InGameBrowsing();

	GS_Assert(m_pInGameBrowsing != NULL);
#endif

	if (!Initialize())
    {
		Finalize();
    }
}

InGameMarketplace::~InGameMarketplace()
{
#if defined(_XBOX)
	if (m_pContentMgr)
    {
        GS_DELETE m_pContentMgr;
        m_pContentMgr = NULL;
    }
    if (m_pDownloadMgr)
    {
        GS_DELETE m_pDownloadMgr;
        m_pDownloadMgr = NULL;
    }
    if (m_pOfferMgr)
    {
        GS_DELETE m_pOfferMgr;
        m_pOfferMgr = NULL;
    }
#elif defined(_PS3)
	if (m_pInGameBrowsing)
	{
		GS_DELETE m_pInGameBrowsing;
		m_pInGameBrowsing = NULL;
	}
#endif
}

GS_BOOL InGameMarketplace::Initialize()
{
#if defined(_XBOX)
    if (m_pContentMgr)
        m_pContentMgr->Initialize();

    if (m_pOfferMgr)
        m_pOfferMgr->Initialize();

#elif defined(_PS3)
	if (m_pInGameBrowsing)
		m_pInGameBrowsing->Init();
#endif

    m_dwSelectedOffer = ( GS_DWORD )-1;

    return TRUE;
}

GS_VOID InGameMarketplace::Finalize()
{
#if defined(_XBOX)
    if (m_pContentMgr)
        m_pContentMgr->Finalize();

    if (m_pOfferMgr)
        m_pOfferMgr->Cleanup();

#elif defined(_PS3)
	if (m_pInGameBrowsing)
		m_pInGameBrowsing->Term();
#endif
}

GS_VOID InGameMarketplace::Update()
{
#if defined(_XBOX)
    if (m_pContentMgr)
        m_pContentMgr->Update();

    if (m_pDownloadMgr)
        m_pDownloadMgr->Update();

    if (m_pOfferMgr)
        m_pOfferMgr->Update();

#elif defined(_PS3)
	if (m_pInGameBrowsing)
		m_pInGameBrowsing->Update();
#endif

    // +LCTEST
    static int s_request = 0;
    if (s_request)
        RequestContentList(GS_EDLCCAT_Song | GS_EDLCCAT_Gear);
    // -LCTEST
}

GS_BOOL InGameMarketplace::RequestContentList(GS_INT category)
{
	m_iCurrentCategory = category;
    m_bIsCatRequestPending = TRUE;

#if defined(_XBOX)
    if (m_pOfferMgr)
    {
        return m_pOfferMgr->EnumerateOffers(category);
    }
#elif defined(_PS3)
    if (m_pInGameBrowsing)
    {
        return m_pInGameBrowsing->GetCategory(category);
    }
#endif

    return FALSE;
}

GS_BOOL InGameMarketplace::RequestProductDetail(GS_INT index)
{
    m_dwSelectedOffer = index;
    m_bIsProdRequestPending = TRUE;
    m_bDownloadImageStarted = m_bDownloadImageEnded = FALSE;

#if defined(_XBOX)
    return TRUE;
#elif defined(_PS3)
    if (m_pInGameBrowsing)
        return m_pInGameBrowsing->GetProductDetail(index, FALSE);
#endif

    return FALSE;
}

GS_BOOL	InGameMarketplace::RequestDownloadItems( GS_INT index, ContentInstalledCallBack callBack )
{
#if defined(_XBOX)
	if ( m_pDownloadMgr )
	{
		m_ContentInstalledCallBack = callBack;
		ULONGLONG offerID;
		if ( !m_pOfferMgr->GetOfferIDByIndex( index, offerID ) )
		{
			return FALSE;
		}
		DownloadRequest downloadRequest( 1 );
		downloadRequest.AddOffer( offerID );
		m_pDownloadMgr->DownloadItems( &downloadRequest );
		return TRUE;
	}
#elif defined(_PS3)
    if (m_pInGameBrowsing)
    {
		m_ContentInstalledCallBack = callBack;
        return m_pInGameBrowsing->BuyProduct( index, callBack );
    }
#endif

	return FALSE;
}

GS_BOOL InGameMarketplace::CheckContentListFinished(TArray<CMarketplaceItem>& productList)
{
    if (!m_bIsCatRequestPending)
        return FALSE;

    productList.Empty();

#if defined(_XBOX)
    if (m_pOfferMgr && m_pOfferMgr->IsOfferEnumerationFinished())
    {
        m_pOfferMgr->GetContentList(productList);
        m_bIsCatRequestPending = FALSE;
        return TRUE;
    }
#elif defined(_PS3)
    if (m_pInGameBrowsing)
    {
        if (m_pInGameBrowsing->IsGetCategoryFinished(productList))
        {
            m_bIsCatRequestPending = FALSE;
            return TRUE;
        }
    }
#endif
    return FALSE;
}

GS_BOOL InGameMarketplace::CheckProductDetailFinished(CMarketplaceDetail& detail)
{
    if (!m_bIsProdRequestPending)
        return FALSE;

#if defined(_XBOX)
    if (m_pOfferMgr && m_pOfferMgr->IsOfferEnumerationFinished())
    {
        if (!m_bDownloadImageStarted)
        {
            m_bDownloadImageStarted = TRUE;

            m_pOfferMgr->GetProductDetail(m_dwSelectedOffer, detail);

            DWORD dwErr;

            XSTORAGE_DOWNLOAD_TO_MEMORY_RESULTS pResults = {0};
            static char s_FileReadBuffer[500*1024];
            ZeroMemory( s_FileReadBuffer, sizeof( s_FileReadBuffer ) );

            WCHAR tmp_imageURL[XMARKETPLACE_IMAGE_URL_MINIMUM_WCHARCOUNT*2];
            size_t ret_value = 0; 
            mbstowcs_s(&ret_value, tmp_imageURL, XMARKETPLACE_IMAGE_URL_MINIMUM_WCHARCOUNT*2, detail.m_strImagePath, XMARKETPLACE_IMAGE_URL_MINIMUM_WCHARCOUNT*2);

            dwErr = XStorageDownloadToMemory( SignIn::GetActiveUserIndex(), tmp_imageURL,
                                              sizeof( s_FileReadBuffer ), ( const BYTE* )s_FileReadBuffer,
                                              sizeof( XSTORAGE_DOWNLOAD_TO_MEMORY_RESULTS ), &pResults, NULL );

            if( dwErr != ERROR_SUCCESS )
            {
                Master::G()->Log( "Error: File was not read." );
            }
            // else if( g_dwStorageFacilities[m_dwStorageIndex] != XSTORAGE_FACILITY_PER_TITLE )
            // {
            //     if( pResults.dwBytesTotal == sizeof( m_dwFileBuffer ) &&
            //         memcmp( ( void* )m_dwFileBuffer, m_dwFileReadBuffer, pResults.dwBytesTotal ) == 0 )
            //         Master::G()->Log( "File was successfully read and verified." );
            //     else
            //         Master::G()->Log( L"File did not pass verification." );
            // }
            else
            {
                Master::G()->Log( "File was successfully read." );
            }

            if (Master::G()->WriteLocalFile(GS_EFileIndex_TmpImg, (GS_BYTE*)s_FileReadBuffer, pResults.dwBytesTotal))
			{
                // Flash file system will set to Data/GFxAssets folder which is same as the LogFile class setting
                strcpy(detail.m_strImagePath, "GS_TmpDLCImgFile.png");
			}

            m_bDownloadImageEnded = FALSE;
            m_bIsProdRequestPending = FALSE;
            return TRUE;
            // Master::G()->GetHttpSrv()->QueryURL(detail.m_strImagePath.c_str());

            // // let TaskMgr track the QueryURL process
            // CTaskID id = 0;
            // Master::G()->GetTaskMgr()->AddTask(EGSTaskType_RetrieveImageFromHTTP,this,&id);
            // Master::G()->GetTaskMgr()->StartTask(id,ERROR_IO_PENDING);
        }
        else
        {
            // in downloading image
            if (m_bDownloadImageEnded)
            {
                m_bDownloadImageEnded = FALSE;
                m_bIsProdRequestPending = FALSE;
                return TRUE;
            }
        }
    }
#elif defined(_PS3)
    if (m_pInGameBrowsing)
    {
        if (m_pInGameBrowsing->IsGetProductFinished(detail))
        {
            if (!m_bDownloadImageStarted)
            {
                m_bDownloadImageStarted = TRUE;

                Master::G()->GetHttpSrv()->QueryURL(detail.m_strImagePath);

                // let TaskMgr track the QueryURL process
                CTaskID id = 0;
                Master::G()->GetTaskMgr()->AddTask(EGSTaskType_RetrieveImageFromHTTP,0,this,&id);
                Master::G()->GetTaskMgr()->StartTask(id);
            }
            else
            {
                // in downloading image
                if (m_bDownloadImageEnded)
                {
                    m_bDownloadImageEnded = FALSE;
                    m_bIsProdRequestPending = FALSE;

                    strcpy(detail.m_strImagePath, Master::G()->GetLocalFileName(GS_EFileIndex_TmpImg));
                    return TRUE;
                }
            }
        }
    }
#endif

    return FALSE;
}

//GS_BOOL InGameMarketplace::Add2WishList(GS_INT index)
//{
//    WishlistItem tmp;
//    tmp.uId = 0;
//    tmp.szName = "";
//#if defined(_XBOX)
//    if (m_pOfferMgr && m_pOfferMgr->IsOfferEnumerationFinished())
//    {
//        tmp.uId = (*m_pOfferMgr)[index].qwOfferID;
//        Wishlist::GetSingleton()->AddItem(tmp);
//		return TRUE;
//    }
//#elif defined(_PS3)
//    if (m_pInGameBrowsing)
//    {
//        GS_CHAR tmp_id[64];
//        if (m_pInGameBrowsing->GetProductID(index, tmp_id))
//        {
//            tmp.szName = tmp_id;
//			return TRUE;
//        }
//    }
//#endif
//	return FALSE;
//}


GS_VOID InGameMarketplace::MessageResponse(Message* message)
{
#if defined(_XBOX)
    if (EMessage_SignInChanged == message->GetMessageID())
    {
        GS_INT signed_in = *(GS_INT*)message->ReadPayload(0);
        if (signed_in)
        {
            if (m_pOfferMgr)
                m_pOfferMgr->EnumerateOffers(m_iCurrentCategory);
        }
    }
    else if (EMessage_ContentInstalled == message->GetMessageID())
    {
        if (m_pContentMgr)
            m_pContentMgr->OnContentInstalled();

        if (m_pOfferMgr)
            m_pOfferMgr->EnumerateOffers(m_iCurrentCategory);

		if ( m_ContentInstalledCallBack )
			m_ContentInstalledCallBack();
    }
    else 
#endif
    if (EMessage_InternalTaskDone == message->GetMessageID())
    {	
        CTaskID task_id = *(CTaskID*)message->ReadPayload(0);
        GS_TaskType taskType = (GS_TaskType)(*(GS_INT*)message->ReadPayload(1));
        GS_DWORD taskResult = *(GS_DWORD*)message->ReadPayload(2);

        // finished no matter whether succeed in the process
        if (EGSTaskType_RetrieveImageFromHTTP == taskType)
        {
            m_bDownloadImageEnded = TRUE;
        }
    }
}

} // namespace
