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
	//void AddPayload( void* payload ) { }			
	void AddPayload( bool payload ) { m_PayloadArray.AddItem((GS_BYTE*)new(GSOPType) bool(payload)); }
	void AddPayload( char payload )	{ m_PayloadArray.AddItem((GS_BYTE*)new(GSOPType) char(payload)); }
	void AddPayload( signed char payload ) { m_PayloadArray.AddItem((GS_BYTE*)new(GSOPType) signed char(payload)); }
	void AddPayload( unsigned char payload ) { m_PayloadArray.AddItem((GS_BYTE*)new(GSOPType) unsigned char(payload)); }	
	void AddPayload( short payload ) { m_PayloadArray.AddItem((GS_BYTE*)new(GSOPType) short(payload)); }
	void AddPayload( unsigned short payload ) { m_PayloadArray.AddItem((GS_BYTE*)new(GSOPType) unsigned short(payload)); }
	void AddPayload( int payload ) { m_PayloadArray.AddItem((GS_BYTE*)new(GSOPType) int(payload)); }
	void AddPayload( unsigned int payload )	{ m_PayloadArray.AddItem((GS_BYTE*)new(GSOPType) unsigned int(payload)); }
	void AddPayload( long payload )	{ m_PayloadArray.AddItem((GS_BYTE*)new(GSOPType) long(payload)); }
	void AddPayload( unsigned long payload ) { m_PayloadArray.AddItem((GS_BYTE*)new(GSOPType) unsigned long(payload)); }
	void AddPayload( long long payload ) { m_PayloadArray.AddItem((GS_BYTE*)new(GSOPType) long long(payload)); }
	void AddPayload( unsigned long long payload ) { m_PayloadArray.AddItem((GS_BYTE*)new(GSOPType) unsigned long long(payload)); }
	void AddPayload( float payload ) { m_PayloadArray.AddItem((GS_BYTE*)new(GSOPType) float(payload)); }		
	void AddPayload( double payload ) { m_PayloadArray.AddItem((GS_BYTE*)new(GSOPType) double(payload)); }
	void AddPayload( MessageID payload ) { m_PayloadArray.AddItem((GS_BYTE*)new(GSOPType) MessageID(payload)); }
	//void AddPayload( char* payload )

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
