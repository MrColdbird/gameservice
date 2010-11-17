// ======================================================================================
// File         : SignIn.h
// Author       : sun xiao dong 
// Last Change  : 07/29/2010 | 15:48:22 PM | Thursday,July
// Description  : 
// ======================================================================================


#pragma once
#ifndef INGAMEBROWSING_H
#define INGAMEBROWSING_H
#ifdef _PS3
#include <sys/memory.h>
#include <cell/http.h>
namespace GameService
{

class InGameBrowsing
{
public:
	sys_memory_container_t cid;
	uint32_t gi_ctxId;
	void *gSslPool;
	void *gHttpPool;
	void *gHttpCookiePool;
	CellHttpsData caList;
	enum InGameBrowsingPhase{
	PHASE_NONE,
	PHASE_RUNNING,
	PHASE_FINISHED,
	PHASE_TERMINATING,
	PHASE_TERMINATED
	};
	InGameBrowsingPhase phase;
	void event_handler(int event);

	InGameBrowsing();
	~InGameBrowsing();

	void Init();//called after np signed in
	void Term();
	void Update();
	void Start();
	bool IsInited(){ return bInited; }
private:
	bool bInited;
	void freeCerts();
	int loadCerts();
	bool CreatContext4Commerce();
	bool bCreatContextSuccess;
};

} // namespace GameService
#endif

#endif // INGAMEBROWSING_H
