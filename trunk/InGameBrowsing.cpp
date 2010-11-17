#include "stdafx.h"
#ifdef _PS3
#include "interface.h"
#include "master.h"
#include "InGameBrowsing.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <cell/ssl.h>
#include <np/commerce2.h>
#include <netex/net.h>
#include <netex/libnetctl.h>
#include <cell/sysmodule.h>
#include <np.h>


#define HTTP_POOL_SIZE      (512 * 1024)
#define HTTP_COOKIE_POOL_SIZE      (128 * 1024)
#define SSL_POOL_SIZE       (300 * 1024)

namespace GameService
{
void InGameBrowsing_event_handler(uint32_t ctx_id,uint32_t subject_id,int event,int error_code,void *arg)
{
	(void)ctx_id;
	(void)subject_id;
	(void)error_code;
	(void)arg;
#ifdef INGAMEBROWSING
	Master::G()->GetInGameBrowsingSrv()->event_handler(event);
#endif
}

InGameBrowsing::InGameBrowsing()
{
	cid = SYS_MEMORY_CONTAINER_ID_INVALID;
	gi_ctxId = 0;
	phase = PHASE_NONE;
	bInited = false;
	bCreatContextSuccess = false;
}
InGameBrowsing::~InGameBrowsing()
{
	if(bInited)
		Term();
}
void InGameBrowsing::freeCerts()
{
	if(caList.ptr != NULL)
		free(caList.ptr);
}
int InGameBrowsing::loadCerts()
{
	char *buf = NULL;
	size_t size = 0;
	int ret = 0;

	ret = cellSslCertificateLoader(CELL_SSL_LOAD_CERT_ALL, NULL, 0, &size);
	if(ret < 0){
		printf("cellSslCertificateLoader() failed. ret = 0x%x\n", ret);
		goto error;
	}

	memset(&caList, 0, sizeof(caList));
	caList.ptr = (char*)malloc(size);
	if(NULL == caList.ptr){
		printf("malloc failed for cert pinter... \n");
		ret = -1;
		goto error;
	}
	buf = (char*)(caList.ptr);
	caList.size = size;

	ret = cellSslCertificateLoader(CELL_SSL_LOAD_CERT_ALL, buf, size, NULL);
	if(ret < 0){
		printf("cellSslCertificateLoader() failed. ret = 0x%x\n", ret);
		goto error;
	}

	return 0;

error:
	freeCerts();

	return ret;
}
void InGameBrowsing::event_handler(int event)
{

	switch(event){
	case SCE_NP_COMMERCE2_EVENT_REQUEST_ERROR:
	case SCE_NP_COMMERCE2_EVENT_DO_PROD_BROWSE_SUCCESS:
	case SCE_NP_COMMERCE2_EVENT_DO_PROD_BROWSE_BACK:
		phase = PHASE_FINISHED;
		break;
	case SCE_NP_COMMERCE2_EVENT_DO_PROD_BROWSE_FINISHED:
		phase = PHASE_TERMINATED;
		break;
	case SCE_NP_COMMERCE2_EVENT_DO_PROD_BROWSE_STARTED:
	default:
		break;
	}
}


void InGameBrowsing::Init()
{
	int ret;

	//ret = cellSysmoduleLoadModule(CELL_SYSMODULE_NET);
	//if (ret < 0) {
	//	printf("cellSysmoduleLoadModule(CELL_SYSMODULE_NET) failed (0x%x)\n", ret);
	//	goto error;
	//}

	//ret = cellSysmoduleLoadModule(CELL_SYSMODULE_HTTPS);
	//if (ret < 0) {
	//	printf("cellSysmoduleLoadModule(CELL_SYSMODULE_HTTPS) failed (0x%x)\n", ret);
	//	goto error;
	//}

	ret = cellSysmoduleLoadModule(CELL_SYSMODULE_SYSUTIL_NP_COMMERCE2);
	if (ret < 0) {
		printf("cellSysmoduleLoadModule(CELL_SYSMODULE_SYSUTIL_COMMERCE2) failed (0x%x)\n", ret);
		goto error;
	}
	ret = sys_net_initialize_network();
	if (ret < 0) {
		printf("sys_net_initialize_network() failed (0x%x)\n", ret);
		goto error;
	}

	//ret = cellNetCtlInit();
	//if(ret < 0){
	//	goto error;
	//}

	gSslPool = malloc(SSL_POOL_SIZE);
	if(gSslPool == NULL){
		ret = -1;
		goto error;
	}

	ret = cellSslInit(gSslPool, SSL_POOL_SIZE);
	if (ret < 0) {
		printf("cellSslInit() failed (0x%x)\n", ret);
		goto error;
	}

	gHttpPool = malloc(HTTP_POOL_SIZE);
	if(gHttpPool == NULL){
		printf("failed to malloc libhttp memory pool\n");
		ret = -1;
		goto error;
	}
	ret = cellHttpInit(gHttpPool, HTTP_POOL_SIZE);
	if(ret < 0){
		printf("cellHttpInit() failed. ret = 0x%x\n", ret);
		goto error;
	}

	gHttpCookiePool = malloc(HTTP_COOKIE_POOL_SIZE);
	if(gHttpCookiePool == NULL){
		printf("failed to malloc libhttp memory pool\n");
		ret = -1;
		goto error;
	}
	ret = cellHttpInitCookie(gHttpCookiePool, HTTP_COOKIE_POOL_SIZE);
	if(ret < 0){
		printf("cellHttpInitCookie() failed. ret = 0x%x\n", ret);
		goto error;
	}

	loadCerts();
	
	ret = cellHttpsInit(1,&caList);
	if (ret < 0) {
		printf("cellHttpsInit() failed (0x%x)\n", ret);
		goto error;
	}

	ret = sceNpCommerce2Init();
	if (ret < 0) {
		printf("cellSslCertificateLoader() failed (0x%x)\n", ret);
		goto error;
	}
	bInited = true;
	CreatContext4Commerce();
	return;
error:
	printf(" commerce init failed\n");

}
bool InGameBrowsing::CreatContext4Commerce()
{
	int ret;
	SceNpId npId;

	//create a commerce2 context
	ret = sceNpManagerGetNpId(&npId);
	if(ret < 0){
		printf("sceNpManagerGetNpId() failed. ret = 0x%x\n", ret);
		goto error;
	}

	ret = sceNpCommerce2CreateCtx(SCE_NP_COMMERCE2_VERSION, &npId,
	    InGameBrowsing_event_handler, NULL, &gi_ctxId);
	if(ret < 0){
		printf("sceNpCommerce2CreateCtx() failed. ret = 0x%x\n", ret);
		goto error;
	}
	printf("%s: ctxId = %u\n", __FUNCTION__, gi_ctxId);
	bCreatContextSuccess = true;
	return true;
error:
	return false;
}

void InGameBrowsing::Term()
{
	int ret;
	cellHttpsEnd();

	cellSslEnd();
	if(gSslPool != NULL){
		free(gSslPool);
		gSslPool = NULL;
	}

	freeCerts();

	cellHttpEndCookie();
	if(gHttpCookiePool != NULL){
		free(gHttpCookiePool);
		gHttpCookiePool = NULL;
	}
	cellHttpEnd();
	if(gHttpPool != NULL){
		free(gHttpPool);
		gHttpPool = NULL;
	}

	sys_net_finalize_network();

	if(bCreatContextSuccess)
	{
		ret = sceNpCommerce2DestroyCtx(gi_ctxId);
		if (ret < 0) {
			printf("sceNpCommerce2DestroyCtx() failed (0x%x)\n", ret);
		}
	}
	ret = sceNpCommerce2Term();
	if (ret < 0) {
		printf("sceNpCommerce2Term() failed (0x%x)\n", ret);
	}

	//ret = cellSysmoduleUnloadModule(CELL_SYSMODULE_HTTPS);
	//if (ret < 0) {
	//	printf("cellSysmoduleUnloadModule(CELL_SYSMODULE_HTTPS) failed (0x%x)\n", ret);
	//}

	ret = cellSysmoduleUnloadModule(CELL_SYSMODULE_SYSUTIL_NP_COMMERCE2);
	if (ret < 0) {
		printf("cellSysmoduleUnloadModule(CELL_SYSMODULE_SYSUTIL_NP_COMMERCE2) failed (0x%x)\n", ret);
	}

	if(cid != SYS_MEMORY_CONTAINER_ID_INVALID)
		sys_memory_container_destroy(cid);
	cid = SYS_MEMORY_CONTAINER_ID_INVALID;

}
void InGameBrowsing::Update()
{
	if(phase == PHASE_NONE)
		return;

	switch(phase){
	case PHASE_FINISHED:
		sceNpCommerce2DoProductBrowseFinishAsync(gi_ctxId);
		phase = PHASE_TERMINATING;
		break;
	case PHASE_TERMINATED:
		if(cid != SYS_MEMORY_CONTAINER_ID_INVALID)
			sys_memory_container_destroy(cid);
		cid = SYS_MEMORY_CONTAINER_ID_INVALID;
		phase = PHASE_NONE;
		break;
	default:
		break;
	}

	return;
}

void InGameBrowsing::Start()
{
	int ret = 0;
	if(!bCreatContextSuccess)
		CreatContext4Commerce();

	if(!bCreatContextSuccess)
		goto error;

	phase = PHASE_NONE;
	cid = SYS_MEMORY_CONTAINER_ID_INVALID;

	ret = sys_memory_container_create(&cid,
	    SCE_NP_COMMERCE2_DO_PROD_BROWSE_MEMORY_CONTAINER_SIZE);
	if(ret < 0){
		printf("sys_memory_container_create() failed. ret = 0x%x\n", ret);
		goto error;
	}

	ret = sceNpCommerce2DoProductBrowseStartAsync(gi_ctxId,
	    NP_GUI_PRODUCT_ID, cid, NULL);
	if(ret < 0){
		printf("sceNpCommerce2DoProductBrowseStartAsync() failed. ret = 0x%x\n", ret);
		goto error;
	}

	phase = PHASE_RUNNING;

	return;

error:
	phase = PHASE_NONE;
	if(cid != SYS_MEMORY_CONTAINER_ID_INVALID)
		sys_memory_container_destroy(cid);
	cid = SYS_MEMORY_CONTAINER_ID_INVALID;
	printf("error ingame_browse_enter_func\n");
	return;
}


}
#endif