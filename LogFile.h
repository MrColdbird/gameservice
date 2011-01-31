#ifndef __GAMESERVICE_LOGFILE_H__
#define __GAMESERVICE_LOGFILE_H__

namespace GameService
{

enum 
{
    GS_EFileIndex_Log = 0,
    GS_EFileIndex_TmpImg,
    GS_EFileIndex_MAX
};

class LogFile
{
public:
    LogFile(GS_BOOL isDumpLog);
    virtual ~LogFile();

    GS_BOOL DumpLog(const GS_CHAR* log);
    GS_BOOL WriteLocalFile(GS_INT fileIndex, GS_BYTE* data, GS_DWORD size);

    static const GS_CHAR* GetFileName(GS_INT fileIndex);

private:
    static GS_CHAR G_FileName[GS_EFileIndex_MAX][64];

#if defined(_XBOX) || defined(_XENON)
    HANDLE m_hFile[GS_EFileIndex_MAX];
#elif defined(_PS3)
    int m_iFile[GS_EFileIndex_MAX];
#endif
};

} // namespace


#endif //__GAMESERVICE_LOGFILE_H__
