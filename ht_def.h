#ifndef _HT_DEF_H
#define _HT_DEF_H                  1

/******************************************************************************
 * Copyrigh (C) 2007 by IOVST
 *
 * File: ht_def.h
 *
 * Date: 2007-06-08
 *
 * Author: Liu Yong, <liuyong@iovst.com>
 *
 * Version: 0.1
 *
 * Descriptor:
 *   Define the internal data structure, const, and variables
 *
 * Modified:
 *
 ******************************************************************************/
/*-----------------------------------------------------------------------------
  System header files 
------------------------------------------------------------------------------*/
#include <stdio.h>
#include <signal.h>
#include <pthread.h>
#include <time.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/*-----------------------------------------------------------------------------
  Local header files
------------------------------------------------------------------------------*/
#include "list.h"
#include "common.h"
/*-----------------------------------------------------------------------------
  Consts
------------------------------------------------------------------------------*/
/* Debug                  */
#define HT_DEBUG                  (1)

/* Module Options         */
#define HT_CGI                    (1)

#define HT_SELECT_TIMEOUT         (1000)           /* ms default 1000*/
#define HT_CONNECT_TIMEOUT        (1000)           /* ms */

#define HT_DEF_LISTEN             (8)

/* vshttpd configure file */
#define HT_CONFFILE               "/etc/xlhttpd.conf"

/* configure file option  */
#define HT_VERSION_OPT            "version"
#define     HT_DEF_VERSION            "vshttpd/0.2"
#define HT_WKHOME_OPT             "work_home"
#define     HT_DEF_HOME               "."
#define HT_SERVER_IP_OPT          "server_ip"
#define HT_SERVER_PORT_OPT        "server_port"
#define     HT_DEF_PORT               "80"
#define HT_CGI_SUFFIX_OPT         "cgi_suffix"
#define     HT_DEF_CGISUFFIX          "csp"
#define HT_ROOT_DEF_OPT           "root_default"
#define     HT_DEF_ROOTDEF            "/index.csp"
#define HT_RUN_USER               "run_user"
#define HT_THREAD_MIN_OPT         "thread_min"
#define     HT_DEF_THREAD_MIN         (1)
#define HT_THREAD_MAX_OPT         "thread_max"
#define     HT_DEF_THREAD_MAX         (2)
#define HT_THREAD_IDLETIME_OPT    "thread_idle_time"
#define     HT_DEF_THREAD_IDLETIME    (60)
#define HT_SOCKET_MAX_OPT         "socket_max"
#define     HT_DEF_SOCKET_MAX         (16)
#define HT_SOCKET_MAX_TIME_OPT    "socket_max_time"
#define     HT_DEF_SOCKET_MAX_TIME    (120*60)
#define HT_SESS_MAX_TIME_OPT      "sess_max_time"
#define     HT_DEF_SESS_MAX_TIME      (30)

/* See RFC1123           */
#define HT_RFC1123FMT             "%a, %d %b %Y %H:%M:%S GMT"
/* 
 * Error code definition, please see rfc2616.pdf, chapter 10.
 */
enum {
	/* Informational 1xx */
	HTTP_CONTINUE         = 100,
	HTTP_SWITCH_PROTOCOLS = 101,
	
	/* Successful 2xx    */
	HTTP_OK               = 200,
	HTTP_CREATED          = 201,
	HTTP_ACCEPTED         = 202,
	HTTP_NON_AUTO         = 203,
	HTTP_NO_CONTENT       = 204,
	HTTP_RESET_CONTENT    = 205,
	HTTP_PARTIAL_CONTENT  = 206,

	/* Redirection 3xx   */
	HTTP_MULTI_CHOICES    = 300,
	HTTP_MOVE_PERM        = 301,
	HTTP_FOUND            = 302,
	HTTP_SEE_OTHER        = 303,
	HTTP_NOT_MODIFIED     = 304,
	HTTP_USE_PROXY        = 305,
	HTTP_TEMP_REDIRECT    = 307,

	/* Client Error 4xx */
	HTTP_BAD_REQ          = 400,
	HTTP_UNAUTH           = 401,
	HTTP_PAYMENT_REQ      = 402,
	HTTP_FORBIDDEN        = 403,
	HTTP_NOT_FOUND        = 404,
	HTTP_METHOD_NOT_ALLOW = 405,
	HTTP_NOT_ACCEPT       = 406,
	HTTP_PROXY_AUTH       = 407,
	HTTP_REQ_TIMEOUT      = 408,
	HTTP_CONFLICT         = 409,
	HTTP_GONE             = 410,
	HTTP_LENGTH_REQ       = 411,
	HTTP_PRECON_FAILED    = 412,
	HTTP_REQ_ENTITY_LARGE = 413,
	HTTP_REQ_URI_LONG     = 414,
	HTTP_UNSUP_MEDIA_TYPE = 415,
	HTTP_REQ_RANGE_NOT_SAT= 416,
	HTTP_EXPECT_FAILED    = 417,

