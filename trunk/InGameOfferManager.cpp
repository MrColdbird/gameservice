// ======================================================================================
// File         : InGameOfferManager.cpp
// Author       : Li Chen 
// Last Change  : 01/07/2011 | 14:48:54 PM | Friday,January
// Description  : 
// ======================================================================================

#include "stdafx.h"

#if defined(_XBOX)

#include "InGameDownloadManager.h"
#include "InGameOfferManager.h"
#include "SignIn.h"
#include "Master.h"
#include "Task.h"

#define MAX_ENUMERATION_RESULTS 10

namespace GameService
{

OfferManager::OfferManager(MessageMgr* msgMgr)
: m_hEnumeration( INVALID_HANDLE_VALUE ),
m_dwBufferSize( 0 ),
m_pBuffer( NULL ),
m_bIsOfferEnumerationFinished(FALSE)
{
	if (msgMgr)
	{
		msgMgr->Register(EMessage_SignInChanged,this);
		msgMgr->Register(EMessage_OnlineTaskDone,this);
	}
}

// ======================================================== 
// Initialize
// ======================================================== 
GS_BOOL OfferManager::Initialize()
{
	Cleanup();

	return TRUE;
}

// ======================================================== 
// Cleanup
// ======================================================== 
GS_VOID OfferManager::Cleanup()
{
    m_bIsOfferEnumerationFinished = FALSE;

    // Free the handle
    if ( m_hEnumeration != INVALID_HANDLE_VALUE )
    {
        XCloseHandle( m_hEnumeration );
        m_hEnumeration = INVALID_HANDLE_VALUE;
    }

    // Delete the buffer
    if (m_pBuffer)
    {
        delete [] m_pBuffer;
        m_pBuffer = NULL;
    }
    m_dwBufferSize = 0;

	ClearOffers();
}

//--------------------------------------------------------------------------------------
// Name: Update
//--------------------------------------------------------------------------------------
GS_VOID OfferManager::Update()
{
}

//--------------------------------------------------------------------------------------
// Name: EnumerateOffers
//--------------------------------------------------------------------------------------
GS_BOOL OfferManager::EnumerateOffers(GS_INT category)
{
	if (!SignIn::AreUsersSignedIn())
        return FALSE;

	Cleanup();

    // Enumerate at most MAX_ENUMERATION_RESULTS items
	GS_DWORD dwError = XMarketplaceCreateOfferEnumerator
		(
		SignIn::GetActiveUserIndex(),       // Get the offer data for this player
		XMARKETPLACE_OFFERING_TYPE_CONTENT,   // Marketplace content (can combine values using ||)
		category,                           // Retrieve all content catagories
		MAX_ENUMERATION_RESULTS,              // Number of results per call to XEnumerate
		&m_dwBufferSize,                        // Size of buffer needed for results
		&m_hEnumeration                         // Enumeration handle
		);

    if ( FAILED( HRESULT_FROM_WIN32(dwError) ) )
    {
        Cleanup();
        return FALSE;
    }

    m_pBuffer = GS_NEW BYTE[m_dwBufferSize];
    if ( !m_pBuffer )
    {
        Master::G()->Log("OfferManager::Initialize - Out of Memory!");

        Cleanup();
        return FALSE;
    }

    // Enumerate contents asynchronized
	CTaskID id = 0;
    GS_DWORD dwStatus = XEnumerate( m_hEnumeration, m_pBuffer, m_dwBufferSize, NULL,
                    Master::G()->GetTaskMgr()->AddTask(EGSTaskType_OfferMgrEnumeration,this,&id) );
	Master::G()->GetTaskMgr()->StartTask(id,dwStatus);

    if( dwStatus != ERROR_IO_PENDING )
    {
		Master::G()->Log("OfferManager::Enumerate() failed with error %d", dwStatus);
    }

    return TRUE;
}

//--------------------------------------------------------------------------------------
// Name: _AddOffer
// Desc: Create a copy of the referenced offer and add it to the array of offers during
//       enumeration.
//--------------------------------------------------------------------------------------
GS_VOID OfferManager::AddOffer( const XMARKETPLACE_CONTENTOFFER_INFO& Offer )
{
	// Caculate the size needed for the structure
	GS_DWORD dwBufferSize = sizeof( XMARKETPLACE_CONTENTOFFER_INFO );
	// arrange for extra storage at the end to hold the three strings
	dwBufferSize += Offer.dwOfferNameLength;
	dwBufferSize += Offer.dwSellTextLength;
	dwBufferSize += Offer.dwTitleNameLength;

	// Allocate and initialize
	PXMARKETPLACE_CONTENTOFFER_INFO pOfferData = (PXMARKETPLACE_CONTENTOFFER_INFO)GS_NEW BYTE[dwBufferSize];
	ZeroMemory( pOfferData, dwBufferSize );
	XMemCpy( pOfferData, &Offer, sizeof( XMARKETPLACE_CONTENTOFFER_INFO ) );
	
    // convert wchar to char!!!
    size_t ret_value = 0;
	// Copy the Offer Name
	pOfferData->wszOfferName = (WCHAR *)&pOfferData[1];
    wcstombs_s(&ret_value, (char*)pOfferData->wszOfferName, Offer.dwOfferNameLength, Offer.wszOfferName, Offer.dwOfferNameLength);
    // XMemCpy(pOfferData->wszOfferName, Offer.wszOfferName, Offer.dwOfferNameLength);

	// Copy the Sell Text
	pOfferData->wszSellText = pOfferData->wszOfferName + Offer.dwOfferNameLength;
    wcstombs_s(&ret_value, (char*)pOfferData->wszSellText, Offer.dwSellTextLength, Offer.wszSellText, Offer.dwSellTextLength);
    // XMemCpy(pOfferData->wszSellText, Offer.wszSellText, Offer.dwSellTextLength);

	// Copy the Title Name
	pOfferData->wszTitleName = pOfferData->wszSellText  + Offer.dwSellTextLength;
    wcstombs_s(&ret_value, (char*)pOfferData->wszTitleName, Offer.dwTitleNameLength, Offer.wszTitleName, Offer.dwTitleNameLength);
    // XMemCpy(pOfferData->wszTitleName, Offer.wszTitleName, Offer.dwTitleNameLength);

	// Add the record to the table
	m_aOfferData.AddItem(pOfferData);
}

//--------------------------------------------------------------------------------------
// Name: ClearOffers
// Desc: Free each structure in the collection then clear the collection
//--------------------------------------------------------------------------------------
GS_VOID OfferManager::ClearOffers()
{
	for (unsigned i = 0; i < m_aOfferData.Num(); ++i)
	{
		GS_DELETE [] m_aOfferData(i);
	}
	m_aOfferData.Empty();
}

// ======================================================== 
// 
// ======================================================== 
GS_BOOL OfferManager::IsOfferEnumerationFinished()
{
    return m_bIsOfferEnumerationFinished;
}

GS_VOID OfferManager::GetContentList(TArray<CMarketplaceItem>& productList)
{
    if (m_bIsOfferEnumerationFinished)
    {
        for (GS_UINT i=0; i<m_aOfferData.Num(); i++)
        {
            CMarketplaceItem tmp;
            tmp.m_iIndex = i;
			strcpy(tmp.m_strName, (char*)(m_aOfferData(i)->wszOfferName));
            productList.AddItem(tmp);
        }
    }
}
GS_VOID OfferManager::GetProductDetail(GS_INT index, CMarketplaceDetail& detail)
{
    if (m_bIsOfferEnumerationFinished)
    {
		strcpy(detail.m_strName, (char*)(m_aOfferData(index)->wszOfferName));
        strcpy(detail.m_strDesc, (char*)(m_aOfferData(index)->wszSellText));

        // get image URL
        WCHAR tmp_imageURL[XMARKETPLACE_IMAGE_URL_MINIMUM_WCHARCOUNT*2];
        XMarketplaceGetImageUrl(m_aOfferData(index)->dwTitleID, m_aOfferData(index)->qwOfferID, 
                                XMARKETPLACE_IMAGE_URL_MINIMUM_WCHARCOUNT*2, tmp_imageURL);
        size_t ret_value = 0; 
        wcstombs_s(&ret_value, m_cCurrentProductImageURL, XMARKETPLACE_IMAGE_URL_MINIMUM_WCHARCOUNT*2, tmp_imageURL, XMARKETPLACE_IMAGE_URL_MINIMUM_WCHARCOUNT*2);
        strcpy(detail.m_strImagePath, m_cCurrentProductImageURL);
    }
}
GS_BOOL OfferManager::GetOfferIDByIndex( GS_INT index, ULONGLONG& offerID )
{
	if ( (size_t)index < m_aOfferData.Num() )
	{
		offerID = m_aOfferData( index )->qwOfferID;
		return TRUE;
	}

	return FALSE;
}

// ======================================================== 
// MessageResponse
// ======================================================== 
GS_VOID OfferManager::MessageResponse(Message* message)
{
    if (EMessage_SignInChanged == message->GetMessageID())
    {
        return;
    }

	CTaskID task_id = *(CTaskID*)message->ReadPayload(0);
	GS_TaskType taskType = (GS_TaskType)(*(GS_INT*)message->ReadPayload(1));
	GS_DWORD taskResult = *(GS_DWORD*)message->ReadPayload(2);
	GS_DWORD taskDetailedResult = *(GS_DWORD*)message->ReadPayload(3);
#if defined(_XBOX) || defined(_XENON)
	GS_DWORD taskExtendedResult = *(GS_DWORD*)message->ReadPayload(4);
#endif

#if defined(_XBOX) || defined(_XENON)
    if (taskResult == ERROR_SUCCESS)
#elif defined(_PS3)
    if (0 == taskResult && taskDetailedResult >= 0)
#endif
    {
        switch(taskType)
        {
        case EGSTaskType_OfferMgrEnumeration:
            {
                PXMARKETPLACE_CONTENTOFFER_INFO pTempOfferData = (PXMARKETPLACE_CONTENTOFFER_INFO)m_pBuffer;
                for ( GS_DWORD dw = 0; dw < taskDetailedResult; ++dw )
                {
                    AddOffer( pTempOfferData[dw] );
                }

                m_bIsOfferEnumerationFinished = TRUE;
            }
            break;
        }
    }
}


} // namespace

#endif