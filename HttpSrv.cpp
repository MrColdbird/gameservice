#include "stdafx.h"
#include "Master.h"
#include "HttpSrv.h"
#include "html-lexer.h"



#define PRINT_HTTP_STATE 1

namespace GameService
{

// ======================================================== 
// static functions for callback
// ======================================================== 
#if defined(_PS3)
static int32_t httpAuthProc(CellHttpTransId transId, const char *realm, const CellHttpUri *uri, char *username, char *password, bool *save, void *arg)
{
	transId = transId;
	realm = realm;
	uri = uri;
	username = username;
	password = password;
	save = save;
	arg = arg;
	Master::G()->Log("!!! Basic Authentication not supported.");
	return -1;
}

CHttpTransaction* HttpSrv::HttpTransactionFindByID(CellHttpTransId transID)
{
	int i;
  
	for (i = 0; m_lsv.nNumTransactions && i < m_lsv.nMaxNumTransactions; ++i) {
		if (m_lsv.transactionQueue[i].transID == transID)
			return &m_lsv.transactionQueue[i];
	}
	return NULL;
}

static int32_t httpStatusProc(CellHttpTransId transId, int32_t state, void *arg)
{
#if PRINT_HTTP_STATE
	HttpSrv* owner = (HttpSrv*)arg;
	CHttpTransaction* httpTrans = owner->HttpTransactionFindByID(transId);
	const char* s = NULL;
  
	switch (state) {
	case CELL_HTTP_TRANSACTION_STATE_GETTING_CONNECTION:
		s = "getting connection";
		break;
	case CELL_HTTP_TRANSACTION_STATE_PREPARING_REQUEST:
		s = "preparing request";
		break;
	case CELL_HTTP_TRANSACTION_STATE_SENDING_REQUEST:
		s = "sending request";
		break;
	case CELL_HTTP_TRANSACTION_STATE_SENDING_BODY:
		s = "sending body";
		break;
	case CELL_HTTP_TRANSACTION_STATE_WAITING_FOR_REPLY:
		s = "waiting for reply";
		break;
	case CELL_HTTP_TRANSACTION_STATE_READING_REPLY:
		s = "reading in reply";
		break;
	case CELL_HTTP_TRANSACTION_STATE_SETTING_REDIRECTION:
		s = "redirecting request";
		break;
	case CELL_HTTP_TRANSACTION_STATE_SETTING_AUTHENTICATION:
		s = "authenticating request";
		break;
	default:
		s = "!!! unknown state !!!";
		break;
	}
	Master::G()->Log("transID 0x%x: %s (%d)", transId, s, state);
  
	if (httpTrans) {
		/* ??? */
	}
#else
	(void)transId;
	(void)state;
	(void)arg;
#endif

	return 0;
}
#endif

// ======================================================== 
// HTML static functions
// ======================================================== 
#if defined(_PS3)
static GS_BOOL isAbsolutePath(const char* path)
{
	if (!isalpha(*path))
		return FALSE;
	++path;
	while (*path) {
		if (':' == *path)
			return TRUE;
		if (isalnum(*path) || '+' == *path || '-' == *path || '.' == *path) {
			++path;
			continue;
		}
		return FALSE;
	}
	return FALSE;
}

GS_VOID HttpSrv::HtmlReferenceEnqueue(CHttpTransaction* httpTrans, const char* path)
{
	int r = 0;
	const char* f = NULL;
  
  
	/*E if path is absolute, just queue it -- otherwise,
	 * merge with current uri path and queue it
	 */
	if (isAbsolutePath(path)) {
		QueueUri(path, CELL_HTTP_METHOD_GET, httpTrans->depth + 1);
	} else {
		CellHttpUri uri;
		char *uriPool = NULL;
		char *uriPath = NULL;
		size_t poolSize = 0;
		uint32_t passes;
		//Master::G()->Log("HtmlReferenceEnqueue: %s", path);
		for (passes = 0; passes < 2; ++passes) {
			r = cellHttpUtilMergeUriPath((poolSize)?&uri:NULL,
										 &httpTrans->uri,
										 path,
										 uriPool,
										 poolSize,
										 (poolSize)?NULL:&poolSize);
			if (0 > r) {
				f = "cellHttpUtilMergeUriPath";
				goto abort;
			}
			if (!uriPool && poolSize) {
				//Master::G()->Log(": poolSize = %u", poolSize);
				uriPool = GS_NEW char[poolSize];
			}
		}
		//Master::G()->Log("  %s://%s:%d%s\n", uri.scheme, uri.hostname, uri.port, uri.path);
		/*E TODO: add username and password support and proper port support */
		poolSize = strlen(uri.scheme) + strlen(uri.hostname) + strlen(uri.path) + 20;
		uriPath = GS_NEW char[poolSize];
		snprintf(uriPath, poolSize, "%s://%s:%d%s", uri.scheme, uri.hostname, uri.port, uri.path);
		QueueUri(uriPath, CELL_HTTP_METHOD_GET, httpTrans->depth + 1);
        if (uriPath)
            GS_DELETE [] uriPath;
        if (uriPool)
            GS_DELETE [] uriPool;
	}
  
abort:
	if (r) {
		if (! f)
			f = "<unknown>";
		Master::G()->Log("HtmlReferenceEnqueue: %s: 0x%x", f, r);
	}

	return;
}

static void myHtmlTagProc(void*	appData, void* appOwner, const HtmlTag* htmlTag)
{
	if (htmlTag) {
		static const char* tags[] = {"html", "head", "body", "script", "img", "a"};
		enum {TagHtml, TagHead, TagBody, TagScript, TagImg, TagA, TagUnknown};
		CHtmlParser* htmlParser = (CHtmlParser*)appData;
		HttpSrv* owner = (HttpSrv*)appOwner;
		unsigned ii;

		for (ii = 0; ii < ArrayCount(tags); ++ii) {
			if (0 == strcasecmp(htmlTag->name, tags[ii]))
				break;
		}

		switch (ii) {
		case TagHtml:
			if (!htmlParser->bInScript)
				htmlParser->bInHtml = !htmlTag->bIsEndTag;
			break;
		case TagHead:
			if (!htmlParser->bInScript)
				htmlParser->bInHead = !htmlTag->bIsEndTag;
			break;
		case TagBody:
			//if (!htmlParser->bInScript)
			if (!htmlTag->bIsEndTag) {
				htmlParser->bInBody = true;
				htmlParser->bInScript = false;
				htmlParser->bInHead = false;
			}
			htmlParser->bInBody = !htmlTag->bIsEndTag;
			break;
		case TagScript:
			htmlParser->bInScript = !htmlTag->bIsEndTag;
			break;
		case TagImg:
			/*E pick up references only in <body/> */
#if PARSE_IMG
			if (!htmlParser->bInScript && htmlParser->bInBody) {
				int jj;
				for (jj = 0; jj < htmlTag->nNumAttrs; ++jj) {
					if (0 == strcasecmp(htmlTag->attrs[jj].name, "src")) {
						owner->HtmlReferenceEnqueue(htmlParser->httpTrans, htmlTag->attrs[jj].value);
						break;
					}
				}
			}
#endif /* PARSE_IMG */
			break;
		case TagA:
#if PARSE_HREF
			if (!htmlParser->bInScript && htmlParser->bInBody) {
				int jj;
				for (jj = 0; jj < htmlTag->nNumAttrs; ++jj) {
					if (0 == strcasecmp(htmlTag->attrs[jj].name, "href")) {
						owner->HtmlReferenceEnqueue(htmlParser->httpTrans, htmlTag->attrs[jj].value);
						break;
					}
				}
			}
#endif /* PARSE_HREF */
			break;
		default:
			break;
		}
	}
}
#endif

HttpSrv::HttpSrv()
: 
#if defined(_PS3)
    m_iFullQueueWarned(0), 
#endif
    m_bIsInQuery(FALSE), m_bIsQueryStarted(FALSE), m_pRecvData(NULL), m_iRecvSize(0)
{
}

HttpSrv::~HttpSrv()
{
}

GS_BOOL HttpSrv::Initialize()
{
#if defined(_XBOX)
    m_buffer.port = 80;

    // Create an event handle and setup an overlapped structure.
    m_buffer.overlapped.hEvent = WSACreateEvent();
    if( m_buffer.overlapped.hEvent == NULL )
        m_status = HTTP_STATUS_ERROR;
    else
        m_status = HTTP_STATUS_READY;

#elif defined(_PS3)
	int32_t r = 0;
	const char* f = "<unknown>";
	memset(&m_lsv, 0, sizeof(m_lsv));

	r = cellHttpCreateClient(&m_lsv.clientID);
	if (0 > r) {
		f = "cellHttpCreateClient";
		goto abort;
	}

	/*E set proxy */
#if USE_PROXY
	{
		void *uriPool = NULL;
		CellHttpUri uri;
		size_t poolSize = 0;

		r = cellHttpUtilParseProxy(NULL, PROXY_SERVER, NULL, 0, &poolSize);
		if (0 > r) {
			Master::G()->Log("error parsing proxy... (0x%x)", r);
			goto abort;
		}
		if (NULL == (uriPool = GS_NEW GS_BYTE[poolSize])) {
			Master::G()->Log("error mallocing uriPool (0x%x)", poolSize);
			goto abort;
		}
		r = cellHttpUtilParseProxy(&uri, PROXY_SERVER, uriPool, poolSize, NULL);
		if (0 > r) {
			GS_DELETE [] uriPool;
			Master::G()->Log("error parsing proxy... (0x%x)", r);
			goto abort;
		}

		Master::G()->Log("Setting the proxy to: ");
		Master::G()->Log("  hostname: %s", uri.hostname);
		Master::G()->Log("  port:     %d", uri.port);
		Master::G()->Log("  username: %s", uri.username);
		Master::G()->Log("  password: %s", uri.password);
	
		r = cellHttpSrvSetProxy(m_lsv.clientID, &uri);
		GS_DELETE [] uriPool;
		if (0 > r) {
			Master::G()->Log("failed to set proxy... (0x%x)", r);
			goto abort;
		}
	}
#endif /* USE_PROXY */

	r = cellHttpClientSetAuthenticationCallback(m_lsv.clientID, httpAuthProc, NULL);
	if (0 > r) {
		f = "cellHttpSrvSetAuthenticationCallback";
		goto abort;
	}
	r = cellHttpClientSetTransactionStateCallback(m_lsv.clientID, httpStatusProc, this);
	if (0 > r) {
		f = "cellHttpSrvSetTransactionStateCallback";
		goto abort;
	}
	r = cellHttpClientSetAutoRedirect(m_lsv.clientID, false);
	if (0 > r) {
		f = "cellHttpSrvSetAutoRedirect";
		goto abort;
	}
	r = cellHttpClientSetAutoAuthentication(m_lsv.clientID, false);
	if (0 > r) {
		f = "cellHttpSrvSetAutoAuthentication";
		goto abort;
	}
	r = cellHttpClientSetPipeline(m_lsv.clientID, (USE_PIPELINING)?true:false);
	if (0 > r) {
		f = "cellHttpSrvSetPipeline";
		goto abort;
	}
	m_lsv.transactionQueue = (GameService::CHttpTransaction*)GS_NEW GS_BYTE[sizeof(*m_lsv.transactionQueue) * INIT_QUEUE_SIZE];
	if (NULL == m_lsv.transactionQueue) {
		r = sizeof(CellHttpTransId) * INIT_QUEUE_SIZE;
		f = "malloc";
		goto abort;
	}
	memset(m_lsv.transactionQueue, 0, sizeof(*m_lsv.transactionQueue) * INIT_QUEUE_SIZE);
	m_lsv.nMaxNumTransactions = INIT_QUEUE_SIZE;
	m_lsv.nNumTransactions = 0;
	r = 0;

    return TRUE;

abort:
    Master::G()->Log("HttpSrv Initialize error - %s: 0x%x", f, r);
    return FALSE;
#endif

    return TRUE;
}

GS_VOID HttpSrv::Finalize()
{
    // clear the data
    ClearData();

#if defined(_XBOX)
    WSACloseEvent( m_buffer.overlapped.hEvent );

#elif defined(_PS3)
	if (m_lsv.transactionQueue) {
		int i;
		/*E release any pending transactions */
		for (i = 0; m_lsv.nNumTransactions && i < m_lsv.nMaxNumTransactions; ++i) {
			if (m_lsv.transactionQueue[i].state) {
				HttpTransactionDelete(&m_lsv.transactionQueue[i]);
				--m_lsv.nNumTransactions;
			}
		}
		GS_DELETE [] m_lsv.transactionQueue;
	}
	if (m_lsv.clientID)
		cellHttpDestroyClient(m_lsv.clientID);

	memset(&m_lsv, 0, sizeof(m_lsv));

#endif
}

#if defined(_PS3)
GS_INT HttpSrv::HttpTransactionDelete(CHttpTransaction* httpTrans)
{
	if (httpTrans) {
		httpTrans->state = HttpTransactionState_Destroy;
		if (httpTrans->uriPool)
			GS_DELETE [] httpTrans->uriPool;
		if (httpTrans->buffer)
			GS_DELETE [] httpTrans->buffer;
		if (httpTrans->transID) {
			cellHttpDestroyTransaction(httpTrans->transID);
		}
		memset(httpTrans, 0x00, sizeof(*httpTrans));
		httpTrans->state = HttpTransactionState_Free;
		return 0;
	}
	return -1;
}

GS_INT HttpSrv::HttpTransactionNew(CHttpTransaction* httpTrans, const char* uri, const char* method, size_t depth)
{
	const char* f = "<unknown>";
	int r = -1;
	char* uriBuffer = NULL;
	EHttpTransactionState oldState = HttpTransactionState_Unknown;
  
	if (! httpTrans) {
		f = "httpTrans == NULL";
		goto abort;
	}
  
	oldState = httpTrans->state;
	if (HttpTransactionState_Free == httpTrans->state) {
		size_t uriSize = 0;
		httpTrans->state = HttpTransactionState_Create;
		httpTrans->depth = depth;
		/*E compute storage for parsed URI */
		r = cellHttpUtilParseUri(NULL, uri, NULL, 0, &uriSize);
		if (0 > r) {
			f = "cellHttpUtilParseUri<size query>";
			goto abort;
		}
		uriBuffer = GS_NEW char[uriSize];
		if (! uriBuffer) {
			f = "malloc";
			r = (int)uriSize;
			goto abort;
		}
		r = cellHttpUtilParseUri(&httpTrans->uri, uri, uriBuffer, uriSize, NULL);
		if (0 > r) {
			f = "cellHttpUtilParseUri<final>";
			goto abort;
		}
		if ((httpTrans->uri.scheme == 0)
			|| (*httpTrans->uri.scheme == '\0')
			|| (0 == strncmp(httpTrans->uri.scheme, "http", strlen("http")))
			|| (0 == strncmp(httpTrans->uri.scheme, "https", strlen("https")))) {

			r = cellHttpCreateTransaction(&httpTrans->transID, m_lsv.clientID, method, &httpTrans->uri);
			if (0 > r) {
				f = "cellHttpCreateTransaction";
				goto abort;
			}
			r = 0;
		} else {
			f = "invalid HTTP URI";
			r = -1;
		}
		httpTrans->uriPool = uriBuffer;
	}
	else {
		f = "httpTrans not Free";
		r = -1;
	}

abort:
	if (r) {
		if (uriBuffer)
			GS_DELETE [] uriBuffer;
		if (httpTrans)
			httpTrans->state = oldState;
		Master::G()->Log("HttpTransactionNew: %s: 0x%x for %s", f, r, uri);
		r = -1;
	}
	return r;
}

GS_INT HttpSrv::HttpTransactionSend(CHttpTransaction* httpTrans)
{
	int r;

	r = cellHttpSendRequest(httpTrans->transID, NULL, 0, NULL);
	if (0 > r) {
		if (r != CELL_HTTP_ERROR_NO_CONNECTION) {
			Master::G()->Log("HttpTransactionSend: failed to complete HTTP transaction: 0x%x", r);
			httpTrans->state = HttpTransactionState_Destroy;
			r = -1;
		} else {
			r = 1;
		}
	}
	else {
		httpTrans->state = HttpTransactionState_Send;
		r = 0;
	}
  
	return r;
}

GS_INT HttpSrv::QueueUri(const char *uri, const char *method, size_t depth)
{
	int r = 0;
	CHttpTransaction*	trans = NULL;
	int i;
  
	/*E room in transaction queue? */
	if (m_lsv.nNumTransactions == m_lsv.nMaxNumTransactions) {
		if (!m_iFullQueueWarned) {
			Master::G()->Log("httpQueueUri: queue is full");
			m_iFullQueueWarned = 1;
		}
		return -1;
	} else {
		m_iFullQueueWarned = 0;
	}
  
	/*E find an available transaction slot */
	for (i = 0; i < m_lsv.nMaxNumTransactions; ++i) {
		if (HttpTransactionState_Free == m_lsv.transactionQueue[i].state) {
			trans = &m_lsv.transactionQueue[i];
			break;
		}
	}
  
	r = HttpTransactionNew(trans, uri, method, depth);
	if (r) {
		return -1;
	}

	++m_lsv.nNumTransactions;
	++m_lsv.nTotalTransactions;
	return 0;
}

GS_INT HttpSrv::RequeueUri(CHttpTransaction* httpTrans, const char *method, const char *problem)
{
	/*E pipe was broken or connection was reset,
	 * so try to put it back in the queue
	 */
	char *uriPath = NULL;
	const char *result = NULL;
	size_t poolSize = 0;
	int32_t r;

	(void)problem;

	poolSize = strlen(httpTrans->uri.scheme) + strlen(httpTrans->uri.hostname) + strlen(httpTrans->uri.path) + 20;
	uriPath = GS_NEW char[poolSize];
	snprintf(uriPath, poolSize, "%s://%s:%d%s", httpTrans->uri.scheme, httpTrans->uri.hostname, httpTrans->uri.port, httpTrans->uri.path);
	r = QueueUri(uriPath, method, httpTrans->depth);
	if (r == 0) {
		//result = "requeued";
	} else {
		result = "dropped";
        Master::G()->Log("httpRequeueUri: The transaction's %s, but the request was %s: %s", problem, result, uriPath);
	}

	GS_DELETE [] uriPath;
	return 0;
}

GS_INT HttpSrv::HttpTransactionCheck(CHttpTransaction* httpTrans)
{
	int r = 0;
	const char* f = NULL;
	CellHttpHeader httpHeader;
	char buffer[256];
  
	/*E check HTTP status */
	r = cellHttpResponseGetStatusCode(httpTrans->transID, &httpTrans->code);
	if (0 > r) {
		if (r == CELL_HTTP_ERROR_BROKEN_PIPELINE) {
			RequeueUri(httpTrans, CELL_HTTP_METHOD_GET, "pipe was broken");
			httpTrans->state = HttpTransactionState_Destroy;
		} else if (r == CELL_HTTP_NET_ERROR(CELL_HTTP_ERROR_NET_RECV, SYS_NET_ECONNRESET)) {
			RequeueUri(httpTrans, CELL_HTTP_METHOD_GET, "connection was reset");
			httpTrans->state = HttpTransactionState_Destroy;
		} else if (r != CELL_HTTP_ERROR_OUT_OF_ORDER_PIPE) {
			f = "cellHttpResponseGetStatusCode";
			httpTrans->state = HttpTransactionState_Destroy;
		} else {
			r = 0;
		}
		goto abort;
	}
  
	/*E check for Content-Length: */
	r = cellHttpResponseGetContentLength(httpTrans->transID, &httpTrans->nContentLength);
	if (0 > r) {
		if (CELL_HTTP_ERROR_NO_CONTENT_LENGTH == r) {
			//Master::G()->Log("** no content length **");
			httpTrans->nContentLength = 0;
			httpTrans->bHasContentLength = false;
			r = 0;
		} else {
			f = "cellHttpResponseGetContentLength";
			httpTrans->state = HttpTransactionState_Destroy;
			goto abort;
		}
	} else {
		httpTrans->bHasContentLength = true;
	}
  
	/*E check for Content-Type */
	httpHeader.name = NULL;
	httpHeader.value = NULL;
	r = cellHttpResponseGetHeader(httpTrans->transID, &httpHeader, "Content-Type", buffer, sizeof(buffer), NULL);
	if (0 > r) {
		f = "cellHttpRequestGetHeader(Content-Type)";
		if (CELL_HTTP_ERROR_NO_HEADER != r) {
			httpTrans->state = HttpTransactionState_Destroy;
			goto abort;
		} else {
			httpHeader.value = NULL;
			r = 0;
		}
	}
	if (httpHeader.value && 0 == strncmp(httpHeader.value, CONTENT_TYPE_TEXT_HTML, strlen(CONTENT_TYPE_TEXT_HTML))) {
		httpTrans->bIsHtml = true;
	} else {
		httpTrans->bIsHtml = false;
	}
  
	/*E ready to go to next stage */
	httpTrans->state = HttpTransactionState_Receive;
  
abort:
	if (r) {
		if (! f)
			f = "<unknown>";
		if ((r != CELL_HTTP_ERROR_BROKEN_PIPELINE)
			&& (r != CELL_HTTP_NET_ERROR(CELL_HTTP_ERROR_NET_RECV, SYS_NET_ECONNRESET))) {
			Master::G()->Log("HttpTransactionCheck: %s: 0x%x from %s://%s:%d%s", f, r,
				   (httpTrans->uri.scheme == "")?httpTrans->uri.scheme:"http",
				   httpTrans->uri.hostname, httpTrans->uri.port,
				   httpTrans->uri.path);
		}
		r = -1;
	}

	return r;
}

GS_INT HttpSrv::HttpTransactionReceive(CHttpTransaction* httpTrans)
{
	int r = 0;
	char buffer[BUFFER_SIZE];
	char* p = buffer;
	size_t bufSize = sizeof(buffer);
	const char* f = NULL;
  
	/*E TODO: make buffer management general instead of special-case
	 * need a receive buffer? -- Note: only HTML gets a malloc'ed buffer
	 */
	if (httpTrans->bIsHtml) {
		if (httpTrans->bHasContentLength) {
			if (httpTrans->nContentLength <= HTML_BUFFER_MAX_SIZE) {
				bufSize = httpTrans->nContentLength;
			} else {
				bufSize = HTML_BUFFER_MAX_SIZE;
			}
		} else {
			bufSize = HTML_BUFFER_MAX_SIZE;
		}
		p = GS_NEW char[bufSize];
		if (! p) {
			f = "malloc";
			goto abort;
		}
		httpTrans->buffer = p;
		httpTrans->bufferSize = bufSize;
	}

    // initialize receive data
    AllocSpace(httpTrans->nContentLength);

	do {
		size_t recvd = 0;
		r = cellHttpRecvResponse(httpTrans->transID, p, bufSize, &recvd);
		//Master::G()->Log(".");
		if (0 > r) {
			f = "cellHttpRecvResponse";
			if (r == CELL_HTTP_ERROR_BROKEN_PIPELINE) {
				RequeueUri(httpTrans, CELL_HTTP_METHOD_GET, "pipe was broken");
			} else if (r == CELL_HTTP_NET_ERROR(CELL_HTTP_ERROR_NET_RECV, SYS_NET_ECONNRESET)) {
				RequeueUri(httpTrans, CELL_HTTP_METHOD_GET, "connection was reset");
			}
			httpTrans->state = HttpTransactionState_Destroy;
			goto abort;
		}
		/*E no more data for this transaction? */
		if (0 == recvd) {
			/*E TODO: if content-length doesn't match received, error? */
			break;
		}

        // record data locally:
        RecordBuffer((GS_DWORD)httpTrans->nBytesReceived, (GS_BYTE*)p, recvd);
		httpTrans->nBytesReceived += recvd;
		m_lsv.nNumBytesReceived += recvd;
    
		/*E if we have a dynamic buffer, but have not filled it,
		 * advance through the buffer
		 */
		if (p != buffer) {
			if (recvd <= bufSize) {
				p += recvd;
				bufSize -= recvd;
			}
      
			/*E we have received as much as our dynamic buffer will allow
			 * -- receive the rest into disposable
			 */
			if (0 == bufSize) {
				//Master::G()->Log("\nHttpTransactionReceive: warning Content-Length > dynamic buffer size\n");
				p = buffer;
				bufSize = sizeof(buffer);
			}
		}
	}
	while ((!httpTrans->bHasContentLength) || (httpTrans->nBytesReceived < httpTrans->nContentLength));

    m_iRecvSize = httpTrans->nBytesReceived;

	if (httpTrans->bHasContentLength && httpTrans->nBytesReceived != httpTrans->nContentLength) {
		Master::G()->Log("content-length = %llu, received = %llu", httpTrans->nContentLength, httpTrans->nBytesReceived);
	}
  
	/*E next stage */
	httpTrans->state = HttpTransactionState_Parse;

	Master::G()->Log("HttpTransactionReceive: (%d) received %llu bytes from %s://%s:%d%s",
		   httpTrans->code,
		   httpTrans->nBytesReceived,
		   (httpTrans->uri.scheme == "")?httpTrans->uri.scheme:"http",
		   httpTrans->uri.hostname, httpTrans->uri.port, httpTrans->uri.path);
  
abort:
	if (r) {
		if (! f)
			f = "<unknown>";
		if ((r != CELL_HTTP_ERROR_BROKEN_PIPELINE)
			&& (r != CELL_HTTP_NET_ERROR(CELL_HTTP_ERROR_NET_RECV, SYS_NET_ECONNRESET))) {
			Master::G()->Log("HttpTransactionReceive: %s: 0x%x", f, r);
		} else {
			r = 0;
		}

        ClearData();
	}


	return r;
}



GS_INT HttpSrv::HttpTransactionParse(CHttpTransaction* httpTrans)
{
	if (httpTrans->bIsHtml && httpTrans->buffer && httpTrans->depth < MAX_DEPTH) {
		CHtmlParser htmlParser;
		int bufferSize;
		if (httpTrans->bufferSize <= httpTrans->nBytesReceived) {
			bufferSize = httpTrans->bufferSize;
		} else {
			bufferSize = httpTrans->nBytesReceived;
		}
		htmlParser.httpTrans = httpTrans;
		htmlParser.bInHtml = false;
		htmlParser.bInHead = false;
		htmlParser.bInBody = false;
		htmlParser.bInScript = false;
		HtmlLexer(httpTrans->buffer, bufferSize, &htmlParser, this, myHtmlTagProc, NULL, NULL);
	}
	httpTrans->state = HttpTransactionState_Destroy;
	return 0;
}

GS_INT HttpSrv::QueueProcess(void)
{
	int i;
	int b;
	int r;
  
	for (i = 0; m_lsv.nNumTransactions && i < m_lsv.nMaxNumTransactions; ++i ) {
		r = 0;
		switch (m_lsv.transactionQueue[i].state) {
		/*E available */
		case HttpTransactionState_Free:
			break;
		/*E newly created -- send it */
		case HttpTransactionState_Create:
			r = HttpTransactionSend(&m_lsv.transactionQueue[i]);
			break;
		/*E request send -- check status */
		case HttpTransactionState_Send:
			r = HttpTransactionCheck(&m_lsv.transactionQueue[i]);
			break;
		/*E receiving response */
		case HttpTransactionState_Receive:
			b = HttpTransactionReceive(&m_lsv.transactionQueue[i]);
			if (b > 0)
				m_lsv.nNumBytesReceived += b;
			break;
		/*E parsing response (for HTML content only) */
		case HttpTransactionState_Parse:
			r = HttpTransactionParse(&m_lsv.transactionQueue[i]);
			break;
		/*E destroying transaction -- all data received and processed */
		case HttpTransactionState_Destroy:
			r = HttpTransactionDelete(&m_lsv.transactionQueue[i]);
			--m_lsv.nNumTransactions;
			break;
		/*E should never occur under normal circumstances */
		case HttpTransactionState_Unknown:
			Master::G()->Log("HttpQueueProcess: transaction %d in Unknown state", i);
			break;
		default:
			Master::G()->Log("HttpQueueProcess: transaction %d in undefined state", i);
			break;
		}
		if (r < 0) {
			Master::G()->Log("failed to process Http request [%p]", m_lsv.transactionQueue[i].transID);
		}
	}

	return m_lsv.nNumTransactions;
}
#elif defined(_XBOX)
GS_DWORD WINAPI HttpSendCommand( LPVOID lpParameter )
{
    Master::G()->Log( "HttpSendCommand" );

    HttpSrv* pHttpSrv = ( HttpSrv* )lpParameter;

    if( !pHttpSrv )
        return S_FALSE;

    pHttpSrv->StartQuery();

    // internal buffer
    HTTP_BUFFER& httpBuffer = pHttpSrv->GetInternalBuffer();

    int nErrorCode;

    // Create TCP/IP socket
    SOCKET hSocket;
    hSocket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
    if( ( hSocket == SOCKET_ERROR ) || ( hSocket == INVALID_SOCKET ) )
    {
        nErrorCode = WSAGetLastError();
        pHttpSrv->ReportError(nErrorCode);
        return nErrorCode;
    }

    sockaddr_in httpServerAdd;

    httpServerAdd.sin_family = AF_INET;
    httpServerAdd.sin_port = htons( httpBuffer.port );
    httpServerAdd.sin_addr.s_addr = inet_addr( httpBuffer.serverName );

    XNDNS* pxndns = NULL;

    if( httpServerAdd.sin_addr.s_addr == INADDR_NONE )        // Hostname
    {
        //dns LOOKUP
        XNetDnsLookup( httpBuffer.serverName, httpBuffer.overlapped.hEvent, &pxndns );
        WaitForSingleObject( httpBuffer.overlapped.hEvent, INFINITE );
        WSAResetEvent( httpBuffer.overlapped.hEvent );
        if( pxndns->iStatus != 0 )
        {
            //  An error occurred.  One of the following:
            //  pxndns->iStatus == WSAHOST_NOT_FOUND - No such host
            //  pxndns->iStatus == WSAETIMEDOUT - No response from DNS server( s )
            nErrorCode = pxndns->iStatus;
            XNetDnsRelease( pxndns );
            shutdown( hSocket, SD_BOTH );
            closesocket( hSocket );
            pHttpSrv->ReportError(nErrorCode);// DNS lookup fail
            return nErrorCode;
        }

        UINT i;
        BOOL bConnection = FALSE;
        for( i = 0; i < pxndns->cina; ++i )
        {
            httpServerAdd.sin_addr = pxndns->aina[ i ];
            if( connect( hSocket, ( struct sockaddr* )&httpServerAdd, sizeof( httpServerAdd ) ) == 0 )
            {
                bConnection = TRUE;
                break;
            }
        }

        XNetDnsRelease( pxndns );

        if( !bConnection )        // we already tried all the IPs
        {
            nErrorCode = WSAGetLastError();
            shutdown( hSocket, SD_BOTH );
            closesocket( hSocket );
            pHttpSrv->ReportError(nErrorCode);
            return nErrorCode;
        }

    }
    else    // IP
    {
        if( connect( hSocket, ( struct sockaddr* )&httpServerAdd, sizeof( httpServerAdd ) ) != 0 )
        {
            nErrorCode = WSAGetLastError();
            shutdown( hSocket, SD_BOTH );
            closesocket( hSocket );
            pHttpSrv->ReportError(nErrorCode);
            return nErrorCode;
        }
    }

    if( SOCKET_ERROR != send( hSocket, ( const char* )httpBuffer.MB_request.GetData(),
                              httpBuffer.MB_request.GetDataLength(), 0 ) )
    {
        int nSize = 0;
        char buff[TCP_RECV_BUFFER_SIZE];
        httpBuffer.MB_response.Rewind();

        while( ( nSize = recv( hSocket, buff, TCP_RECV_BUFFER_SIZE, 0 ) ) != 0 )
        {
            httpBuffer.MB_response.Add( buff, nSize );
        }

        shutdown( hSocket, SD_BOTH );
        closesocket( hSocket );

        pHttpSrv->SetSocketErrorCode( ERROR_SUCCESS );                    // socket OK
        pHttpSrv->EndQuery();
        return S_OK;
    }
    else
    {
        shutdown( hSocket, SD_BOTH );
        closesocket( hSocket );
        pHttpSrv->ReportError(WSAGetLastError());
        return S_FALSE;
    }
}

GS_VOID HttpSrv::ReportError(GS_DWORD errorCode)
{
    SetSocketErrorCode( errorCode );
    SetStatus( HttpSrv::HTTP_STATUS_ERROR );    // Always put this last

    ClearData();
    EndQuery();
}
#endif

GS_VOID HttpSrv::ClearData()
{
    // for xbox360, it may need clear data by
    // m_buffer.MB_response.Clear();
#if defined(_PS3)
    if (m_pRecvData)
    {
        GS_DELETE [] m_pRecvData;
    }
#endif
    m_pRecvData = NULL;
    m_iRecvSize = 0;
}

GS_VOID HttpSrv::StartQuery()
{
#if defined(_XBOX)
#elif defined(_PS3)
    m_iQueryStartTime = sys_time_get_system_time();
    QueueUri(m_cURL, CELL_HTTP_METHOD_GET, 0);
#endif
}

GS_VOID HttpSrv::EndQuery()
{
#if defined(_XBOX)
    m_pRecvData = m_buffer.MB_response.GetData();
    m_iRecvSize = m_buffer.MB_response.GetDataLength();
#elif defined(_PS3)
    system_time_t total_time = sys_time_get_system_time() - m_iQueryStartTime;

    Master::G()->Log("%u byte%s received in %d transaction%s over %lld.%06lld seconds",
		   m_lsv.nNumBytesReceived, (m_lsv.nNumBytesReceived == 1)?"":"s",
		   m_lsv.nTotalTransactions, (m_lsv.nTotalTransactions == 1)?"":"s",
		   total_time / (1000 * 1000), total_time % (1000 * 1000));
#endif

    m_bIsInQuery = FALSE;
}

#if defined(_PS3)
void GS_HttpQueueProcess_Thread(uint64_t instance)
{
    HttpSrv* httpsrv_inst = (HttpSrv*)instance;
    
    httpsrv_inst->StartQuery();

    while(httpsrv_inst->QueueProcess());

    httpsrv_inst->EndQuery();

    sys_ppu_thread_exit(0);
}
#endif

// ======================================================== 
// User Interface 
// ======================================================== 
GS_BOOL HttpSrv::QueryURL(const char* url)
{
    if (m_bIsInQuery)
        return FALSE;

    m_bIsQueryStarted = TRUE;
    m_bIsInQuery = TRUE;

    ClearData();

#if defined(_XBOX)
    // seperate URL into server name and file name for 360 socket function restriction
    char cmdBuff[ HTTP_COMMAND_BUFFER_SIZE ];
    char server_name[128];
    char file_name[128] = "/";

    // check "://"
    char seps[] = ":/";
    char* token;
    strcpy_s( server_name, url );
    token = strtok( server_name, seps );
    // now token should be "http", server_name updated to "www...."
    token = strtok( server_name, seps );
    strcat_s(file_name, server_name);
    strcpy_s(server_name, token);

    sprintf_s( cmdBuff, "GET %s HTTP/1.0 \r\n\r\n", file_name );
    m_buffer.MB_request.Rewind();
    m_buffer.MB_request.Add( cmdBuff, strlen( cmdBuff ) );
    strcpy_s( m_buffer.serverName, server_name );

    // start worker thread
    HANDLE hThread = CreateThread( NULL, 0, HttpSendCommand, ( VOID* )this, 0, NULL );
    if( hThread )
    {
        CloseHandle( hThread );
        return TRUE;
    }

    return FALSE;

#elif defined(_PS3)
    strcpy(m_cURL, url);

    sys_ppu_thread_t temp_id;
	int ret = -1;
    ret = sys_ppu_thread_create(
        &temp_id, GS_HttpQueueProcess_Thread,
        (uintptr_t) this, THREAD_PRIO, STACK_SIZE,
        0, "GameService HttpQueueProcess Thread");
    if (ret < 0) {
        Master::G()->Log("[GameService] - sys_ppu_thread_create() failed (%x)", ret);
        return FALSE;
    }

    return TRUE;
#endif

}

} // namespace

