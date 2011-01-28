/*  SCE CONFIDENTIAL                                      */
/*  PlayStation(R)3 Programmer Tool Runtime Library 330.001 */
/*  Copyright (C) 2006 Sony Computer Entertainment Inc.   */
/*  All Rights Reserved.                                  */
/*E
 * HTML Lexical Analyzer -
 * Decomposes HTML stream into tags and corresponding attributes
 */

#ifndef __HTTP_PIPELINE_SAMPLE_HTML_LEXER_H__
#define __HTTP_PIPELINE_SAMPLE_HTML_LEXER_H__

namespace GameService
{

/*E attribute name/value pair of an HTML tag */
typedef struct {
	const char *name;			/* attribute name */
	const char *value;			/* attribute value */
} HtmlAttrTuple;

/*E HTML tag and one or more attributes */
typedef struct {
	char* name;			/* tag name */
	int	bIsEndTag;		/* TRUE if end-tag (e.g. </p>) */
	int   nNumAttrs;		/* number of attributes */
	HtmlAttrTuple* attrs;		/* attributes */
} HtmlTag;

/*E Callbacks: */
/*E HtmlTagProcCB - called when an individual tag has been processed */

/*E provides parsed tag to consumer
 * appData - application-specific data -- passed unchanged
 * htmlTag - parsed HTML tag
 */
typedef void (*HtmlTagProcCB)(void *appData, void* appOwner, const HtmlTag *htmlTag);

/*E provides data outside of tag to consumer
 * appData - application-specific data -- passed unchanged
 * data    - data between tags
 * n       - number of bytes of data
 */
typedef void (*HtmlDataProcCB)(void *appData, const char *data, int n);

/*E provides comments to consumer
 * appData - application-specific data -- passed unchanged
 * data    - comment data, including delimiters
 * n       - number of bytes of data
 */
typedef void (*HtmlCommentProcCB)(void *appData,const char *data,int n);

/*E called to process an HTML stream
 * htmlStream    - HTML stream to analyze, must be non-NULL
 * nNumBytes     - number of bytes in HTML stream, must be non-zero
 * flags         - to be passed to callback functions unchanged,
 *                 may be one or more of HtmlLexFlags
 * tagProcCB     - called for each tag and associated attributes,
 *                 must be non-NULL
 * dataProcCB    - called for each data element between tags, may be non-NULL
 * commentProcCB - called for each comment, may be non-NULL
 */
void HtmlLexer(const char *htmlStream, int nNumBytes, void *appData, void* appOwner, HtmlTagProcCB tagProcCB, HtmlDataProcCB dataProcCB, HtmlCommentProcCB commentProcCB);

} // namespace

#endif /* __HTTP_PIPELINE_SAMPLE_HTML_LEXER_H__ */
