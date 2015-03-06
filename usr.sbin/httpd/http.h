/*	$OpenBSD: http.h,v 1.5.2.1 2014/11/20 07:48:45 jasper Exp $	*/

/*
 * Copyright (c) 2012 - 2014 Reyk Floeter <reyk@openbsd.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _HTTP_H
#define _HTTP_H

#define HTTP_PORT	80
#define HTTPS_PORT	443

enum httpmethod {
	HTTP_METHOD_NONE	= 0,

	/* HTTP/1.1, RFC 2616 */
	HTTP_METHOD_GET,
	HTTP_METHOD_HEAD,
	HTTP_METHOD_POST,
	HTTP_METHOD_PUT,
	HTTP_METHOD_DELETE,
	HTTP_METHOD_OPTIONS,
	HTTP_METHOD_TRACE,
	HTTP_METHOD_CONNECT,

	/* WebDAV, RFC 4918 */
	HTTP_METHOD_PROPFIND,
	HTTP_METHOD_PROPPATCH,
	HTTP_METHOD_MKCOL,
	HTTP_METHOD_COPY,
	HTTP_METHOD_MOVE,
	HTTP_METHOD_LOCK,
	HTTP_METHOD_UNLOCK,

	/* WebDAV Versioning Extension, RFC 3253 */
	HTTP_METHOD_VERSION_CONTROL,
	HTTP_METHOD_REPORT,
	HTTP_METHOD_CHECKOUT,
	HTTP_METHOD_CHECKIN,
	HTTP_METHOD_UNCHECKOUT,
	HTTP_METHOD_MKWORKSPACE,
	HTTP_METHOD_UPDATE,
	HTTP_METHOD_LABEL,
	HTTP_METHOD_MERGE,
	HTTP_METHOD_BASELINE_CONTROL,
	HTTP_METHOD_MKACTIVITY,

	/* WebDAV Ordered Collections, RFC 3648 */
	HTTP_METHOD_ORDERPATCH,

	/* WebDAV Access Control, RFC 3744 */
	HTTP_METHOD_ACL,

	/* WebDAV Redirect Reference Resources, RFC 4437 */
	HTTP_METHOD_MKREDIRECTREF,
	HTTP_METHOD_UPDATEREDIRECTREF,

	/* WebDAV Search, RFC 5323 */
	HTTP_METHOD_SEARCH,

	/* PATCH, RFC 5789 */
	HTTP_METHOD_PATCH,

	/* Server response (internal value) */
	HTTP_METHOD_RESPONSE
};

struct http_method {
	enum httpmethod		 method_id;
	const char		*method_name;
};
#define HTTP_METHODS		{			\
	{ HTTP_METHOD_GET,		"GET" },	\
	{ HTTP_METHOD_HEAD,		"HEAD" },	\
	{ HTTP_METHOD_POST,		"POST" },	\
	{ HTTP_METHOD_PUT,		"PUT" },	\
	{ HTTP_METHOD_DELETE,		"DELETE" },	\
	{ HTTP_METHOD_OPTIONS,		"OPTIONS" },	\
	{ HTTP_METHOD_TRACE,		"TRACE" },	\
	{ HTTP_METHOD_CONNECT,		"CONNECT" },	\
	{ HTTP_METHOD_PROPFIND,		"PROPFIND" },	\
	{ HTTP_METHOD_PROPPATCH,	"PROPPATCH" },	\
	{ HTTP_METHOD_MKCOL,		"MKCOL" },	\
	{ HTTP_METHOD_COPY,		"COPY" },	\
	{ HTTP_METHOD_MOVE,		"MOVE" },	\
	{ HTTP_METHOD_LOCK,		"LOCK" },	\
	{ HTTP_METHOD_UNLOCK,		"UNLOCK" },	\
	{ HTTP_METHOD_VERSION_CONTROL,	"VERSION-CONTROL" }, \
	{ HTTP_METHOD_REPORT,		"REPORT" },	\
	{ HTTP_METHOD_CHECKOUT,		"CHECKOUT" },	\
	{ HTTP_METHOD_CHECKIN,		"CHECKIN" },	\
	{ HTTP_METHOD_UNCHECKOUT,	"UNCHECKOUT" },	\
	{ HTTP_METHOD_MKWORKSPACE,	"MKWORKSPACE" }, \
	{ HTTP_METHOD_UPDATE,		"UPDATE" },	\
	{ HTTP_METHOD_LABEL,		"LABEL" },	\
	{ HTTP_METHOD_MERGE,		"MERGE" },	\
	{ HTTP_METHOD_BASELINE_CONTROL,	"BASELINE-CONTROL" }, \
	{ HTTP_METHOD_MKACTIVITY,	"MKACTIVITY" },	\
	{ HTTP_METHOD_ORDERPATCH,	"ORDERPATCH" },	\
	{ HTTP_METHOD_ACL,		"ACL" },	\
	{ HTTP_METHOD_MKREDIRECTREF,	"MKREDIRECTREF" }, \
	{ HTTP_METHOD_UPDATEREDIRECTREF, "UPDATEREDIRECTREF" }, \
	{ HTTP_METHOD_SEARCH,		"SEARCH" },	\
	{ HTTP_METHOD_PATCH,		"PATCH" },	\
	{ HTTP_METHOD_NONE,		NULL }		\
}