	/* Server Error 5xx  */
	HTTP_INTER_SERVER_ERR = 500,
	HTTP_NOT_IMPLEMENTED  = 501,
	HTTP_BAD_GATEWAY      = 502,
	HTTP_SERV_UNAVAIL     = 503,
	HTTP_GATEWAY_TIMEOUT  = 504,
	HTTP_VER_NOT_SUP      = 505,
};

#define HTTP_MIN_REQ_LEN       (16) /* "GET / http/1.1\n\n" */

/*
 * Method, see RFC2612, Chapter 9(Method Definitions)
 */
#define HTTP_OPTION_METHOD    "OPTIONS"
#define HTTP_GET_METHOD       "GET"
#define HTTP_HEAD_METHOD      "HEAD"
#define HTTP_POST_METHOD      "POST" 
#define HTTP_PUT_METHOD       "PUT"
#define HTTP_DELETE_METHOD    "DELETE"
#define HTTP_TRACE_METHOD     "TRACE"
#define HTTP_CONNECT_METHOD   "CONNECT"

/*
 * Header
 */
/* General header  */
#define HTTP_CACHE_CON_GHDR        "Cache-Control"
#define HTTP_CONNECT_GHDR          "Connection"
#define HTTP_DATE_GHDR             "Date"
#define HTTP_PRAGMA_GHDR           "Pragma"
#define HTTP_TRAILER_GHDR          "Trailer"
#define HTTP_TRAN_ENCODING_GHDR    "Transfer-Encoding"

/* Request header  */
#define HTTP_ACCEPT_RHDR           "Accept"
#define HTTP_ACCEPT_CHAR_RHDR      "Accept-Charset"  
#define HTTP_ACCEPT_ENCODING_RHDR  "Accept-Encoding" 
#define HTTP_ACCEPT_LANG_RHDR      "Accept-Language"
#define HTTP_AUTH_RHDR             "Authorization"  
#define HTTP_EXCEPT_RHDR           "Expect"       
#define HTTP_FORM_RHDR             "From"              
#define HTTP_HOST_RHDR             "Host"
#define HTTP_IF_MATCH_RHDR         "If-Match"
#define HTTP_IF_MODI_SINCE_RHDR    "If-Modified-Since"
#define HTTP_IF_NONE_MATCH_RHDR    "If-None-Match"  
#define HTTP_IF_RANGE_RHDR         "If-Range"       
#define HTTP_IF_UNMODI_SINCE_RHDR  "If-Unmodified-Since"
#define HTTP_MAX_FORW_RHDR         "Max-Forwards"
#define HTTP_PROXY_AUTH_RHDR       "Proxy-Authorization"
#define HTTP_RANGE_RHDR            "Range"
#define HTTP_REF_RHDR              "Referer"
#define HTTP_TE_RHDR               "TE"
#define HTTP_USER_AGENT_RHDR       "User-Agent"

/* Entity header  */
#define HTTP_ALLOW_EHDR            "Allow"
#define HTTP_CONT_ENCODING_EHDR    "Content-Encoding"
#define HTTP_CONT_LANG_EHDR        "Content-Language"
#define HTTP_CONT_LEN_EHDR         "Content-Length"
#define HTTP_CONT_LOC_EHDR         "Content-Location"
#define HTTP_CONT_MD5_EHDR         "Content-MD5"
#define HTTP_CONT_RANGE_EHDR       "Content-Range"
#define HTTP_CONT_DISPOS_EHDR      "Content-Disposition"
#define   HTTP_CONT_DISPOS_FORMDATA  "form-data"
#define HTTP_CONT_TYPE_EHDR        "Content-Type"
#define   HTTP_CONT_APP_TYPE         "application/x-www-form-urlencoded"
#define   HTTP_CONT_MULTIPART_TYPE   "multipart/form-data"
#define   HTTP_CONT_TEXT_TYPE        "text/plain"
#define HTTP_CONT_EXPIRES_EHDR     "Expires"
#define HTTP_CONT_LAST_MODI_EHDR   "Last-Modified"

