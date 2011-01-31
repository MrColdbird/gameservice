// ======================================================================================
// File         : ContentManager.h
// Author       : Li Chen 
// Last Change  : 01/07/2011 | 14:42:59 PM | Friday,January
// Description  : 
// ======================================================================================

#ifndef GAMESERVICE_CONTENT_MANAGER_H
#define GAMESERVICE_CONTENT_MANAGER_H

#include "Message.h"

namespace GameService
{

//--------------------------------------------------------------------------------------
// Name: ContentManager
// Desc: Maintains a list of installed content packages.
//--------------------------------------------------------------------------------------
class ContentManager : public MessageRecipient
{   

public:

	// Need to define these since I've declared the copy constructor
	ContentManager(MessageMgr* msgMgr);

    GS_BOOL Initialize();
    GS_VOID Finalize();

    //--------------------------------------------------------------------------------------
    // Name: operator[]
    // Desc: Used for iterating over the XCONTENT_DATA collection
    //--------------------------------------------------------------------------------------
    XCONTENT_DATA operator[](GS_DWORD dw) { return m_aContentData(dw);}

    //--------------------------------------------------------------------------------------
    // Name: Count
    // Desc: report the number of elements in the XCONTENT_DATA collection
    //--------------------------------------------------------------------------------------
    GS_DWORD Count() { return m_aContentData.Num(); }

    GS_BOOL Enumerate();

    GS_VOID Cleanup();

	// inherit from MessageHandler
	GS_VOID MessageResponse(Message* message);

public:

    //--------------------------------------------------------------------------------------
    // Name: Update
    // Desc: Checks to see if the enumerator is ready and if so updates the enumerator
    //--------------------------------------------------------------------------------------
    GS_VOID Update();

    //--------------------------------------------------------------------------------------
    // Name: OnContentInstalled
    // Desc: This method is called in response to XN_LIVE_CONTENT_INSTALLED
    //       Kicks off another enumeration to discover new content packages
    //--------------------------------------------------------------------------------------
    GS_BOOL OnContentInstalled();

private:
	TArray<XCONTENT_DATA> m_aContentData;     // Collection of XCONTENT_DATA

    HANDLE m_hEnumeration;
    XOVERLAPPED m_Overlapped;   // Overlapped structure for asynchronous I/O
    GS_DWORD       m_dwBufferSize; // Size of the enumeration buffer
    GS_BYTE*       m_pBuffer;      // Pointer to the enumeration buffer

};

} // namespace

#endif // GAMESERVICE_CONTENT_MANAGER_H
