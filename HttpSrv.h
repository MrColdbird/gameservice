// ======================================================================================
// File         : HttpSrv.h
// Author       : Li Chen 
// Last Change  : 01/21/2011 | 15:37:43 PM | Friday,January
// Description  : 
// ======================================================================================

#ifndef GAMESERVICE_HTTPSRV_H
#define GAMESERVICE_HTTPSRV_H


namespace GameService
{

#if defined(_XBOX)
//-----------------------------------------------------------------------------
// Internal buffer length
//-----------------------------------------------------------------------------
#define HTTP_HOST_IP_STRING_LENGTH  128
#define INIT_BUFFER_SIZE            512
#define TCP_RECV_BUFFER_SIZE        512
#define HTTP_COMMAND_BUFFER_SIZE    512

//--------------------------------------------------------------------------------------
// Name: class MemoryBuffer
// Desc: Memory buffer, automatically expands as needed to hold more data
//--------------------------------------------------------------------------------------
class MemoryBuffer
{
public:

    MemoryBuffer( GS_DWORD dwSize = INIT_BUFFER_SIZE )
    {
        m_pBuffer = NULL;
        m_dwDataLength = 0;
        m_dwBufferSize = 0;

        if( ( dwSize < UINT_MAX ) && ( dwSize != 0 ) )
        {
            // remain malloc for pairing realloc - LiChen
            m_pBuffer = ( BYTE* )malloc( dwSize + 1 );    // one more char, in case when using string funcions
            if( m_pBuffer )
            {
                m_dwBufferSize = dwSize;
                m_pBuffer[0] = 0;
            }
        }
    };

    ~MemoryBuffer()
    {
        Clear();
    };

