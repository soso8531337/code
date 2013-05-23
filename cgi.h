#ifndef _CGI_H
#define _CGI_H                  1
/******************************************************************************
 * Copyrigh (C) 2007 by IOVST
 *
 * File: cgi.h
 *
 * Date: 2007-05-27
 *
 * Author: Liu Yong, <liuyong@iovst.com>
 *
 * Version: 0.1
 *
 * Descriptor:
 * 
 * Modified:
 *   2007-09-12, Liu Yong, Modify the defination of data structure and function
 *
 ******************************************************************************/

/*-----------------------------------------------------------------------------
  System header files 
------------------------------------------------------------------------------*/
#include <stdarg.h>

/*-----------------------------------------------------------------------------
  Local header files
------------------------------------------------------------------------------*/
#include "ht_def.h"

/*-----------------------------------------------------------------------------
  Const and variables declare
------------------------------------------------------------------------------*/
#define CGI_DEBUG         1
#define CGI_FILE_PROTOCOL "tuanapi.csp"
/*
 * Session
 */
#define CGI_SESSID_NAME           "SESSID"

/*
 * CGI 1.1 canonical metavariables
 */
#define CGI_AUTH_TYPE             "AUTH_TYPE"
#define CGI_CONTENT_LENGTH        "CONTENT_LENGTH"
#define CGI_CONTENT_TYPE          "CONTENT_TYPE"
#define CGI_GATEWAY_INTERFACE     "GATEWAY_INTERFACE"
#define CGI_PATH_INFO             "PATH_INFO"
#define CGI_PATH_TRANSLATED       "PATH_TRANSLATED"
#define CGI_QUERY_STRING          "QUERY_STRING"
#define CGI_REMOTE_ADDR           "REMOTE_ADDR"
#define CGI_REMOTE_HOST           "REMOTE_HOST"
#define CGI_REMOTE_IDENT          "REMOTE_IDENT"
#define CGI_REMOTE_USER           "REMOTE_USER"
#define CGI_REQUEST_METHOD        "REQUEST_METHOD"
#define CGI_SCRIPT_NAME           "SCRIPT_NAME"
#define CGI_SERVER_NAME           "SERVER_NAME"
#define CGI_SERVER_PORT           "SERVER_PORT"
#define CGI_SERVER_PROTOCOL       "SERVER_PROTOCOL"
#define CGI_SERVER_SOFTWARE       "SERVER_SOFTWARE"

/* CATEWAY_INTERFACE values */
#define CGI_GATWAY_INTERFACE_VAL  "CGI/1.1"

/* SERVER_PROTOCOL values   */
#define CGI_SERVER_PROTOCOL_VAL   "HTTP/1.1"

typedef void (*cgihandler_t)(void *);
/*-----------------------------------------------------------------------------
  Data structures
------------------------------------------------------------------------------*/
/*
 * CGI value element
 */
typedef struct _cgi_val_t {
	struct list_head node;      /* value node                   */

	char *name;                 /* cgi value name               */
	char *val;                  /* cgi value string             */
} cgi_val_t, *cgi_val_tp;

/*
 * CGI environment class
 */
typedef struct _cgi_env_t {
/* Data             */
	struct list_head head;      /* environment variable list header */
	int num;                    /* environment variable's number    */

/* Basic ways       */
	void (*delete)(struct _cgi_env_t *);

/* Special ways     */
	int (*add_val)(struct _cgi_env_t *, const char *n, const char *v);
	char *(*get_val)(struct _cgi_env_t *, const char *n);
	void (*destroy)(struct _cgi_env_t *);
	
} cgi_env_t, *cgi_env_tp;

/*
 * CGI record class
 */
typedef struct _cgi_rec_t {
/* Data             */
	struct list_head head;        /* cgi variable list header  */
	int num;                      /* cgi record's number       */

/* Basic ways       */
	void (*delete)(struct _cgi_rec_t *);

/* Special ways     */
	int (*add_val)(struct _cgi_rec_t *, const char *n, const char *v);
	char *(*get_val)(struct _cgi_rec_t *, const char *n);
	char *(*get_val_no)(struct _cgi_rec_t *, const char *n, int no);
	void (*destroy)(struct _cgi_rec_t *);

} cgi_rec_t, *cgi_rec_tp;

/*
 * CGI session class
 */
typedef struct _cgi_sess_t {
        struct list_head  node;      /* next session record         */

/* Data             */
#define SESS_ID_LEN	             (45)
	char sessid[SESS_ID_LEN+1];  /* session id                  */

        struct list_head head;       /* session values list header  */

	int expire;                  /* session expire              */

/* Basic Ways       */
	void (*delete)(struct _cgi_sess_t *);

/* Special ways     */
	/* Check session ID              */
	int (*chk_sid)(struct _cgi_sess_t *, const char *id);
	/* Register a session variable   */
	int (*add_val)(struct _cgi_sess_t *, const char *n, const char *v);
	/* Alter the value of the session variable,
	 * Only alter the shorter than old string.
	 */
	int (*alter_val)(struct _cgi_sess_t *, const char *n, const char *v);
	/* Unregister a session variable */
	int (*del_val)(struct _cgi_sess_t *, const char *n);
	/* Get the value of the session variable
	 * If return NULL when call get_val way, there isn't
         * the variable name in the session list
	 */
	char *(*get_val)(struct _cgi_sess_t *, const char *n);

} cgi_sess_t, *cgi_sess_tp;

