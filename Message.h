// ======================================================================================
// File         : Message.h
// Author       : Li Chen 
// Last Change  : 07/29/2010 | 15:47:57 PM | Thursday,July
// Description  : 
// ======================================================================================

#ifndef GAMESERVICE_MESSAGE_H
#define GAMESERVICE_MESSAGE_H

#include "Array.h"

namespace GameService
{

enum MessageID
{
	EMessage_NONE = 0,
	EMessage_Test,
	EMessage_OnlineTaskDone,
	EMessage_ShowXUIDone,
	EMessage_AsyncTaskDone, //ZJ - For AsyncTasks;
	EMessage_CustomGuide_ActionManage,
	EMessage_ShowUserSharedReplay,
	EMessage_CallBackInterface,
    EMessage_SignInChanged,
	EMessage_MAX
};

class MessageRecipient;

class Message
{
public:
	Message() {m_MessageId = EMessage_NONE; m_recipient= NULL;}
	Message(MessageID messageId) { m_MessageId = messageId; m_PayloadArray.Empty(); m_recipient = NULL;}
	~Message();
	//Create();
	static Message* Create(MessageID messageId);
	void Discard();
	MessageID GetMessageID() { return m_MessageId; }

	GS_VOID AddPayload( GS_BOOL payload ) { m_PayloadArray.AddItem((GS_BYTE*)new(GSOPType) GS_BOOL(payload)); }
	//GS_VOID AddPayload( GS_CHAR payload )	{ m_PayloadArray.AddItem((GS_BYTE*)new(GSOPType)GS_CHAR>(payload)); }
	//GS_VOID AddPayload( GS_BYTE payload ) { m_PayloadArray.AddItem(new(GSOPType)GS_BYTE>(payload)); }	
	//GS_VOID AddPayload( GS_WORD payload ) { m_PayloadArray.AddItem((GS_BYTE*)new(GSOPType)GS_WORD>(payload)); }
#if defined(_XBOX) || defined(_XENON) || defined(_WINDOWS)
	GS_VOID AddPayload( GS_DWORD payload ) { m_PayloadArray.AddItem((GS_BYTE*)new(GSOPType) GS_DWORD(payload)); }
#elif defined(_PS3)
	GS_VOID AddPayload( GS_INT payload ) { m_PayloadArray.AddItem((GS_BYTE*)new(GSOPType) GS_INT(payload)); }
#endif
	//GS_VOID AddPayload( GS_SIZET payload )	{ m_PayloadArray.AddItem((GS_BYTE*)new(GSOPType) GS_SIZET(payload)); }
	//GS_VOID AddPayload( GS_INT64 payload ) { m_PayloadArray.AddItem((GS_BYTE*)new(GSOPType) GS_INT64(payload)); }
	//GS_VOID AddPayload( GS_UINT64 payload ) { m_PayloadArray.AddItem((GS_BYTE*)new(GSOPType) GS_UINT64(payload)); }
	GS_VOID AddPayload( GS_FLOAT payload ) { m_PayloadArray.AddItem((GS_BYTE*)new(GSOPType) GS_FLOAT(payload)); }		
	GS_VOID AddPayload( GS_DOUBLE payload ) { m_PayloadArray.AddItem((GS_BYTE*)new(GSOPType) GS_DOUBLE(payload)); }
	GS_VOID AddPayload( MessageID payload ) { m_PayloadArray.AddItem((GS_BYTE*)new(GSOPType) MessageID(payload)); }

	void AddTarget(MessageRecipient* target) ;
	MessageRecipient* GetTarget() {return m_recipient;}

	GS_BYTE* ReadPayload(GS_UINT index);

private:
	MessageID m_MessageId;
	TArray<GS_BYTE*> m_PayloadArray;
	MessageRecipient* m_recipient; //when set , this message will be only send to this recipient
};

class MessageRecipient
{
public:
	virtual void MessageResponse(Message* message) = 0;
};

//typedef void (*MessageMgrRegisterFunc)(MessageID messageId);
class MessageHandler
{
public:
	MessageHandler() {}
	MessageHandler(MessageID messageId) { m_MessageId = messageId; }
	void AddHandler(MessageRecipient* recipient);
	void RemoveHandler(MessageRecipient* recipient);
	void ExecuteAll(Message* message);

private:
	MessageID m_MessageId;
	TArray<MessageRecipient*> m_CallbackArray;
};

class MessageMgr
{
public:
	MessageMgr();
	~MessageMgr();
	GS_BOOL Send(Message* message);
	void Register(MessageID messageId, MessageRecipient* recipient);

private:
	MessageHandler* m_MessageHandlers[EMessage_MAX];
};


} // namespace GameService

#endif //GAMESERVICE_ONLIINEMESSAGE_H
