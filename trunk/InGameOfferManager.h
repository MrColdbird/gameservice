// ======================================================================================
// File         : InGameOfferManager.h
// Author       : Li Chen 
// Last Change  : 01/07/2011 | 14:45:30 PM | Friday,January
// Description  : 
// ======================================================================================

#ifndef GAMESERVICE_INGAME_OFFER_MANAGER_H
#define GAMESERVICE_INGAME_OFFER_MANAGER_H

#include <set>
#include "Message.h"
#include "Interface.h"

namespace GameService
{

//--------------------------------------------------------------------------------------
// Name: OfferManager
// Desc: Manage all the Offers
//--------------------------------------------------------------------------------------
class OfferManager : public MessageRecipient
{

public:

	OfferManager(MessageMgr* msgMgr);
	virtual ~OfferManager()	{ ClearOffers(); }

    GS_BOOL Initialize();

	//--------------------------------------------------------------------------------------
	// Name: Count
	// Desc: report the number of elements in the XMARKETPLACE_CONTENTOFFER_INFO collection
	//--------------------------------------------------------------------------------------
	GS_DWORD Count() { return m_aOfferData.size(); }

	//--------------------------------------------------------------------------------------
	// Name: operator[]
	// Desc: Allow iterating over the XMARKETPLACE_CONTENTOFFER_INFO collection
	//--------------------------------------------------------------------------------------
	const XMARKETPLACE_CONTENTOFFER_INFO& operator[](GS_DWORD dw){ return *m_aOfferData[dw]; }

	//--------------------------------------------------------------------------------------
	// Name: Update
	// Desc: Checks to see if the enumerator is ready and if so updates the enumerator
	//--------------------------------------------------------------------------------------
	GS_VOID Update();

	//--------------------------------------------------------------------------------------
	// Name: EnumerateOffers
	// Desc: Cancel any pending offer enumeration and start over
	//--------------------------------------------------------------------------------------
	GS_BOOL EnumerateOffers(GS_INT category);

    GS_VOID Cleanup();

	// inherit from MessageHandler
	GS_VOID MessageResponse(Message* message);

    GS_BOOL IsOfferEnumerationFinished();
    GS_VOID GetContentList(efd::vector<CMarketplaceItem>& productList);
    GS_VOID GetProductDetail(GS_INT index, CMarketplaceDetail& detail);
	GS_BOOL GetOfferIDByIndex( GS_INT index, ULONGLONG& offerID );

private:
	//--------------------------------------------------------------------------------------
	// Name: _AddOffer
	// Desc: Create a copy of the referenced offer and add it to the array of offers during
	//       enumeration.
	//--------------------------------------------------------------------------------------
	GS_VOID AddOffer( const XMARKETPLACE_CONTENTOFFER_INFO& Offer );

	//--------------------------------------------------------------------------------------
	// Name: ClearOffers
	// Desc: Free each structure in the collection then clear the collection
	//--------------------------------------------------------------------------------------
	GS_VOID ClearOffers();

private:
	typedef efd::vector<PXMARKETPLACE_CONTENTOFFER_INFO> OfferDataArray;

	OfferDataArray m_aOfferData;        // Collection of XMARKETPLACE_CONTENTOFFER_INFO structures

    HANDLE m_hEnumeration;
    XOVERLAPPED m_Overlapped;   // Overlapped structure for asynchronous I/O
    GS_DWORD       m_dwBufferSize; // Size of the enumeration buffer
    GS_BYTE*       m_pBuffer;      // Pointer to the enumeration buffer

    char m_cCurrentProductImageURL[XMARKETPLACE_IMAGE_URL_MINIMUM_WCHARCOUNT*2];

    GS_BOOL m_bIsOfferEnumerationFinished;
};

} // namespace

#endif // GAMESERVICE_INGAME_OFFER_MANAGER_H
