// ======================================================================================
// File         : InGameDownloadManager.cpp
// Author       : Li Chen 
// Last Change  : 01/07/2011 | 14:38:29 PM | Friday,January
// Description  : 
// ======================================================================================

#include "stdafx.h"

#if defined(_XBOX)

#include "Array.h"
#include "SignIn.h"
#include "Master.h"
#include "InGameDownloadManager.h"

namespace GameService
{

//--------------------------------------------------------------------------------------
// DownloadManager implementation
//--------------------------------------------------------------------------------------
DownloadManager::DownloadManager() :
m_pCurrentRequest(NULL),
m_hrOverlappedResult(S_OK)
{
    ZeroMemory( &m_Overlapped, sizeof( XOVERLAPPED ) );
    m_Overlapped.hEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
    if( m_Overlapped.hEvent == NULL )
    {
		Master::G()->Log( "Failed to create Overlapped event.\n" );
    }
}

DownloadManager::~DownloadManager()
{
    // Cancel any outstanding request
    if (m_pCurrentRequest)
    {
        XCancelOverlapped( &m_Overlapped );
        delete m_pCurrentRequest;
        m_pCurrentRequest = NULL;
    }

    CloseHandle( m_Overlapped.hEvent );
}

//--------------------------------------------------------------------------------------
// Name: RequestDownload
// Desc: Creates a DownloadRequest object to store OfferIDs to be downloaded.
//       Caller will add OfferIDs to the DownloadRequest object using
//       DownloadRequest::AddOffer
//--------------------------------------------------------------------------------------
DownloadRequest*
DownloadManager::RequestDownload( GS_DWORD dwOffers )
{
    assert( NULL == m_pCurrentRequest );
    if ( NULL == m_pCurrentRequest )
    {
        m_pCurrentRequest = new DownloadRequest ( dwOffers );
        assert( m_pCurrentRequest );
        return m_pCurrentRequest;
    }
    return NULL;
}

//--------------------------------------------------------------------------------------
// Name: DownloadItems
// Desc: Displays the Marketplace blade to the user to allow purchasing/downloading
//       the list of OfferIDs that were added to the DownloadRequest
//--------------------------------------------------------------------------------------
GS_VOID
DownloadManager::DownloadItems( DownloadRequest* pRequest )
{
    GS_DWORD dwErr = ERROR_SUCCESS;

    GS_DWORD dwEntryPoint = XSHOWMARKETPLACEDOWNLOADITEMS_ENTRYPOINT_PAIDITEMS;
    
    dwErr = XShowMarketplaceDownloadItemsUI(
        SignIn::GetActiveUserIndex(),
        dwEntryPoint,
        pRequest->GetOfferIDs(),        // the OfferIDs for the items we're downloading
        pRequest->GetOfferIDCount(),    // the number of OfferIDs to download
        &m_hrOverlappedResult,
        &m_Overlapped                   // XShowMarketplaceDownloadItemsUI must be called asynchronously
        );

    assert( ERROR_IO_PENDING == dwErr );
}

//--------------------------------------------------------------------------------------
// Name: Update
// Desc: Check to see if the Marketplace blade is still showing
//       Refresh the status of all the pending downloads that are being tracked
//--------------------------------------------------------------------------------------
GS_VOID
DownloadManager::Update()
{
    // m_pCurrentRequest is non-null during the call to XShowMarketplaceDownloadItemsUI
    if ( m_pCurrentRequest )
    {
        GS_DWORD dwResult;
        GS_DWORD dwErr = XGetOverlappedResult( &m_Overlapped, &dwResult, FALSE );
        if ( dwErr != ERROR_IO_INCOMPLETE && dwErr != ERROR_CANCELLED)
        {
            assert( dwErr == ERROR_SUCCESS );

            // Add to the pending downloads
            for( GS_DWORD dw = 0; dw < m_pCurrentRequest->GetOfferIDCount(); ++dw )
            {
                AddPendingDownload( m_pCurrentRequest->GetOfferID(dw) );
            }

            delete m_pCurrentRequest;
            m_pCurrentRequest = NULL;
        }
    }

    RefreshPendingDownloads();
}

//--------------------------------------------------------------------------------------
// Name: AddPendingDownload
// Desc: Add another OfferID to the list of pending downloads without duplication
//--------------------------------------------------------------------------------------
GS_VOID
DownloadManager::AddPendingDownload( ULONGLONG qwOfferID )
{
    // Record the current download status for the given OfferID
    GS_DWORD dwDownloadStatus;
    GS_DWORD dwErr = XMarketplaceGetDownloadStatus( SignIn::GetActiveUserIndex(), qwOfferID, &dwDownloadStatus );
    assert( dwErr == ERROR_SUCCESS );

    if ( dwErr == ERROR_SUCCESS )
    {
        // Add the OfferID and corresponding status to the map
        m_mPendingDownloads[qwOfferID] = dwDownloadStatus;
    }
}

//--------------------------------------------------------------------------------------
// Name: RefreshPendingDownloads
// Desc: Get the latest status for each OfferID in the collection
//--------------------------------------------------------------------------------------
GS_VOID
DownloadManager::RefreshPendingDownloads()
{
    GS_DWORD dwErr = ERROR_SUCCESS;
    for ( PendingDownloadMap::iterator iter = m_mPendingDownloads.begin(); iter != m_mPendingDownloads.end(); ++iter )
    {
        if ( iter->second == ERROR_IO_PENDING )
        {
            GS_DWORD dwDownloadStatus;
            dwErr = XMarketplaceGetDownloadStatus( SignIn::GetActiveUserIndex(), iter->first, &dwDownloadStatus );
            if ( ERROR_SUCCESS == dwErr )
            {
                iter->second = dwDownloadStatus;
            }
            else
            {
                break;
            }
        }
    }
    assert( ERROR_SUCCESS == dwErr );
}

//--------------------------------------------------------------------------------------
// Name: RemoveExpiredDownloads
// Desc: Clean out all the downloads that have completed or errored out
//--------------------------------------------------------------------------------------
GS_VOID
DownloadManager::RemoveExpiredDownloads()
{
    // First go through the set and find all the keepers
    TArray<ULONGLONG> vKeepers;
    for ( PendingDownloadMap::iterator iter = m_mPendingDownloads.begin(); iter != m_mPendingDownloads.end(); ++iter )
    {
        if ( ERROR_IO_PENDING == iter->second )
        {
            vKeepers.AddItem( iter->first );
        }
    }

    // Clear out the map
    m_mPendingDownloads.clear();
    

    // Now add all the keepers back in if they are still pending
    for ( GS_DWORD dw = 0; dw < vKeepers.Num(); ++dw )
    {
        GS_DWORD dwDownloadStatus;
        GS_DWORD dwErr = XMarketplaceGetDownloadStatus( SignIn::GetActiveUserIndex(), vKeepers(dw), &dwDownloadStatus );
        assert( dwErr == ERROR_SUCCESS );
        if ( dwErr == ERROR_SUCCESS && dwDownloadStatus == ERROR_IO_PENDING )
        {
            m_mPendingDownloads[vKeepers(dw)] = dwDownloadStatus;
        }
    }
}

//--------------------------------------------------------------------------------------
// DownloadRequest implementation
//--------------------------------------------------------------------------------------

DownloadRequest::DownloadRequest(GS_DWORD dwSize) :
m_dwOfferIDs(0),
m_dwSize(0),
m_pOfferIDs(NULL)
{
    m_pOfferIDs = new ULONGLONG [dwSize];
    assert( m_pOfferIDs );

    if ( m_pOfferIDs )
    {
        m_dwSize = dwSize;
    }
}

DownloadRequest::~DownloadRequest()
{
    if ( m_pOfferIDs )
    {
        delete [] m_pOfferIDs;
    }
    m_dwSize = 0;
    m_dwOfferIDs = 0;
}

//--------------------------------------------------------------------------------------
// Name: AddOffer
// Desc: Adds an OfferID to the array of OfferIDs
//--------------------------------------------------------------------------------------
GS_BOOL
DownloadRequest::AddOffer( ULONGLONG qwOfferIDtoAdd )
{
    if ( m_dwOfferIDs < m_dwSize )
    {
        m_pOfferIDs[m_dwOfferIDs] = qwOfferIDtoAdd;
        ++m_dwOfferIDs;
        return TRUE;
    }
    return FALSE;
}

} // namespace

#endif