#ifndef _HTTPD_H
#define _HTTPD_H            1

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */	

/******************************************************************************
 * Copyrigh (C) 2007 by IOVST
 *
 * File: httpd.h
 *
 * Date: 2007-05-30
 *
 * Author: Liu Yong, <liuyong@iovst.com>
 *
 * Version: 0.1
 *
 * Descriptor:
 *   mininal httpd server
 *
 * Modified:
 *
 ******************************************************************************/
/*-----------------------------------------------------------------------------
  Header files 
------------------------------------------------------------------------------*/
#include <stdio.h>
#include <time.h>
#include <sys/types.h>

#include "ht_def.h"
#include "common.h"
#include "lthread.h"
#include "cgi.h"
/*-----------------------------------------------------------------------------
  Consts
------------------------------------------------------------------------------*/
#define HTTP_CGI_SERVER           (1)    /* Support CGI server  */

/*-----------------------------------------------------------------------------
  Data structures
------------------------------------------------------------------------------*/
/*
 * httpd
 */
typedef struct _httpd_t {
/*
 * Data
 */
	/* Basic httpd data       */
#define HT_START                  (0x00000001)
#define HT_STOP                   (~HT_START)
#define HT_SUP_AUTH               (0x00000010)
	volatile int flag;        /* Server function flag*/

	time_t time;              /* Starting time       */

        /* Configure file object  */
	sys_conf_t *conf;
	char *ver;                /* version informaion  */
	char *wkhome;             /* httpd work directory*/
	char *cgisuffix;          /* CGI suffix          */
	char *rootdef;            /* '/' to default file */
	int thread_min;           /* The Min. threads    */
	int thread_max;           /* The Max. threads    */
	int thread_idle_time;     /* The idle time,Unit:S*/
        int socket_max;           /* Max. client link    */
	int socket_max_time;      /* Max. link time for socket,Unit:S */
	int sess_max_time;        /* Max. session time   */

	net_usa_t server_ip;      /* Server ip address   */
	unsigned int server_port; /* server port         */

	int server_sock;          /* Server socket fd    */

#define HT_TERM_SIG               (1)
	volatile int term_sig;    /* Terminate signal flag*/

	/* Client socket table    */
	ht_socktab_t *socktab;    /* Socket table list    */

	
	thdatatab_t *thdata;      /* thread data list     */
	th_pool_t *th_pool;       /* thread pool          */
	
	/* error table            */
	ht_errtable_t *errtab;
	ht_mime_t     *mime;

	/* CGI pool               */
	cgi_tab_t *cgi;

/*
 * Basic ways
 */
	void (*delete)(struct _httpd_t *);

/*
 * Special ways
 */
	int (*init)(struct _httpd_t *, cgicall_t handler);
	void (*wait)(struct _httpd_t *);
	void (*handle)(struct _httpd_t *);
	void (*finish)(struct _httpd_t *);

	void (*schedule)(struct _httpd_t *);

	int (*set_opt)(struct _httpd_t *, const char *n, const char *v);
	
} httpd_t, *httpd_tp;

/*-----------------------------------------------------------------------------
  Function declaration
------------------------------------------------------------------------------*/
/*
 * Create a new httpd object
 */
extern httpd_t *httpd_new(httpd_t **h);

#ifdef __cplusplus
}
#endif /* __cpluscplus */

#endif /* _HTTPD_H */


