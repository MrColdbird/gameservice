// ======================================================================================
// File         : Message.cpp
// Author       : Li Chen 
// Last Change  : 07/29/2010 | 15:47:52 PM | Thursday,July
// Description  : 
// ======================================================================================

#include "stdafx.h"
#include "Message.h"


namespace GameService
{

Message* Message::Create(MessageID messageId)
{
	return GS_NEW Message(messageId);
}

void Message::Discard()
{
	GS_DELETE this;
}

Message::~Message()
{
	for(GS_UINT i=0;i<m_PayloadArray.Num();i++)
	{
		if (m_PayloadArray(i))
		{
			// No destructor will be called!
			GS_DELETE m_PayloadArray(i);
		}
	}

	m_PayloadArray.Empty();
}

void Message::AddTarget(MessageRecipient* target)
{
	if (m_recipient)
	{
		GS_Assert(0); 
	}
	else 
	{
		if (target) 
		{
			m_recipient = target;
		}
		else 
		{
			GS_Assert(0);
		}
	}
}

GS_VOID Message::AddPayload( GS_BOOL payload ) 
{ 
	m_PayloadArray.AddItem((GS_BYTE*)GS_NEW GS_BOOL(payload)); 
}
#if defined(_XBOX) || defined(_XENON) || defined(_WINDOWS)
GS_VOID Message::AddPayload( GS_DWORD payload ) 
{ 
	m_PayloadArray.AddItem((GS_BYTE*)GS_NEW GS_DWORD(payload)); 
}
#elif defined(_PS3)
GS_VOID Message::AddPayload( GS_INT payload )
{ 
	m_PayloadArray.AddItem((GS_BYTE*)GS_NEW GS_INT(payload)); 
}
#endif
GS_VOID Message::AddPayload( GS_FLOAT payload ) 
{ 
	m_PayloadArray.AddItem((GS_BYTE*)GS_NEW GS_FLOAT(payload)); 
}		
GS_VOID Message::AddPayload( GS_DOUBLE payload ) 
{ 
	m_PayloadArray.AddItem((GS_BYTE*)GS_NEW GS_DOUBLE(payload)); 
}


GS_BYTE* Message::ReadPayload(GS_UINT index)
{
	return m_PayloadArray(index);
}

void MessageHandler::AddHandler(MessageRecipient* recipient)
{
	m_CallbackArray.AddItem(recipient);
}

void MessageHandler::RemoveHandler(MessageRecipient* recipient)
{
	m_CallbackArray.RemoveItem(recipient);
}

void MessageHandler::ExecuteAll(Message* message)
{
	MessageRecipient* Target = message->GetTarget();
	for(GS_UINT i=0;i<m_CallbackArray.Num();i++)
	{
		if (Target)
		{
			if (Target == m_CallbackArray(i))
				m_CallbackArray(i)->MessageResponse(message);
		}
		else
			m_CallbackArray(i)->MessageResponse(message);		
	}
}

MessageMgr::MessageMgr()
{
	for(GS_INT i=0;i<EMessage_MAX;i++)
	{
		m_MessageHandlers[i] = NULL;
	}
}

MessageMgr::~MessageMgr()
{
	for(GS_INT i=0;i<EMessage_MAX;i++)
	{
		if(m_MessageHandlers[i])
		{
			GS_DELETE m_MessageHandlers[i];
		}
	}
}

void MessageMgr::Register(MessageID messageId, MessageRecipient* recipient)
{
	if(NULL == m_MessageHandlers[messageId])
	{
		m_MessageHandlers[messageId] = GS_NEW MessageHandler(messageId);
	}
	
	m_MessageHandlers[messageId]->AddHandler(recipient);
}

GS_BOOL MessageMgr::Send(Message *message)
{
	MessageID message_id = message->GetMessageID();

	GS_BOOL bSent = FALSE;	
	
	GS_Assert(message_id > EMessage_NONE && message_id < EMessage_MAX);

	if(m_MessageHandlers[message_id])
	{
		m_MessageHandlers[message_id]->ExecuteAll(message);

		bSent = TRUE;
	}	
	
	message->Discard();
	return bSent; 
}

} // namespace GameService
