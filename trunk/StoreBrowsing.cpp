#include "stdafx.h"
#ifdef _PS3
#include "interface.h"
#include "master.h"
#include "StoreBrowsing.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <cell/ssl.h>
#include <np/commerce2.h>
#include <netex/net.h>
#include <netex/libnetctl.h>
#include <cell/sysmodule.h>
#include <np.h>


namespace GameService
{
	StoreBrowsing::StoreBrowsing()
	{
		bInited = false;
	}
	StoreBrowsing::~StoreBrowsing()
	{
		if(bInited)
		{
			int ret = 0;
			ret = sceNpCommerce2Term();
			if (ret < 0) {
				printf("sceNpCommerce2Term() failed (0x%x)\n", ret);
			}

			ret = cellSysmoduleUnloadModule(CELL_SYSMODULE_SYSUTIL_NP_COMMERCE2);
			if (ret < 0) {
				printf("cellSysmoduleUnloadModule(CELL_SYSMODULE_SYSUTIL_NP_COMMERCE2) failed (0x%x)\n", ret);
			}
		}
	}

	void StoreBrowsing::Init()
	{
		int ret = 0;
		ret = cellSysmoduleLoadModule(CELL_SYSMODULE_SYSUTIL_NP_COMMERCE2);
		if (ret < 0) {
			printf("cellSysmoduleLoadModule(CELL_SYSMODULE_SYSUTIL_COMMERCE2) failed (0x%x)\n", ret);
			return;
		}
		//sceNpInit()
		ret = sceNpCommerce2Init();
		if (ret < 0) {
			printf("cellSslCertificateLoader() failed (0x%x)\n", ret);
			return;
		}
		bInited = true;
	}
	void StoreBrowsing::Start()
	{
		int ret = 0;
		if(!bInited) 
		{
			printf("inited failed\n");
			return;
		}

		ret = sceNpCommerce2ExecuteStoreBrowse(
		    SCE_NP_COMMERCE2_STORE_BROWSE_TYPE_PRODUCT,
		    NP_GUI_PRODUCT_ID, 0x87654321);
		if(ret < 0){
			printf("sceNpCommerce2ExecuteStoreBrowse() failed. ret = 0x%x\n", ret);
		}
	}
}
#endif