/* CGI session table */
typedef struct _cgi_sess_tab_t {
/* Data               */
	struct list_head head; /* Session list    */
	int expire;            /* the expire time */
	pthread_mutex_t lock;  /* Protect:
			        * 1. accessing session list
				* 2. expire time
			        */


/* Basic Ways         */
	void (*delete)(struct _cgi_sess_tab_t *);
	
/* Special Ways       */
	/* Create a new session     */
	cgi_sess_t *(*new)(struct _cgi_sess_tab_t *, char *ssid);
	/* Fee a nession            */
	void (*cfree)(struct _cgi_sess_tab_t *, cgi_sess_t *s);
	/* Find the session according to session ID */
	cgi_sess_t *(*find)(struct _cgi_sess_tab_t *, const char *sid);
	/* Set expire               */
	int (*set_expire)(struct _cgi_sess_tab_t *, int expire);
	/* Check expiring session   */
	void (*chk_expire)(struct _cgi_sess_tab_t *, int sec);

} cgi_sess_tab_t, *cgi_sess_tab_tp;

/*
 * CGI class
 */
#define INIT_OK                       (1)
#define INIT_NO                       (0)
typedef struct _cgi_t {
	struct list_head  node;  /* Record node */

/* Data     */
#define CGI_FLAG_CLR                  (0x00000000)
#define CGI_USED                      (0x00000001)
#define CGI_UNUSED                    (~CGI_USED)
#define CGIH_INIT                     (0x00000040)
#define CGIH_NOT_INIT                 (~CGIH_INIT)
#define SESS_INIT                     (0x00000200)
#define SESS_NOT_INIT                 (~SESS_INIT)
#define CGI_PUTFILE                   (0x00001000)
#define CGI_BUF_OVERFLOW              (0x00002000)
	volatile unsigned int flag;
	/* CGI socket fd                   */
	int sockfd;

	cgihandler_t handler; 

	/* cgi environment variables object */
#define CGI_ENVBUF_LEN                (2096)
	char envbuf[CGI_ENVBUF_LEN];  /* environment buffer  */
	int envlen;
	cgi_env_t *cgienv;

	/* cgi record object  */
#define CGI_RCVBUF_LEN                (8192*10)
	char recbuf[CGI_RCVBUF_LEN];
	int reclen;
	cgi_rec_t *cgirec;

	/* cgi sending buffer */
#define CGI_SNDBUF_LEN                (8192)
	int sendlen;
	char sendbuf[CGI_SNDBUF_LEN];

	/* session object    */
	cgi_sess_tab_t *stab;
#define CGI_SESSBUF_LEN               (1024)
	char sessbuf[CGI_SESSBUF_LEN];
	cgi_sess_t *sess;

/* Basic Ways     */
	void (*delete)(struct _cgi_t *);
	void (*set_sess)(struct _cgi_t *, cgi_sess_t *s);

/* Special Ways    */
	/* CGI     */
	int (*cgi_init)(struct _cgi_t *);
	void (*cgi_end)(struct _cgi_t *);
	void (*cgi_parse_rec)(struct _cgi_t *);
	void (*cgi_parse_env)(struct _cgi_t *);
	int (*cgi_form)(struct _cgi_t *, const char *dpath, int *per);
	char *(*cgi_read_fvar)(struct _cgi_t *, const char *vname);

#define CGI_HEADER_TYPE_HTML           (1)
#define CGI_HEADER_TYPE_XML            (2)
	int (*cgi_init_header)(struct _cgi_t *, unsigned long long len, int type);
	int (*cgi_redirect)(struct _cgi_t *, const char *url);

	/* Session */
	int (*sess_start)(struct _cgi_t *); /* find or create a session */
	int (*sess_output)(struct _cgi_t *); /* Output session header   */
	int (*sess_destroy)(struct _cgi_t *); /* Destroy a session      */
	
	/* I/O     */
	int (*printf)(struct _cgi_t *, const char *fmt, ...);
	int (*vprintf)(struct _cgi_t *, const char *fmt, va_list ap);
	int (*output)(struct _cgi_t *, const char *buf, int len);
	int (*flush)(struct _cgi_t *);

} cgi_t, *cgi_tp;

/* CGI table     */
/*
 * CGI handler type
 */
typedef void (*cgicall_t)(void *);

typedef struct _cgi_tab_t {
/* Data            */
	struct list_head head; /* cgi record list   */
	pthread_mutex_t lock;  /* Protect:
			        * 1. head
			        */

	cgi_sess_tab_t *stab;  /* session table object*/
	cgicall_t handler;

/* Basic Ways      */
	void (*delete)(struct _cgi_tab_t *);
	
/* Special Ways    */
	/* Allocate a unused CGI record     */
	cgi_t *(*alloc)(struct _cgi_tab_t *);
	/* Allocate a used CGI record       */
	void (*cfree)(struct _cgi_tab_t *, cgi_t *);

} cgi_tab_t, *cgi_tab_tp;

/*-----------------------------------------------------------------------------
  Data structures
------------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
  Function declaration
------------------------------------------------------------------------------*/
extern cgi_tab_t *cgi_tab_new(cgi_tab_t **ctab, int max);
void prgcgi_main_handler(void *data);
void default_cgi_handler(void *data);
/*-----------------------------------------------------------------------------
  Macros
------------------------------------------------------------------------------*/

#endif /* _CGI_H */

