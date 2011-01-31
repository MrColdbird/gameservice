// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#include <stdio.h>
#include <assert.h>
#if defined(_XBOX) || defined(_XENON)
#include <xtl.h>
#include <xboxmath.h>
#include <winsockx.h>
#elif defined(_PS3)
#include <cell/sysmodule.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <np.h>
#include <np/drm.h>
#include <np/trophy.h>
#include <sysutil/sysutil_msgdialog.h>
#include <sysutil/sysutil_sysparam.h>
#include <sysutil/sysutil_bgmplayback.h>
#include <sysutil/sysutil_gamecontent.h>
#include <netex/libnetctl.h>
#include <sys/ppu_thread.h>
#include <np/commerce2.h>
#include <sys/memory.h>
#include <cell/http.h>
#include <sys/sys_time.h>
#include <netex/errno.h>
#endif

// TODO: reference additional headers your program requires here

#include "Utils.h"
#include "Customize/CusMemory.h"