struct http_error {
	int			 error_code;
	const char		*error_name;
};
#define HTTP_ERRORS		{			\
	{ 100,	"Continue" },				\
	{ 101,	"Switching Protocols" },		\
	{ 200,	"OK" },					\
	{ 201,	"Created" },				\
	{ 202,	"Accepted" },				\
	{ 203,	"Non-Authorative Information" },	\
	{ 204,	"No Content" },				\
	{ 205,	"Reset Content" },			\
	{ 206,	"Partial Content" },			\
	{ 300,	"Multiple Choices" },			\
	{ 301,	"Moved Permanently" },			\
	{ 302,	"Moved Temporarily" },			\
	{ 303,	"See Other" },				\
	{ 304,	"Not Modified" },			\
	{ 307,	"Temporary Redirect" },			\
	{ 400,	"Bad Request" },			\
	{ 401,	"Unauthorized" },			\
	{ 402,	"Payment Required" },			\
	{ 403,	"Forbidden" },				\
	{ 404,	"Not Found" },				\
	{ 405,	"Method Not Allowed" },			\
	{ 406,	"Not Acceptable" },			\
	{ 407,	"Proxy Authentication Required" },	\
	{ 408,	"Request Timeout" },			\
	{ 409,	"Conflict" },				\
	{ 410,	"Gone" },				\
	{ 411,	"Length Required" },			\
	{ 412,	"Precondition Failed" },		\
	{ 413,	"Request Entity Too Large" },		\
	{ 414,	"Request-URL Too Long" },		\
	{ 415,	"Unsupported Media Type" },		\
	{ 416,	"Requested Range Not Satisfiable" },	\
	{ 417,	"Expectation Failed" },			\
	{ 500,	"Internal Server Error" },		\
	{ 501,	"Not Implemented" },			\
	{ 502,	"Bad Gateway" },			\
	{ 503,	"Service Unavailable" },		\
	{ 504,	"Gateway Timeout" },			\
	{ 505,	"HTTP Version Not Supported" },		\
	{ 0,	NULL }					\
}

struct http_mediatype {
	char		*media_name;
	char		*media_type;
	char		*media_subtype;
};
/* Some default media types */
#define MEDIA_TYPES		{			\
	{ "css",	"text",		"css" },	\
	{ "html",	"text",		"html" },	\
	{ "txt",	"text",		"plain" },	\
	{ "gif",	"image",	"gif" },	\
	{ "jpeg",	"image",	"jpeg" },	\
	{ "jpg",	"image",	"jpeg" },	\
	{ "png",	"image",	"png" },	\
	{ "js",		"application",	"javascript" },	\
	{ NULL }					\
}

/* Used during runtime */
struct http_descriptor {
	struct kv		 http_pathquery;
	struct kv		 http_matchquery;
#define http_path		 http_pathquery.kv_key
#define http_query		 http_pathquery.kv_value
#define http_rescode		 http_pathquery.kv_key
#define http_resmesg		 http_pathquery.kv_value
#define query_key		 http_matchquery.kv_key
#define query_val		 http_matchquery.kv_value

	char			*http_host;
	enum httpmethod		 http_method;
	int			 http_chunked;
	char			*http_version;

	/* Rewritten path remains NULL if not used */
	char			*http_path_alias;

	/* A tree of headers and attached lists for repeated headers. */
	struct kv		*http_lastheader;
	struct kvtree		 http_headers;
};

#endif /* _HTTP_H */