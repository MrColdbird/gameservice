#include "stdafx.h"
#ifdef _PS3
#include "SignIn.h"
#include "StoreBrowsing.h"
#include "Master.h"
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
				Master::G()->Log("sceNpCommerce2Term() failed (0x%x)\n", ret);
			}

			ret = cellSysmoduleUnloadModule(CELL_SYSMODULE_SYSUTIL_NP_COMMERCE2);
			if (ret < 0) {
				Master::G()->Log("cellSysmoduleUnloadModule(CELL_SYSMODULE_SYSUTIL_NP_COMMERCE2) failed (0x%x)\n", ret);
			}
		}
	}

	void StoreBrowsing::Init()
	{
		int ret = 0;
		ret = cellSysmoduleLoadModule(CELL_SYSMODULE_SYSUTIL_NP_COMMERCE2);
		if (ret < 0) {
			Master::G()->Log("cellSysmoduleLoadModule(CELL_SYSMODULE_SYSUTIL_COMMERCE2) failed (0x%x)\n", ret);
			return;
		}
		//sceNpInit()
		ret = sceNpCommerce2Init();
		if (ret < 0) {
			Master::G()->Log("cellSslCertificateLoader() failed (0x%x)\n", ret);
			return;
		}
		bInited = true;
	}
	void StoreBrowsing::Start()
	{
		int ret = 0;
		if(!bInited) 
		{
			Master::G()->Log("inited failed\n");
			return;
		}

		ret = sceNpCommerce2ExecuteStoreBrowse(
		    SCE_NP_COMMERCE2_STORE_BROWSE_TYPE_PRODUCT,
		    SignIn::GetNpProductID(), 0x87654321);
		if(ret < 0){
			Master::G()->Log("sceNpCommerce2ExecuteStoreBrowse() failed. ret = 0x%x\n", ret);
		}
	}
}
#endif