    // Add chunk of memory to buffer
    BOOL    Add( const void* p, GS_DWORD dwSize )
    {
        if( CheckSize( dwSize ) )
        {
            memcpy( m_pBuffer + m_dwDataLength, p, dwSize );
            m_dwDataLength += dwSize;
            *( m_pBuffer + m_dwDataLength ) = 0;    // fill end zero
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    };

    // Get the data in buffer
    BYTE* GetData() const
    {
        return m_pBuffer;
    };

    // Get the lenght of data in buffer
    GS_DWORD   GetDataLength() const
    {
        return m_dwDataLength;
    };

    // Rewind the data pointer to the begining
    void    Rewind()
    {
        m_dwDataLength = 0; m_pBuffer[ 0 ] = 0;
    };

    GS_VOID Clear()
    {
        if( m_pBuffer )
            // remain free for pairing realloc - LiChen
            free( m_pBuffer );

        m_pBuffer = NULL;
        m_dwDataLength = 0;
        m_dwBufferSize = 0;
    }

private:

    BYTE* m_pBuffer;

    GS_DWORD m_dwDataLength;

    GS_DWORD m_dwBufferSize;

    // Automatically adjust increase buffer size if necessary
    BOOL    CheckSize( GS_DWORD dwSize )
    {
        if( m_dwBufferSize >= ( m_dwDataLength + dwSize ) )
        {
            return TRUE;    // Enough space
        }
        else
        {
            // Try to double it
            GS_DWORD dwNewSize = max( m_dwDataLength + dwSize, m_dwBufferSize * 2 );
            BYTE* pNewBuffer = ( UCHAR* )realloc( m_pBuffer, dwNewSize + 1 );        // one more char
            if( pNewBuffer )
            {
                m_pBuffer = pNewBuffer;
                m_dwBufferSize = dwNewSize;
                return TRUE;
            }
            else
            {
                // Failed
                return FALSE;
            }
        }
    }
};

//-----------------------------------------------------------------------------
// Internal Http buffer
//-----------------------------------------------------------------------------
struct HTTP_BUFFER
{
    MemoryBuffer MB_request;
    MemoryBuffer MB_response;
    CHAR serverName[ HTTP_HOST_IP_STRING_LENGTH ];
    GS_DWORD port;
    WSAOVERLAPPED overlapped;
};

#elif defined(_PS3)

#define USE_PROXY              (0)
#define PROXY_SERVER           "proxy.hq.scei.sony.co.jp:10080"

#define HTTP_POOL_SIZE	       (128 * 1024)
#define INIT_QUEUE_SIZE        (64)
#define BUFFER_SIZE            (512)
#define HTML_BUFFER_MAX_SIZE   (256 * 1024)
#define CONTENT_TYPE_TEXT_HTML "text/html"
/*E depth of links to follow */
#define MAX_DEPTH              (1)

/*E enable/disable pipelining */
#define USE_PIPELINING         (1)
/*E follow images */
#define PARSE_IMG              (1)
/*E follow links */
#define PARSE_HREF             (0)

/*E returns number of elements in array (x) */
#define ArrayCount(x) (sizeof(x)/sizeof(*x))

/* *** */
/*E HTTP Transaction states */
typedef enum {
	HttpTransactionState_Free = 0,	/* available */
	HttpTransactionState_Create,	/* newly created */
	HttpTransactionState_Send,		/* sending request */
	HttpTransactionState_Receive,	/* receiving response */
	HttpTransactionState_Parse,		/* parsing response */
	HttpTransactionState_Destroy,	/* destroying transaction */
	HttpTransactionState_Unknown
} EHttpTransactionState;

/*E HTTP Transaction Management */
typedef struct {
	size_t      depth;          /* */
	uint64_t    nContentLength; /* reported content-length */
	uint64_t    nBytesReceived; /* number of bytes received -- will cancel if > 8MB and HTML */
	size_t      bufferSize;     /* current buffer size */
	CellHttpTransId transID;    /* transaction ID (NULL if available) */
	CellHttpUri uri;            /* transaction URI */
	char       *uriPool;        /* */ 
	char       *buffer;         /* receive buffer -- realloc'ed for HTML only */
	int32_t     cellState;      /* current state */
	int32_t     code;           /* status code */
	EHttpTransactionState state;
	bool        bIsHtml;        /* TRUE if content-type is text/html */
	bool        bHasContentLength; /* True if content-length available */
	uint8_t     reserved[2];
} CHttpTransaction;

/* *** */
/*E HTML parse state manager */
/*E note: not an exhaustive HTML parse state manager,
 * by design, invalid HTML will break state manager
 */
typedef struct {
	CHttpTransaction *httpTrans;
	bool bInHtml;
	bool bInHead;
	bool bInBody;
	bool bInScript;
	uint8_t     reserved[4];
} CHtmlParser;

/* *** */
/*E HTTP Pipeline Queue local state */
/*E !!! Could be instanced instead of static */
typedef struct lsv {

    // intialized in SignIn.cpp:
    // void            *httpPool;            /* libhttp memory pool */
    
	size_t           nNumBytesReceived;   /* total number of bytes received */
	char*            baseURI;             /* base URI for initial request */
	CellHttpClientId clientID;
	CHttpTransaction *transactionQueue;    /* array of in-progress transactions */
	int              nNumTransactions;    /* number of pending transactions */
	int              nMaxNumTransactions; /* current array extent */ 
	int              nTotalTransactions;  /* total number of queued transactions */
} CLSV;
#endif

class HttpSrv
{
public:
    HttpSrv();
    virtual ~HttpSrv();

    GS_BOOL Initialize();
    GS_VOID Finalize();

    GS_VOID AllocSpace(GS_DWORD size) 
    {
        GS_Assert(m_pRecvData == NULL);
        m_pRecvData = GS_NEW GS_BYTE[size];
        m_iRecvSize = 0;
    }
    GS_VOID RecordBuffer(GS_DWORD bytesReceived, GS_BYTE* buffer, GS_DWORD bufSize)
    {
        memcpy(m_pRecvData + bytesReceived, buffer, bufSize);
    }

#if defined(_XBOX)
    enum HTTP_STATUS
    {
        HTTP_STATUS_READY,
        HTTP_STATUS_BUSY,
        HTTP_STATUS_DONE,
        HTTP_STATUS_ERROR
    };

