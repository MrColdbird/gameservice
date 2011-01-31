/*  SCE CONFIDENTIAL                                      */
/*  PlayStation(R)3 Programmer Tool Runtime Library 330.001 */
/*  Copyright (C) 2006 Sony Computer Entertainment Inc.   */
/*  All Rights Reserved.                                  */
/*E
 * HTML Lexical Analyzer -
 * Decomposes HTML stream into tags and corresponding attributes
 */

#include "stdafx.h"

#if defined(_PS3)

#define DEBUG 0

#include "html-lexer.h"

/*E minimum attr count */
#define NUM_INIT_ATTRS (32)

/*E minimum pool chunk size */
#define POOL_CHUNK_SIZE (2048)

namespace GameService
{

typedef struct myBufferPool *myBufferPoolPtr;
typedef struct myBufferPool {
	void           *buffer;         // buffer
	int             nBufferSize;    // size of buffer in bytes
	int             nAvailable;     // bytes available in buffer
	void           *pNextAvailable; // next available slot in buffer
	myBufferPoolPtr pNextPool;      // linked list of pools
} myBufferPool;

/*E create a pool chunk */
static myBufferPoolPtr getPoolChunk(int initSize)
{
	myBufferPoolPtr pool = NULL;
	int n = initSize + sizeof(myBufferPool);
	pool = (myBufferPool*)GS_NEW GS_BYTE[n];
	if (pool) {
		pool->buffer = ((char*)pool) + sizeof(myBufferPool);
		pool->nBufferSize = initSize;
		pool->nAvailable = initSize;
		pool->pNextAvailable = pool->buffer;
		pool->pNextPool = NULL;
	}
	return pool;
}

/*E reset a pool -- all chunks are available for use */
static void resetPool(myBufferPoolPtr pool)
{
	while (pool) {
		pool->nAvailable = pool->nBufferSize;
		pool->pNextAvailable = pool->buffer;
		pool = pool->pNextPool;
	}
}

/*E free a pool -- all chunks are released */
static void freePool(myBufferPoolPtr pool)
{
	if (pool) {
		if (pool->pNextPool)
			freePool(pool->pNextPool);
		GS_DELETE [] pool;
	}
}

/*E allocate a portion of the pool, extending the pool if neccessary */
/*E returns: pointer to object's location in pool, NULL if no memory available */
static char* allocFromPool(myBufferPoolPtr pool, int n)
{
	if (pool) {
		/*E find an available pool chunk large enough for object */
		while (1) {
			if (n <= pool->nAvailable) {
				void* p = pool->pNextAvailable;
				pool->nAvailable -= n;
				pool->pNextAvailable = ((char*)p) + n;
				return (char*)p;
			}
			/*E check each pool chunk in turn */
			else if (pool->pNextPool) {
				pool = pool->pNextPool;
			}
			/*E no pool chunk large enough or with enough free space,
			 * extend pool by adding a chunk
			 */
			else {
				pool->pNextPool = getPoolChunk(n*2 > POOL_CHUNK_SIZE ? n*2 : POOL_CHUNK_SIZE);
				if (pool->pNextPool) {
					pool = pool->pNextPool;
				} else {
					break;
				}
			}
		}
	}
	return NULL;
}

/*E copy and NUL-terminate string to pool */
static char* cleanCopyToPool(myBufferPoolPtr pool, const char* s, int len)
{
	char* p = allocFromPool(pool, len + 1);
	if (p) {
		char* d = p;
		while(*s && len) {
			*d++ = *s++;
			len--;
		}
		*d = 0;
	}
	return p;
}

/*E scan for the end of comment, return NULL if not found */
static const char* findCommentEnd(const char* p, const char* end)
{
	while (p < end-2) {
		if (p[0] == '-' && p[1] == '-') {
			p += 2;
			while (p < end && isspace(p[0]))
				++p;
			if (p < end && p[0] == '>')
				return p+1;
		}
		++p;
	}
	return NULL;
}

/*E *** */
/* HtmlLexer -- helper macros (should be inline !!!) */
/* *** */

/*E advance to next character in stream,
 * goto x if at or beyond the end of the stream
 */
#define NextOrAbort(p,e) { ++(p); if ((p) >= (e)) goto abort; }

/*E test p for comment/declaration start */
#define IsCommentTag(p,e) ( ((p) + 3 <= (e)) && ((p)[0] == '!' && (p)[1] == '-' && (p)[2] == '-') )

/*E test p for end tag */
#define xIsEndTag(c) ( (c) == '/' )

/*E test p for being valid tag character: non-whitespace, not oneof: =, >, / */
//#define IsValidChar(p) ( (p)[0] > ' ' && (p)[0] <= '~' && (p)[0] != '=' && (p)[0] != '>' && (p)[0] != '/' )
#define xIsValidChar(c) ( (c) > ' ' && (c) <= '~' && (c) != '=' && (c) != '>' && (c) != '/' )

/*E test p for tag close */
#define xIsTagClose(c) ( (c) == '>' )

/*E test p for assignment operator */
#define xIsAssignOp(c) ( (c) == '=' )

/*E test p for allowed quote characters */
#define xIsQuote(c) ( (c) == '"' || (c) == '\'' )

/*E skip over whitespace */
#define SkipWhitespace(p,e) { while(isspace(*(p))) NextOrAbort((p),(e)); }

/*E dispatch tag processing */
#define doHtmlTagProc(cb, a, b, h) ( cb(a, b, h) )

/*E dispatch data processing */
#define doHtmlDataProcCB(cb, a, d, n) { if (cb) cb(a, d, n); }

/*E dispatch comment processing */
#define doHtmlCommentProcCB(cb, a, d, n) { if (cb) cb(a, d, n); }

#if DEBUG
static int IsEndTag(char c) { return xIsEndTag(c); }
static int IsValidChar(char c) { return xIsValidChar(c); }
static int IsTagClose(char c) { return xIsTagClose(c); }
static int IsAssignOp(char c) { return xIsAssignOp(c); }
static int IsQuote(char c) { return xIsQuote(c); }
#else
#define IsEndTag xIsEndTag
#define IsValidChar xIsValidChar
#define IsTagClose xIsTagClose
#define IsAssignOp xIsAssignOp
#define IsQuote xIsQuote
#endif

/*E analyze input buffer, extracting tags, comments and data elements
 * elements are provided to caller via callbacks,
 * all but the tag callback are optional
 * htmlStream    - HTML stream to analyze
 * nNumBytes     - number of bytes in HTML stream
 * appData       - to be passed to callback functions unchanged
 * tagProcCB     - called for each tag and associated attributes
 * dataProcCB    - called for each data element found between tags
 * commentProcCB - called for each comment
 */
void HtmlLexer(const char *htmlStream, int nNumBytes, void *appData, void* appOwner, HtmlTagProcCB tagProcCB, HtmlDataProcCB dataProcCB, HtmlCommentProcCB commentProcCB)
{
	myBufferPoolPtr pool = NULL;
	//HtmlAttrTuple* attrs = NULL;
	int nMaxAttrs = NUM_INIT_ATTRS;
	HtmlTag tag;
	const char* p = htmlStream;
	const char* endStream = htmlStream + nNumBytes;
	const char* rewindPoint = p;
	const char* dataStart = p;
	const char* tagStart = NULL;
	const char* tagEnd = NULL;
	tag.attrs = NULL;

	/*E sanity checks on required inputs */
	if (!htmlStream) goto abort;
	if (!nNumBytes) goto abort;
	if (!tagProcCB) goto abort;

	/*E create buffer pool, allocate attribute array buffer
	 * and initialize state
	 */
	pool = getPoolChunk(POOL_CHUNK_SIZE);
	memset(&tag, 0, sizeof(tag));
	tag.attrs = (HtmlAttrTuple*)GS_NEW GS_BYTE[nMaxAttrs * sizeof(HtmlAttrTuple)];
  
	/*E iterate over stream */
scanForTag:
	while (p < endStream) {
  
		/*E reset tag */
		tag.nNumAttrs = 0;
		tag.bIsEndTag = 0;
		tag.name = NULL;
		resetPool(pool);
    
		/*E look for beginning of a tag */
		//while (p < endStream) {
		p = (char*)memchr(p, '<', endStream - p);
		if ( ! p )
			goto abort;
		rewindPoint = p;
		NextOrAbort(p, endStream);

		/*E Identify tag type: begin, end or declaration/comment */
		if (IsCommentTag(p, endStream)) {
			const char* endComment = findCommentEnd(p + 3, endStream);
			if (endComment) {
				doHtmlCommentProcCB(commentProcCB, appData, rewindPoint, endComment-rewindPoint);
				p = endComment;
				dataStart = p;
			}
			continue; // goto scanForTag;
		} else if (IsEndTag(*p)) {
			tag.bIsEndTag = 1;
			NextOrAbort(p, endStream);
		}

		/*E we appear to be in a tag--accumulate name */
		tagStart = p;
		while (IsValidChar(*p))
			NextOrAbort(p, endStream);
		tagEnd = p;
		SkipWhitespace(p, endStream);

		/*E if not a valid end tag, ignore */
		if (tag.bIsEndTag && ! IsTagClose(*p)) {
			p = tagStart + 1;
			continue; // goto scanForTag;
		}

		/*E we have a tag -- check for validity */
		tag.name = cleanCopyToPool(pool, tagStart, tagEnd-tagStart);
		//tagIsValid = doHtmlTagIsValidCB(tagIsValidCB, appData, tag.name);

		/*E send pending data to consumer */
		if (dataStart < rewindPoint) {
			doHtmlDataProcCB(dataProcCB, appData, dataStart, rewindPoint-dataStart);
			dataStart = rewindPoint;
		}

		/* *** */
		/*E scan for 0 or more attributes */
		while (p < endStream) {
			const char *attrNameStart;
			const char *attrNameEnd;
			const char *attrValueStart;
			const char *attrValueEnd;

			SkipWhitespace(p, endStream);

			/*E self-ending tag?  (eg. <b/>) */
			if (IsEndTag(*p)) {
				NextOrAbort(p, endStream);
				SkipWhitespace(p, endStream);
				/*E not a valid tag, abandon */
				if (! IsTagClose(*p)) {
					p = tagStart + 1;
					goto scanForTag;
				}
			}

			/*E end of tag? done scanning for attributes */
			if (IsTagClose(*p)) {
				break;
			}

			/*E delimit attribute name */
			attrNameStart = p;
			while (IsValidChar(*p)) {
				NextOrAbort(p, endStream);
			}
			attrNameEnd = p;

			/*E scan optional attribute value */
			SkipWhitespace(p, endStream);

			/*E shorthand attribute
			 * (eg. <ul compact> is analogous to <ul compact="compact">) ?
			 */
			if (IsValidChar(*p) || IsEndTag(*p) || IsTagClose(*p)) {
				attrValueStart = attrNameStart;
				attrValueEnd = attrNameEnd;
			}
			/*E found assignment operator, now scan for value */
			else if (IsAssignOp(*p)) {
				NextOrAbort(p, endStream);
				SkipWhitespace(p, endStream);

				/*E quote, either " or ', delimited? */
				if (IsQuote(*p)) {
					int foundNewline = 0;
					char delimiter = *p;
					NextOrAbort(p, endStream);
					attrValueStart = p;

					/*E scan for closing quote */
					while (delimiter != *p) {
						/*E newline in quotes may invalidate tag,
						 * terminating at delimiter or >
						 * if we terminate at >, the tag is invalid
						 */
						if (!foundNewline && '\n' == *p) {
							/*E rescan from after delimiter */
							p = attrValueStart;
							foundNewline = 1;
						} else if (foundNewline && IsTagClose(*p)) {
							break;
						}
						NextOrAbort(p, endStream);
					} /* delimiter */
					attrValueEnd = p;

					/*E found closing quote? */
					if (delimiter == *p) {
						NextOrAbort(p, endStream);
					}
					/*E nope -- invalidate tag */
					else {
						goto scanForTag;
					}
				}
				/*E bare value --
				 * collect everything up to next space or tag close
				 */
				else {
					attrValueStart = p;
					while (!isspace(*p) && !IsTagClose(*p))
						NextOrAbort(p, endStream);
					attrValueEnd = p;
					/*E missing value, abandon tag */
					if (attrValueStart == attrValueEnd) {
						/*E and we're nested enough to be painful */
						p = tagStart + 1;
						goto scanForTag;
					}
				}
			}
			/*E unexpected character, abandon tag */
			else {
				p = tagStart + 1;
				goto scanForTag;
			}

			/*E accumulate attributes -- expand attribute array, if needed */
			if (nMaxAttrs == tag.nNumAttrs) {
				nMaxAttrs *= 2;

				// +change realloc into delete->new - LiChen
				if (tag.attrs)
					GS_DELETE [] tag.attrs;
				tag.attrs = (HtmlAttrTuple*)GS_NEW GS_BYTE [nMaxAttrs * sizeof(HtmlAttrTuple)];
				// -change end

				if (NULL == tag.attrs)
					goto abort;
			}
			tag.attrs[tag.nNumAttrs].name = cleanCopyToPool(pool, attrNameStart, attrNameEnd-attrNameStart);
			if (NULL == tag.attrs[tag.nNumAttrs].name)
				goto abort;
			tag.attrs[tag.nNumAttrs].value = cleanCopyToPool(pool, attrValueStart, attrValueEnd-attrValueStart);
			if (NULL == tag.attrs[tag.nNumAttrs].value)
				goto abort;
			++tag.nNumAttrs;
		} /* scan for tag attributes */

		/*E provide tag and attributes to consumer, if valid */
		doHtmlTagProc(tagProcCB, appData, appOwner, &tag);
		dataStart = p+1;

		/*E advance to next available character */
		NextOrAbort(p, endStream);

	} /* scan HTML stream */
  
abort:
	if (dataStart < endStream)
		doHtmlDataProcCB(dataProcCB, appData, dataStart, endStream-dataStart);
	if (pool) { freePool(pool); }
	if (tag.attrs) { GS_DELETE [] tag.attrs; }
}

} // namespace

#endif