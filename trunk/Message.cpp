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
	return new(GSOPType) Message(messageId);
}

void Message::Discard()
{
	DeleteThis<Message>(this);
}

Message::~Message()
{
	for(GS_INT i=0;i<m_PayloadArray.Num();i++)
	{
		if (m_PayloadArray(i))
		{
			// No destructor will be called!
			Free(m_PayloadArray(i));
		}
	}

	m_PayloadArray.Empty();
}

void Message::AddTarget(MessageRecipient* target)
{
	if (m_recipient)
	{
		assert(0); 
	}
	else 
	{
		if (target) 
		{
			m_recipient = target;
		}
		else 
		{
			assert(0);
		}
	}
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
	for(GS_INT i=0;i<m_CallbackArray.Num();i++)
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
			Delete<MessageHandler>(m_MessageHandlers[i]);
		}
	}
}

void MessageMgr::Register(MessageID messageId, MessageRecipient* recipient)
{
	if(NULL == m_MessageHandlers[messageId])
	{
		m_MessageHandlers[messageId] = new(GSOPType) MessageHandler(messageId);
	}
	
	m_MessageHandlers[messageId]->AddHandler(recipient);
}

GS_BOOL MessageMgr::Send(Message *message)
{
	MessageID message_id = message->GetMessageID();

	GS_BOOL bSent = FALSE;	
	
	assert(message_id > EMessage_NONE && message_id < EMessage_MAX);

	if(m_MessageHandlers[message_id])
	{
		m_MessageHandlers[message_id]->ExecuteAll(message);

		bSent = TRUE;
	}	
	
	message->Discard();
	return bSent; 
}

} // namespace GameService