    // Content Data, exclude the HTTP header
    BYTE* GetResponseContentData() const  { return m_pResponseContentData; };

    // Content Data Length, exclude the HTTP header
    GS_DWORD       GetResponseContentDataLength() const  { return m_dwResponseContentDataLength; };

    // Simple status, HTTP_STATUS_BUSY means it's still waiting for the last HTTP response
    HTTP_STATUS GetStatus() { return m_status; };
    VOID        SetStatus( HTTP_STATUS status ) { m_status = status; };

    // HTTP 200, 500 etc
    GS_DWORD       GetResponseCode() { return m_dwResponseCode; };

    // Socket level error
    GS_VOID     ReportError(GS_DWORD errorCode);
    GS_DWORD       GetSocketErrorCode() { return m_dwSocketErrorCode; };
    VOID        SetSocketErrorCode( GS_DWORD dwCode ) { m_dwSocketErrorCode = dwCode; };

    // Internal http buffer
    HTTP_BUFFER& GetInternalBuffer() { return m_buffer; };

#elif defined(_PS3)
    GS_INT HttpTransactionDelete(CHttpTransaction* httpTrans);
    GS_INT HttpTransactionNew(CHttpTransaction* httpTrans, const char* uri, const char* method, size_t depth);
    GS_INT HttpTransactionSend(CHttpTransaction* httpTrans);
    GS_INT QueueUri(const char *uri, const char *method, size_t depth);
    GS_INT RequeueUri(CHttpTransaction* httpTrans, const char *method, const char *problem);
    GS_INT HttpTransactionCheck(CHttpTransaction* httpTrans);
    GS_INT HttpTransactionReceive(CHttpTransaction* httpTrans);

     // GS_BOOL isAbsolutePath(const char* path);
	CHttpTransaction* HttpTransactionFindByID(CellHttpTransId transID);
    GS_INT HttpTransactionParse(CHttpTransaction* httpTrans);
	GS_VOID HtmlReferenceEnqueue(CHttpTransaction* httpTrans, const char* path);
    GS_INT QueueProcess(void);
#endif

    GS_VOID ClearData();
    const GS_CHAR* GetURL() { return m_cURL; }
    GS_VOID StartQuery();
    GS_VOID EndQuery();

    // User interface
    GS_BOOL QueryURL(const char* url);
    GS_BOOL IsLastQueryFinished() 
    {
        if (m_bIsQueryStarted)
        {
            if (!m_bIsInQuery)
            {
                m_bIsQueryStarted = FALSE;
                return TRUE;
            }
        }
        return FALSE;
    }
    GS_BYTE* GetRecvData() { return m_pRecvData; }
    GS_UINT64 GetRecvSize() { return m_iRecvSize; }

private:
    char m_cURL[256];

#if defined(_XBOX)
    HTTP_STATUS m_status;

    // HTTP buffer, include outgoing and incoming buffer
    HTTP_BUFFER m_buffer;

    BYTE* m_pResponseContentData;
    GS_DWORD m_dwResponseContentDataLength;
    GS_DWORD m_dwResponseCode;
    GS_DWORD m_dwSocketErrorCode;
#elif defined(_PS3)
    GS_INT m_iFullQueueWarned;
    CLSV m_lsv;
    system_time_t m_iQueryStartTime;
#endif

    GS_BYTE* m_pRecvData;
    GS_UINT64 m_iRecvSize;
    GS_BOOL m_bIsQueryStarted;
    GS_BOOL m_bIsInQuery;
};

#if defined(_XBOX)
// Worker thread entrance or main processing routine
static GS_DWORD WINAPI HttpSendCommand( LPVOID lpParameter );
#elif defined(_PS3)
static void GS_HttpQueueProcess_Thread(uint64_t instance);
#endif

}

#endif

