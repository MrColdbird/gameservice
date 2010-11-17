#ifndef __GAMESERVICE_LOGFILE_H__
#define __GAMESERVICE_LOGFILE_H__

namespace GameService
{

class LogFile
{
public:
    LogFile();
    virtual ~LogFile();

    GS_BOOL Write(const GS_CHAR* log);

private:
    static GS_CHAR G_LogFileName[64];
#if defined(_XBOX) || defined(_XENON)
    HANDLE m_hFile;
#elif defined(_PS3)
    int m_iFile;
#endif
};

} // namespace


#endif //__GAMESERVICE_LOGFILE_H__
