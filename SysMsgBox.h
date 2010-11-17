
#ifndef __SYSMSGBOXMANAGER_H__
#define __SYSMSGBOXMANAGER_H__

#if defined(_PS3)
#include <sysutil/sysutil_sysparam.h>
#include <sysutil/sysutil_msgdialog.h>
#define GS_MSGBOX_STRING_MAX CELL_MSGDIALOG_STRING_SIZE
#else
#define GS_MSGBOX_STRING_MAX 512
#endif

namespace GameService
{

enum 
{
	EMODE_IDLE = -1,
    EMODE_SaveDataNoSpace,
    EMODE_TrophyNoSpace,
    EMODE_PlayOtherUserSaveData,
	EMODE_PlayerAgeForbidden,
	EMODE_KeyFileCorrupted,
    EMODE_MAX,
	EMODE_EXIT
};


class SysMsgBoxManager
{
public:
    SysMsgBoxManager();
    virtual ~SysMsgBoxManager();

	void Display(GS_INT mode, void* exdata1 = NULL);
	void Update();

#if defined(_PS3)
	void CB_Dialog( int button_type );
#endif

private:
    GS_INT m_eMode;

    GS_CHAR m_cSysMsgStr[EMODE_MAX][GS_MSGBOX_STRING_MAX];
};


} // namespace GameService
#endif // __SYSMSGBOX_H__