/* Response header */
#define HTTP_ACCEPT_RANGE_PHDR     "Accept-Ranges"
#define HTTP_ACCEPT_AGE_PHDR       "Age"
#define HTTP_ACCEPT_ETAG_PHDR      "ETag"
#define HTTP_ACCEPT_LOC_PHDR       "Location"
#define HTTP_PROXY_AUTH_PHDR       "Proxy-Authenticate"
#define HTTP_RETRY_AFTER_PHDR      "Retry-After"
#define HTTP_SERVER_PHDR           "Server"
#define HTTP_VARY_PHDR             "Vary"
#define HTTP_WWW_AUTH_PHDR         "WWW-Authenticate"
#define HTTP_UPGRADE_PHDR          "Upgrade"
#define HTTP_VIA_PHDR              "Via"
#define HTTP_WARNING_PHDR          "Warning"

/* Extension header*/
#define HTTP_COOKIE_XHDR           "Cookie"

/*
 * Header Values
 */
/* Connect header values */
#define HTTP_CONNECT_ALIVE         "Keep-Alive"
#define HTTP_CONNECT_CLOSE         "close"

/* Content-type values   */
#define HTTP_MIME_HTML             "html"
#define HTTP_MIME_HTM              "htm"
#define HTTP_MIME_TXT              "txt"
#define HTTP_MIME_CSS              "css"
#define HTTP_MIME_ICO              "ico"
#define HTTP_MIME_GIF              "gif"
#define HTTP_MIME_JPG              "jpg"
#define HTTP_MIME_JPEG             "jpeg"
#define HTTP_MIME_PNG              "png"
#define HTTP_MIME_SVG              "svg"
#define HTTP_MIME_TORRENT          "torrent"
#define HTTP_MIME_WAV              "wav"
#define HTTP_MIME_MP3              "mp3"
#define HTTP_MIME_MID              "mid"
#define HTTP_MIME_M3U              "m3u"
#define HTTP_MIME_RAM              "ram"
#define HTTP_MIME_RA               "ra"
#define HTTP_MIME_DOC              "doc"
#define HTTP_MIME_EXE              "exe"
#define HTTP_MIME_ZIP              "zip"
#define HTTP_MIME_XLS              "xls"
#define HTTP_MIME_TGZ              "tgz"
#define HTTP_MIME_TARGZ            "targz"
#define HTTP_MIME_TAR              "tar"
#define HTTP_MIME_GZ               "gz"
#define HTTP_MIME_ARJ              "arj"
#define HTTP_MIME_RAR              "rar"
#define HTTP_MIME_RTF              "rtf"
#define HTTP_MIME_PDF              "pdf"
#define HTTP_MIME_MPG              "mpg"
#define HTTP_MIME_MPEG             "mpeg"
#define HTTP_MIME_ASF              "asf"
#define HTTP_MIME_AVI              "avi"
#define HTTP_MIME_BMP              "bmp"

/* URL type          */
enum {
	HTTP_URL_TYPE_ERROR = 0,
	HTTP_URL_TYPE_CGI,
	//HTTP_URL_TYPE_DIR,
	HTTP_URL_TYPE_FILE,
};

/* Request HEAD ways */
enum {
	HTTP_REQ_NOT_HEAD = 0,
	HTTP_REQ_HEAD,
};

/*-----------------------------------------------------------------------------
  Data structures
------------------------------------------------------------------------------*/
/*
 * Error type
 */
/* Error record  */
typedef struct ht_err_t {
	int code;          /* error code         */
	const char *name;  /* error name         */
	const char *info;  /* name information   */
} ht_err_t, *ht_err_tp;

/* Error table class  */
struct _thread_data_t;
typedef struct _ht_errtable_t {
/* Data         */
	ht_err_t *records; /* error record arrary */

/* Basic Ways   */
	void (*delete)(struct _ht_errtable_t *);

/* Sepcial Ways */
	void (*send_error)(struct _ht_errtable_t *, int code, 
			   struct _thread_data_t *data);
	const char *(*get_name)(struct _ht_errtable_t *, int code);
	const char *(*get_info)(struct _ht_errtable_t *, int code);

} ht_errtable_t, *ht_errtable_tp;

/*
 * Request mode record
 */
/* base handler function type  */
typedef void *(*ht_handler_t)(void *);

typedef struct _ht_mode_t 
{
	const char *ptr;  /* string pointer */
	int len;          /* string length  */

	ht_handler_t func;/* mode handler   */
} ht_mode_t, *ht_mode_tp;

/*
 * Request header record
 */
typedef struct _ht_hdrrec_t 
{
	struct _ht_hdrrec_t *next; /* next header */
	
	const char *name; /* header name        */
	const char *val;  /* header value       */
	int vallen;       /* header value length*/

	ht_handler_t func;/* handler function   */

} ht_hdrrec_t, *ht_hdrrec_tp;

