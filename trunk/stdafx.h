// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#if defined(_XBOX) || defined(_XENON)
#include <xtl.h>
#include <xboxmath.h>
#include <stdio.h>
#include <assert.h>
#elif defined(_PS3)
#include <cell/sysmodule.h>
#include <assert.h>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <np.h>
#include <np/drm.h>
#include <np/trophy.h>
#include <sysutil/sysutil_msgdialog.h>
#include <sysutil/sysutil_sysparam.h>
#include <sysutil/sysutil_bgmplayback.h>
#include <netex/libnetctl.h>
#include <sys/ppu_thread.h>
#endif

// TODO: reference additional headers your program requires here
#include "Utils.h"
#include "Customize/CusMemory.h"

