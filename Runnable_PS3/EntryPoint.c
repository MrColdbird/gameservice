/*  SCE CONFIDENTIAL                                      */
/*  PlayStation(R)3 Programmer Tool Runtime Library 330.001 */
/*  Copyright (C) 2006 Sony Computer Entertainment Inc.   */
/*  All Rights Reserved.                                  */
#include <stdio.h>
#include <sys/process.h>

#include <cell/sysmodule.h>

extern int32_t userMain(void);

SYS_PROCESS_PARAM(1001, 0x10000);

int main(void)
{
	/*E entry point of user program */
	userMain();
	return 0;
}