/* Header class */
typedef struct _ht_header_t
{
/* Data         */
	ht_hdrrec_t *head;
	
/* Basic Ways   */
	void (*delete)(struct _ht_header_t *h);

/* Special Ways */
	int (*create)(struct _ht_header_t *h, char *s);
	int (*remove)(struct _ht_header_t *h, char *n);
	int (*setfunc)(struct _ht_header_t *h, char *n, ht_handler_t func);
	char *(*find)(struct _ht_header_t *h, char *n);

	int (*destroy)(struct _ht_header_t *h);

} ht_header_t, *ht_header_tp;

/*
 * Mime type
 */
/* Mime record type  */
typedef struct _ht_mrec_t {
        const char      *ext;
        int             extlen;
        const char      *mime;
} ht_mrec_t, *ht_mrec_tp;

/* Mime class        */
typedef struct _ht_mime_t 
{
/* Data         */
	ht_mrec_t *type;
	
/* Basic ways   */
	void (*delete)(struct _ht_mime_t *);
	
/* Special ways */
	const char *(*find)(struct _ht_mime_t *, const char *uri);
	
} ht_mime_t, *ht_mime_tp;

/*
 * Thread data
 */
struct _httpd_t;

typedef struct _thread_data_t 
{
	struct list_head node;    /* record node         */

/* Httpd object               */
 	struct _httpd_t *httpd;

	int used;                 /* 1: used
				   * 0: unused
				   */
/* Remote socket information  */
	net_usa_t rmt_ip;         /* remote ip address   */
	int sock;                 /* remote socket FD    */

/* For http request           */
#define THREAD_RCVBUF_LEN         (8192*10) /* byte         */
	char buf[THREAD_RCVBUF_LEN]; /* cache buffer to read*/
	int recvlen;              /* received buffer length */
	int curpos;               /* current position to parse */
	time_t birth_time;        /* Creation time       */

/* Request: 
 * pointer memory share with buf[] */
	ht_mode_t *method;        /* request method      */
	char *uri;          /* URI string          */
	const char *query;        /* QUERY string        */
	int major_ver;            /* major version       */
	int minor_ver;            /* minor version       */
	ht_header_t *rhead;       /* request header list */
	const char *mime_type;    /* Mime type           */
#define THREAD_CON_CLOSE         (0)
#define THREAD_CON_KEEPALIVE     (1)
	int keepalive;            /* keep alive flag     */

/* Respone                    */
	int status;               /* the status code     */
#define THREAD_SNDBUF_LEN        (16384)
	char sbuf[THREAD_SNDBUF_LEN];/*cache buffer to write*/
	int sendlen;              /*send length          */

} thread_data_t, *thread_data_tp;

/* Thread data table          */
typedef struct _thdatatab_t {
	struct list_head list;    /* thread data table list */

	pthread_mutex_t lock;     /* Protect: list          */

/* Ways */
	void (*delete)(struct _thdatatab_t *);
	
	int (*tfree)(struct _thdatatab_t *, thread_data_t *);
	thread_data_t *(*alloc)(struct _thdatatab_t *);

} thdatatab_t, *thdatatab_tp;

/*
 * Socket list
 */
/* Socket record  */
typedef struct _ht_sock_t {
	struct list_head node;   /* socket record node */

	int sock;                /* Socket FD          */
} ht_sock_t, ht_sock_tp;

/* Socket list    */
typedef struct _ht_socktab_t {
	struct list_head head;   /* socket list head   */

	int num;                 /* Socket number      */
	int maxnum;              /* Max socket number  */
	
	pthread_mutex_t lock;    /* Protect:
				  * head (socket list)
				  * num
				  */
	
/* Ways */
	void (*delete)(struct _ht_socktab_t *);
	
	int (*add)(struct _ht_socktab_t *, int fd);
	int (*get)(struct _ht_socktab_t *);
	int (*is_empty)(struct _ht_socktab_t *);
} ht_socktab_t, *ht_socktab_tp;

/*-----------------------------------------------------------------------------
  Function declaration
------------------------------------------------------------------------------*/
/*
 * Create a error table object
 */
extern ht_errtable_t *ht_errtable_new(ht_errtable_t **table);

/*
 * Create a header object
 */
extern ht_header_t *ht_header_new(ht_header_t **head);

/*
 * Create a mime object
 */
extern ht_mime_t *ht_mime_new(ht_mime_t **mime);

/*
 * Create a thread data table
 */
extern thdatatab_t *thdatatab_new(thdatatab_t **tab, int max);

/*
 * Socket list
 */
extern ht_socktab_t *ht_socktab_new(ht_socktab_t **tab, int max);

/*-----------------------------------------------------------------------------
  Macros
------------------------------------------------------------------------------*/

#endif /* _HT_DEF_H */

