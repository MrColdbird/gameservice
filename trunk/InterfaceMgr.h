// ======================================================================================
// File         : InterfaceMgr.h
// Author       : Li Chen 
// Last Change  : 07/29/2010 | 15:47:32 PM | Thursday,July
// Description  : 
// ======================================================================================

#ifndef GAMESERVICE_INTERFACEMGR
#define GAMESERVICE_INTERFACEMGR

#include "Message.h"

namespace GameService
{

class InterfaceMgr : public MessageRecipient
{
public:
	InterfaceMgr() {}
	InterfaceMgr(MessageMgr* msgMgr);

	// inherit from MessageHandler
	void MessageResponse(Message* message);


};

} // namespace GameService

#endif // GAMESERVICE_INTERFACEMGR
