// ======================================================================================
// File         : InGameDownloadManager.h
// Author       : Li Chen 
// Last Change  : 01/07/2011 | 14:36:49 PM | Friday,January
// Description  : 
// ======================================================================================

#ifndef GAMESERVICE_INGAMEDOWNLOAD_MANAGER_H
#define GAMESERVICE_INGAMEDOWNLOAD_MANAGER_H

#include <map>

namespace GameService
{

//--------------------------------------------------------------------------------------
// Name: DownloadManager
// Desc: Initiates Content Downloads with the user, tracks download status
//--------------------------------------------------------------------------------------
class DownloadRequest;
class DownloadManager
{
public:
    DownloadManager();
    ~DownloadManager();

    //--------------------------------------------------------------------------------------
    // Name: RequestDownload
    // Desc: Creates a DownloadRequest object to store OfferIDs to be downloaded.
    //       Caller will add OfferIDs to the DownloadRequest object using
    //       DownloadRequest::AddOffer
    //--------------------------------------------------------------------------------------
    DownloadRequest* RequestDownload( GS_DWORD nOffers );

    //--------------------------------------------------------------------------------------
    // Name: DownloadItems
    // Desc: Displays the Marketplace blade to the user to allow purchasing/downloading
    //       the list of OfferIDs that were added to the DownloadRequest
    //--------------------------------------------------------------------------------------
    GS_VOID DownloadItems( DownloadRequest *pRequest );

    //--------------------------------------------------------------------------------------
    // Name: Update
    // Desc: Check to see if the Marketplace blade is still showing
    //       Refresh the status of all the pending downloads that are being tracked
    //--------------------------------------------------------------------------------------
    GS_VOID Update();

	typedef std::map< ULONGLONG, GS_DWORD > PendingDownloadMap;

    // Enable iterating over the collection of pending downloads
    GS_DWORD Count() { return m_mPendingDownloads.size(); }
    PendingDownloadMap::iterator begin() { return m_mPendingDownloads.begin(); }
    PendingDownloadMap::iterator end()   { return m_mPendingDownloads.end(); }

private:
    //--------------------------------------------------------------------------------------
    // Track Pending Downloads
    //--------------------------------------------------------------------------------------

    //--------------------------------------------------------------------------------------
    // Name: AddPendingDownload
    // Desc: Add another OfferID to the list of pending downloads without duplication
    //--------------------------------------------------------------------------------------
    GS_VOID AddPendingDownload( ULONGLONG qwOfferID );

    //--------------------------------------------------------------------------------------
    // Name: RefreshPendingDownloads
    // Desc: Get the latest status for each OfferID in the collection
    //--------------------------------------------------------------------------------------
    GS_VOID RefreshPendingDownloads();

public:

    //--------------------------------------------------------------------------------------
    // Name: RemoveExpiredDownloads
    // Desc: Clean out all the downloads that have completed or errored out
    //--------------------------------------------------------------------------------------
    GS_VOID RemoveExpiredDownloads();
	
    PendingDownloadMap m_mPendingDownloads;

private:
    // Requesting Offer Downloads
    DownloadRequest *m_pCurrentRequest;
    HRESULT m_hrOverlappedResult;
    XOVERLAPPED m_Overlapped;

};

//--------------------------------------------------------------------------------------
// Name: DownloadRequest
// Desc: Auxilliary class to encapsulate a list of OfferIDs for content to be downloaded
//       DownloadRequests are created and managed by the DownloadManager
//--------------------------------------------------------------------------------------
class DownloadRequest
{
public:
    DownloadRequest( GS_DWORD nSize );
    ~DownloadRequest();

    //--------------------------------------------------------------------------------------
    // Name: AddOffer
    // Desc: Adds an OfferID to the array of OfferIDs
    //--------------------------------------------------------------------------------------
    GS_BOOL AddOffer( ULONGLONG qwOfferIDtoAdd );

private:
    GS_DWORD m_dwOfferIDs;       // Number of OfferIDs in the request
    GS_DWORD m_dwSize;           // Max number of OfferIDs that will fit in the request
    ULONGLONG *m_pOfferIDs;   // Storage for the OfferIDs

public:
    
    GS_DWORD GetOfferIDCount() const { return m_dwOfferIDs; }
    
    const ULONGLONG* GetOfferIDs() const { return m_pOfferIDs; }
    
    ULONGLONG GetOfferID( GS_DWORD dwIndex ) const
    {
        assert( dwIndex < m_dwOfferIDs );
        return m_pOfferIDs[dwIndex];
    }

private:
    // don't want to copy these
    DownloadRequest( const DownloadRequest &) {}
    DownloadRequest &operator=( const DownloadRequest &) { return *this; } 
};

} // namespace

#endif // GAMESERVICE_INGAMEDOWNLOAD_MANAGER_H
