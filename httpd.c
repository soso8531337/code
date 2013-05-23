/******************************************************************************
 * Copyright (C) 2007 by IOVST
 * 
 * File: httpd.c
 *
 * Date: 2011 - 06 - 28
 *
 * Author: Wang Songnian
 *
 * Descriptor:
 *   
 * Note:
 *  mininal httpd server
 *
 * Version: 0.1
 *
 * Modified:
 *
 ******************************************************************************/
/*-----------------------------------------------------------------------------
  System header files 
------------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <string.h>
#include <pwd.h>
#include <errno.h>
#include <ctype.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>

/*-----------------------------------------------------------------------------
  Local header files
------------------------------------------------------------------------------*/
#include "httpd.h"
/*-----------------------------------------------------------------------------
  Global variables and constants
------------------------------------------------------------------------------*/
static httpd_t *ht = NULL;

/* 
 * See RFC2612, Chapter 9(Method Definitions) 
 */
 int ht_cgi_do_post(thread_data_t *data);
 int ht_cgi_do_get(thread_data_t *data);
void ht_read_file(thread_data_t *data, int head);


static void *ht_handle_get_method(void *);
static void *ht_handle_post_method(void *);
static void *ht_handle_head_method(void *);

static ht_mode_t http_methods[] = {
	{HTTP_GET_METHOD, sizeof(HTTP_GET_METHOD), ht_handle_get_method},
	{HTTP_POST_METHOD, sizeof(HTTP_POST_METHOD), ht_handle_post_method},
	{HTTP_HEAD_METHOD, sizeof(HTTP_HEAD_METHOD), ht_handle_head_method},
	{NULL, 0, NULL}
};


/*-----------------------------------------------------------------------------
  Functions Declare
------------------------------------------------------------------------------*/
/*
 * httpd ways' functions
 */
static void httpd_delete(httpd_t *h);
static int httpd_init(httpd_t *h, cgicall_t handler);
static void httpd_wait(httpd_t *h);
static void httpd_handle(httpd_t *h);
static void httpd_finish(httpd_t *h);
static void httpd_schedule(httpd_t *h);
static int httpd_set_opt(httpd_t *h, const char *n, const char *v);

/* httpd thread handler */
static void *httpd_thread_handler(void *d);
static int httpd_parse_request(thread_data_t *data);
static int httpd_parse_request_line(thread_data_t *data);
static int httpd_parse_request_header(thread_data_t *data);
static void httpd_rm_double_dots(char *s);
static int httpd_handle_request(thread_data_t *data);
static int httpd_get_line(thread_data_t *data);

/*
 * httpd other functions
 */
static int httpd_read_conf(httpd_t *h);
static int httpd_open_socket(httpd_t *h);
static int ht_get_uri_type(thread_data_t *data);

/*
 * httpd header
 */
static void ht_header_delete(ht_header_t *header);
static int ht_header_create(ht_header_t *header, char *s);
static int ht_header_remove(ht_header_t *header, char *n);
static int ht_header_setfunc(ht_header_t *header, char *n, ht_handler_t func);
static char *ht_header_find(ht_header_t *header, char *n);
static int ht_header_destroy(ht_header_t *header);

/* 
 * Thread data and pool
 */
static void thdatatab_delete(thdatatab_t *tab);
static thread_data_t *thdatatab_alloc(thdatatab_t *tab);
static int thdatatab_free(thdatatab_t *tab, thread_data_t *data);
static thread_data_t *thdata_new(void);
static void thdata_init(thread_data_t *tmp);
static void thdata_clear(thread_data_t *t);
static void thdata_delete(thread_data_t *tmp);

/*
 * Socket list
 */
static void ht_socktab_delete(ht_socktab_t *tab);
static int ht_socktab_add(ht_socktab_t *tab, int fd);
static int ht_socktab_get(ht_socktab_t *tab);
static int ht_socktab_is_empty(ht_socktab_t *tab);


//static int is_xl_deamon(int fd);
/*-----------------------------------------------------------------------------
  Functions
------------------------------------------------------------------------------*/
/* Purpose: Create a new httpd object
 * In     : **h: the pointer to store the httpd object
 * Return : the httpd object pointer
 *          NULL: error
 */
httpd_t *httpd_new(httpd_t **h)
{
	httpd_t *tmp;

	/* Allocate the space       */
        if ((tmp = calloc(1, sizeof(httpd_t))) == NULL) {
		return NULL;
        } else {
		*h = tmp;
		ht = tmp; /* Only signal handler */
	}
	
        /* Initialize the ways */
	tmp->delete   = httpd_delete;
	
	tmp->init     = httpd_init;
	tmp->wait     = httpd_wait;
	tmp->handle   = httpd_handle;
	tmp->finish   = httpd_finish;
	tmp->schedule = httpd_schedule;
	tmp->set_opt  = httpd_set_opt;

	return tmp;
}

/* Purpose: delete httpd object
 * In     : *h: httpd object pointer
 * Return : void
 */
static void httpd_delete(httpd_t *h)
{
	if (h != NULL) {
		if (h->server_sock != -1) {
			close(h->server_sock);
		}

		if(h->ver)
			free(h->ver);

		if(h->wkhome)
			free(h->wkhome);

		if(h->cgisuffix)
			free(h->cgisuffix);

		if(h->rootdef)
			free(h->rootdef);
		free(h);
	}
}

/* Purpose: initialize the http server
 *    1. read configure file and fill httpd information
 *    2. set process ID
 *    3. set signal handlers
 *    4. initialize file handle
 *    5. initialize dir handle
 *    6. initialize ssl handle
 *    7. initialize cgi handle
 *    8. create thread data pool
 *    9. create thread pool
 *    10. open server socket and listening
 *
 * In     : *h: httpd server object
 *          *ftab: cgi file table.
 *                 Is NULL if isn't support cgi file table
 *
 * Return : FAILURE(0): error
 *          SUCCESS(1): ok
 */
static int httpd_init(httpd_t *h, cgicall_t handler)
{
	/* Create a configure file object */

	printf("httpd_init starts\n");
	if ((h->conf = sys_conf_new(&h->conf)) == NULL) {
		return FAILURE;
	}

	h->conf->fname(h->conf, HT_CONFFILE);

	if (!httpd_read_conf(h)) {
		goto HTTPD_ERR;
	}
	
	/* Initialize file handle */

	/* Initialize dir handle  */

	/* Initialize ssl handle  */

	/* Initialize cgi handle  */
	h->cgi = cgi_tab_new(&h->cgi, h->socket_max);
	if (h->cgi == NULL) {
		goto HTTPD_ERR;
	}
	h->cgi->stab->set_expire(h->cgi->stab, h->sess_max_time);
	h->cgi->handler = handler;


	printf("httpinit init 235\n");
	/* Initialize httpd error table    */
	h->errtab = ht_errtable_new(&h->errtab);
	if (h->errtab == NULL) {
		goto HTTPD_ERR;
	}
	
	/* Initialize mime type            */
	h->mime = ht_mime_new(&h->mime);
	if (h->mime == NULL) {
		goto HTTPD_ERR;
	}

        /* Create socket pool     */
	h->socktab = ht_socktab_new(&h->socktab, h->socket_max);
	if (h->socktab == NULL) {
		goto HTTPD_ERR;
	}
	printf("httpd init 251\n");	
	/* Create a thread data pool */
//	h->thdata = thdatatab_new(&h->thdata, h->thread_max);
	h->thdata = thdatatab_new(&h->thdata, h->socket_max);
	if (h->thdata == NULL) {
		goto HTTPD_ERR;
	}

	/* Create a thread pool      */
	h->th_pool = th_pool_new(&h->th_pool, h->thread_min,
				 h->thread_max, h->thread_idle_time);
	if (h->th_pool == NULL) {
		goto HTTPD_ERR;
	}
	printf("httpd init 267\n");
	/* Create a socket                 */
	if (!httpd_open_socket(h)) {
		printf("ERR\n");
		goto HTTPD_ERR;
	}
	printf("http init 272\n");
	/* Start to receive socket request */
	h->flag |= HT_START;
	printf("httpd init success\n");
	return SUCCESS;

HTTPD_ERR:
	httpd_finish(h);
	return FAILURE;
}

/* Purpose: waiting for a new connect
 *    1. waiting new client link, return when waited or timeout
 *    2. waited a client link, call handle(), back to 1
 *    3. free some unused thread records in the pool and
 *       free some unused thread data records in the pool
 *    4. if received a singal, exit server
 * In     : *h: httpd server object
 * Return : void
 */
static void httpd_wait(httpd_t *h)
{
	fd_set readfd;
	struct timeval tv;
	int ret;
        
	cgi_sess_tab_t *stab = h->cgi->stab;

	while (1) {
		FD_ZERO(&readfd);
		FD_SET(h->server_sock, &readfd);
		tv.tv_sec = HT_SELECT_TIMEOUT / 1000; /* second */
		tv.tv_usec = HT_SELECT_TIMEOUT % 1000;/* us     */

		/* Now waiting a connect */
		ret = select(h->server_sock+1, &readfd, 0, 0, &tv);
		if (ret == 0) {
			/* Check the expire for cgi session */
			stab->chk_expire(stab, HT_SELECT_TIMEOUT / 1000);

			if (h->term_sig & HT_TERM_SIG) {
				h->flag &= HT_STOP;
				break;
			}

			/* If socket isn't empty, schedule  */
			if (!h->socktab->is_empty(h->socktab)) {
				h->schedule(h);
			}
		} else if (ret > 0) {
			/* Handle the new connect           */
			if (FD_ISSET(h->server_sock, &readfd)) {
				h->handle(h);
				continue;
			}
		} else {
			/* Interrupt by signal              */
			if (h->term_sig & HT_TERM_SIG) {
				h->flag &= HT_STOP;
				break;
			}
		}
	}
}

/* Purpose: handle the new connect
 *       1. Accept the new socket connect
 *       2. Add the new socket fd into socket list
 * In     : *h: httpd server object
 * Return : void
 */
static void httpd_handle(httpd_t *h)
{
	net_usa_t rmt_ip;
	int sockfd = -1, flag;

	/* Accept the remote socket    */
	rmt_ip.len = sizeof(struct sockaddr_in);
	sockfd = accept(h->server_sock, &rmt_ip.sa, &rmt_ip.len);
	if (sockfd < 0) {
		goto HANDLE_ERR;
	}

	/* Open keep alive      */
	flag = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE,
		   (void *)&flag, sizeof(flag));

	/* Set the non blocking mode   */
        if (!fd_noblock(sockfd)) {
		goto HANDLE_ERR;
        }

        if (!fd_cloexec(sockfd)) {
		goto HANDLE_ERR;
        }
	/* Add the fd into socket list */
        if (!h->socktab->add(h->socktab, sockfd)) {             
		goto HANDLE_ERR;
        }

	/* Call schedule function      */
	h->schedule(h);
	
	return;

HANDLE_ERR:
        if (sockfd != -1) {
		close(sockfd);
	}
}

/* Purpose: free httpd server's resource and stop server
 *    1. destroy the socket list
 *    2. destroy the thread pool
 *    3. destroy http error table
 *    4. destroy http mime table
 *    5. destroy configure file object
 *    6. close server socket
 * In    : *h: httpd server object
 * Return: void
 */
static void httpd_finish(httpd_t *h)
{
	/* Stop the httpd server         */
	h->flag &= HT_STOP;

	/* Close server sockets          */
	if (h->server_sock != -1) {
		close(h->server_sock);
		h->server_sock = -1;
	}

	/* Destroy socket table          */
	if (h->socktab)
		h->socktab->delete(h->socktab);

	/* Kill all threads              */
	if (h->th_pool)
		h->th_pool->delete(h->th_pool);

	/* Destroy cgi table object      */
	if (h->cgi)
		h->cgi->delete(h->cgi);

	/* Destroy the thread data pool  */
	if (h->thdata)
		h->thdata->delete(h->thdata);	
	
	/* Destroy error table           */
	if (h->errtab)
		h->errtab->delete(h->errtab);
	
	/* Destroy mime type             */
	if (h->mime)
		h->mime->delete(h->mime);

	/* Destroy configure object      */
	if (h->conf)
		h->conf->delete(h->conf);
}

/* Purpose: the thread schedule
 * In     : h: httpd server object
 * Return : void
 */
static void httpd_schedule(httpd_t *h)
{
	ht_socktab_t *stab = h->socktab;   /* Socket list table */
	thdatatab_t *thdata = h->thdata;   /* Thread data table */
	th_pool_t *pool = h->th_pool;      /* Thread pool table */
	thread_data_t *data = NULL;
	int th = 0;
	int sockfd = -1;
	
	while (1) {
		net_usa_t *rmt_ip;
		/* Get a socket fd     */
		if ((sockfd = stab->get(stab)) == -1) {
			break;
		}		
		//printf("get sockfd:%d\n", sockfd);
		/* Get a thread data   */
		if ((data = thdata->alloc(thdata)) == NULL) {
			stab->add(stab, sockfd);
			break; /* Not free thread data */
		}
                
		/* Set thread resource */
		data->httpd = h;
		data->sock = sockfd;
		rmt_ip = &data->rmt_ip;
		rmt_ip->len = sizeof(rmt_ip->sa);
		getpeername(sockfd, &rmt_ip->sa, &rmt_ip->len);
		th = pool->alloc(pool, httpd_thread_handler, (void *)data,
				 h->socket_max_time, NULL, NULL);
		if (!th) {
			//printf("sockfd readd:%d\n", sockfd);
			thdata->tfree(thdata, data);
			stab->add(stab, sockfd);
			break; /* Not free thread */
		}
	}
}

/* Purpose: Set the option of httpd
 * In     : h: httpd server object
 *          *n: the option name
 *          *v: the option value
 * Return : FAILURE: (0)
 *          SUCCESS: (1)
 */
static int httpd_set_opt(httpd_t *h, const char *n, const char *v)
{
	if ((n == NULL) || (v == NULL))
                return 0;
	
        return 1;
}

/* Purpose: httpd thread handler
 *         1. Initialize thread working environment
 *         2. Starting the connect
 *         3. handle the socket stream
 * In     : d: the parameters of thead handler
 * Return : the return value of the httpd thread handler
 */
static void *httpd_thread_handler(void *d)
{
	thread_data_t *data = (thread_data_t *)d;
	thdatatab_t *thdata = data->httpd->thdata;
	sigset_t mask;

	/* Block thread signals */
	sigfillset(&mask);
	sigdelset(&mask, SIGCHLD);
	pthread_sigmask(SIG_BLOCK, &mask, NULL);

	/* Handle Keep-Alive   */
	if (httpd_get_line(data))
		httpd_parse_request(data);

	/* Free thread record, set STOP status */
	thdata->tfree(thdata, data);

	return NULL;
}

/* Purpose: handle the data stream
 * In     : *data: thread data pointer
 * Return : 0: error, close socket and quit the current thread
 *          1: ok
 */
static int httpd_parse_request(thread_data_t *data)
{
	ht_errtable_t *errtab;

	/* Fail to received data */
	if (data->recvlen < 0) {
		goto REQ_ERR;
        }
	
	/* parse httpd header    */
	errtab = data->httpd->errtab;
	if (data->recvlen < HTTP_MIN_REQ_LEN) {
		errtab->send_error(errtab, HTTP_BAD_REQ, data);
		goto REQ_ERR;
	}
	
	/* parse request line    */
	if (!httpd_parse_request_line(data)) {
		goto REQ_ERR;
	}
	
	/* parse request header  */
	if (!httpd_parse_request_header(data)) {
		goto REQ_ERR;
	}
	
	/* handle the request   */
	if (!httpd_handle_request(data)) {
		goto REQ_ERR;
	}
        return 1;

REQ_ERR:
        return 0; /* Close the socket */
}

/* Purpose: remove double dots, for example: "//", "/." etc.
 * In     : *s: the string to deal with
 * Return : void
 */
static void httpd_rm_double_dots(char *s)
{
        char *p;

        if (s == NULL) {
		return;
        }

	p = s;
        while (*s != '\0') {
                *p++ = *s++;
                if (s[-1] == '/')
		  while (/* *s == '.' || */ *s == '/')
                                s++;
        }
        *p = '\0';
}

/* Purpose: parse the request line
 * In     : *data: thread data pointer
 * Return : 0: error
 *          1: ok
 */
static int httpd_parse_request_line(thread_data_t *data)
{
	char *reqline;
	char *tmp;
	char *method;
	ht_errtable_t *errtab;
	ht_mode_t *mode;
	int len;

	reqline = data->buf;
	errtab = data->httpd->errtab;

	/* Get the method        */
	method = reqline;
	if ((reqline = strchr(reqline, ' ')) == NULL) {
		return 0;
	} else
		*reqline++ = '\0';
	mode = http_methods;
	while (mode->ptr != NULL) {
		if (strcmp(method, mode->ptr) == 0) {
			data->method = mode;
			break;
		}
		mode++;
	}
	if (mode->ptr == NULL) {
		errtab->send_error(errtab, HTTP_NOT_IMPLEMENTED, data);
		return 0;
	}
	
	/* Get the uri           */
	data->uri = reqline;
        if ((reqline = strchr(reqline, ' ')) == NULL) {
		return 0;
        } else
		*reqline++ = '\0';
        if (data->uri[0] != '/') {
                errtab->send_error(errtab, HTTP_BAD_REQ, data);
                return 0;
        }
	
	/* Get the major and minor version */
	reqline += 5; /* skip "HTTP/"   */
	tmp = reqline; /* Major version */
        if ((reqline = strchr(reqline, '.')) == NULL) {
		return 0;
        } else
		*reqline++ = '\0';
	data->major_ver = atoi(tmp);
	
	tmp = reqline; /* Minor version */
        if ((reqline = strchr(reqline, '\r')) == NULL) {
		return 0;
        } else
		reqline[0] = reqline[1] = '\0'; /* Kill "\r\n" */
	data->minor_ver = atoi(tmp);

	if ((data->major_ver > 1) ||
	    ((data->major_ver == 1) && (data->minor_ver > 1))) {
		errtab->send_error(errtab, HTTP_VER_NOT_SUP, data);
		return 0;
	}

	tmp = (char *)&reqline[2]; /* Skip "\r\n" */

	/* Set current position of parsing */
	data->curpos = (char *)tmp - (char *)data->buf;

	/* Get the query           */
	if ((tmp = strchr(data->uri, '?')) != NULL) {
		*tmp++ = '\0';
		data->query = tmp;
		
		len = strlen(data->query);
		str_decode_url(data->query, len, (char *)data->query, len+1);
	}

	/* Deal with uri and query */
	len = strlen(data->uri);
	str_decode_url(data->uri, len, (char *)data->uri, len+1);
	httpd_rm_double_dots((char *)data->uri);

	return 1;
}



/* Purpose: parse the request header
 * In     : *data: thread data pointer
 * Return : 0: error
 *          1: ok
 */
static int httpd_parse_request_header(thread_data_t *data)
{
	ht_errtable_t *errtab = NULL;
	char *head, *end;          /* For all headers     */

	if (data->curpos >= data->recvlen) {
                return 1;
	}

	/* If need to receive more header data    */
	head = (char *)&data->buf[data->curpos];
	if (strstr(head, "\r\n\r\n") == NULL) {
		int len;
		head = (char *)&data->buf[data->recvlen];
		len = THREAD_RCVBUF_LEN - data->recvlen - 1;
		len = socket_read(data->sock, head,
				  len, "\r\n\r\n", 1);
		if (len == -1) {
			goto HTTPD_ERR;
		} else {
			data->recvlen += len;
			data->buf[data->recvlen] = '\0';
		}
	}

	errtab = data->httpd->errtab;
	/* Get the every header and add into header list  */
	end = (char *)&data->buf[data->curpos];
	do {
		/* Finding a header */
		for (head = end; (*end != '\r') && (*end != '\0'); end++);
		if (*end == '\0') {
			data->rhead->destroy(data->rhead);
			goto HTTPD_ERR;
		}
		end[0] = end[1] = '\0';

		if (!data->rhead->create(data->rhead, head)) {
			data->rhead->destroy(data->rhead);
			goto HTTPD_ERR;
		}

		end += 2;
	} while ((*end != '\r') && (*end != '\0'));

	/* Set the next parse position  */
	data->curpos = (char *)&end[2] - (char *)data->buf;
        return 1;

HTTPD_ERR:
	errtab->send_error(errtab, HTTP_INTER_SERVER_ERR, data);
        return 0;
}

/* Purpose: handle the request, decide to what to do
 * In     : *th: thread record pointer
 * Return : 0: error
 *          1: ok
 */
static int httpd_handle_request(thread_data_t *data)
{
	data->keepalive = THREAD_CON_CLOSE;

	/* Call method handler */
	if (data->method->func((void *)data) == NULL) {
		ht_errtable_t *errtab;
		errtab = data->httpd->errtab;

		errtab->send_error(errtab, HTTP_NOT_IMPLEMENTED, data);
		return 0;
	}
	
        return 1;
}

/* Purpose: Get the line
 * In     : *th: the thread object pointer
 * Return : the number of character to receive
 *          0: error
 */
static int httpd_get_line(thread_data_t *data)
{
	int len;

	/*  Waiting data   */
	len = socket_read(data->sock, data->buf,
			  THREAD_RCVBUF_LEN - 1, "\r\n", 3);
	if (len < 0) {
		ht_errtable_t *errtab;
		errtab = data->httpd->errtab;

		errtab->send_error(errtab, HTTP_INTER_SERVER_ERR, data);
		return 0;
	} else {
		data->recvlen = len;		
		return len;
	}
}

/* Purpose: read httpd configure file(/etc/vshttpd.conf)
 * In     : *h: httpd server object
 * Return : 0: error
 *          1: ok
 */
static int httpd_read_conf(httpd_t *h)
{
	sys_conf_t *conf;
        char *s, *p;
	struct sockaddr_in *servip;

	conf = h->conf;
	/* Get version string */
        if ((s = conf->find(conf, HT_VERSION_OPT)) != NULL) {
                p = strdup(s);
        } else {
                p = strdup(HT_DEF_VERSION);
        }
        if (p == NULL) {
                goto CONF_ERR;
        } else {
                h->ver = p;
        }

	/* Get work home      */
        if ((s = conf->find(conf, HT_WKHOME_OPT)) != NULL) {
                p = strdup(s);
        } else {
                p = strdup(HT_DEF_HOME);
        }
        if (p == NULL) {
                goto CONF_ERR;
        } else {
                h->wkhome = p;
        }
	
	/* IP and PORT        */
	if ((s = conf->find(conf, HT_SERVER_PORT_OPT)) != NULL)
		h->server_port = atoi(s);
	else
		h->server_port = atoi(HT_DEF_PORT);
	printf("h->server_port is %d\n", h->server_port);
	servip = &h->server_ip.sin;
	servip->sin_family = AF_INET;
	servip->sin_addr.s_addr = htonl(INADDR_ANY);
	servip->sin_port = htons(h->server_port);
	h->server_ip.len = sizeof(*servip);

	/* CGI file suffix name           */
	if ((s = conf->find(conf, HT_CGI_SUFFIX_OPT)) != NULL)
                p = strdup(s);
	else
                p = strdup(HT_DEF_CGISUFFIX);
        if (p == NULL) {
                goto CONF_ERR;
        } else {
                h->cgisuffix = p;
        }

	/* The max session waiting time   */
        if ((s = conf->find(conf, HT_SESS_MAX_TIME_OPT)) != NULL) {
		h->sess_max_time = atoi(s);
        } else {
		h->sess_max_time = HT_DEF_SESS_MAX_TIME;
        }

	/* root default                    */
	if ((s = conf->find(conf, HT_ROOT_DEF_OPT)) != NULL)
                p = strdup(s);
	else
                p = strdup(HT_DEF_ROOTDEF);
        if (p == NULL) {
                goto CONF_ERR;
        } else {
                h->rootdef = p;
        }

	/* The Min. thread number          */
	if ((s = conf->find(conf, HT_THREAD_MIN_OPT)) != NULL)
		h->thread_min = atoi(s);
	else
		h->thread_min = HT_DEF_THREAD_MIN;
	
	/* The max thread number           */
	if ((s = conf->find(conf, HT_THREAD_MAX_OPT)) != NULL)
		h->thread_max = atoi(s);
	else
		h->thread_max = HT_DEF_THREAD_MAX;

	/* The max wait time for idle thread*/
	if ((s = conf->find(conf, HT_THREAD_IDLETIME_OPT)) != NULL)
		h->thread_idle_time = atoi(s);
	else
		h->thread_idle_time = HT_DEF_THREAD_IDLETIME;

	/* The max socket client link     */
	if ((s = conf->find(conf, HT_SOCKET_MAX_OPT)) != NULL)
		h->socket_max = atoi(s);
	else
		h->socket_max = HT_DEF_SOCKET_MAX;

	/* The max socket waiting time    */
	if ((s = conf->find(conf, HT_SOCKET_MAX_TIME_OPT)) != NULL)
		h->socket_max_time = atoi(s);
	else
		h->socket_max_time = HT_DEF_SOCKET_MAX_TIME;

	return 1;

CONF_ERR:
        if (h->ver) {
                free(h->ver);
                h->ver = NULL;
}
        if (h->wkhome) {
                free(h->wkhome);
                h->wkhome = NULL;
        }
#if defined(HT_CGI)
        if (h->cgisuffix) {
                free(h->cgisuffix);
                h->cgisuffix = NULL;
        }
#endif
        if (h->rootdef) {
                free(h->rootdef);
                h->rootdef = NULL;
        }
        return 0;
}

/* Purpose: open a socket on the given port
 * In     : port: 
 * Return : 0: error
 *          1: ok
 */
static int httpd_open_socket(httpd_t *h)
{
	int sock = -1;
	int flag;
	net_usa_t *sa = &h->server_ip;
	
	/* Open a socket          */
        if ((sock = socket(PF_INET, SOCK_STREAM, 6)) == -1) {
		goto HTFAIL;
        }
	/* Set non blocking mode  */
        if (!fd_noblock(sock)) {
		goto HTFAIL;
        }
        if (!fd_cloexec(sock)) {
		goto HTFAIL;
        }
	/* Set socket to re-use address */
	flag = 1;
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, 
		       (void *)&flag, sizeof(flag)) != 0) {
		goto HTFAIL;
	}

	/* Bind ip address to socket    */
	if (bind(sock, &sa->sa, sa->len) < 0) {
		goto HTFAIL;
	}
	
	/* Set the listen max number   */
	if (listen(sock, HT_DEF_LISTEN) != 0) {
		goto HTFAIL;
	}
	
	/* Set starting time   */
	time(&h->time);

	h->server_sock = sock;
	return 1;

HTFAIL:
	if (sock != -1) {
		h->server_sock = -1;
		close(sock);
	}
	return 0;
}

/* Purpose: Create a header object
 * In     : **header: the pointer to store header object
 * Return : NULL: error
 *          the valid header object pointer
 */
ht_header_t *ht_header_new(ht_header_t **header)
{
	ht_header_t *tmp;

	tmp = (ht_header_t *)calloc(1, sizeof(ht_header_t));
        if (tmp == NULL) {
		return NULL;
        } else
		*header = tmp;

/* Intialize ways */
	tmp->delete = ht_header_delete;
	tmp->create = ht_header_create;
	tmp->remove = ht_header_remove;
	tmp->setfunc = ht_header_setfunc;
	tmp->find = ht_header_find;
	tmp->destroy = ht_header_destroy;
	
	return tmp;
}

/* Purpose: Delete the header object
 * In     : *header: the header object pointer
 * Return : void
 */
static void ht_header_delete(ht_header_t *header)
{
	if (header) {
		if (header->head)
			ht_header_destroy(header);
		free(header);
	}
}

/* Purpose: Create header record according to *s
 * In     : *header: the header object
 *          *s: the header string
 * Return : 0: error
 *          1: ok
 */
static int ht_header_create(ht_header_t *header, char *s)
{
	char *name, *value;
	ht_hdrrec_t *rec;

        if ((header == NULL) || (s == NULL)) {
                return 0;
        }

	/* Get name         */
	for (name = s; ((*s != ':') && (*s != '\0')); s++);
	if (*s == ':') {
		*s = '\0';
	} else if (*s == '\0') {
                return 1; /* If isn't value, ignore */
	}
	
	/* Get value, eating the first blank */
	value = s+2;
	
	/* Allocate a record */
	if ((rec = calloc(1, sizeof(ht_hdrrec_t))) == NULL) {
                return 0;
	}

	rec->name = name;
	rec->val = value;
	rec->vallen = strlen(value);

	/* Add header into the header list */
	rec->next = header->head;
	header->head = rec;

        return 1;
}

/* Purpose: Remove header record according to *n(name)
 * In     : *header: the header object
 *          *n: the name string
 * Return : 0: error
 *          1: ok
 */
static int ht_header_remove(ht_header_t *header, char *n)
{
	ht_hdrrec_t *rec, *up;

        if ((header == NULL) || (n == NULL)) {
                return 0;
        }
	
	/* find name       */
	rec = up = header->head;
	while(rec != NULL) {
		if (strcmp(rec->name, n) == 0) { /* found */
			/* Free rec  */
			if (rec == header->head) {
				header->head = rec->next;
				free(rec);
			} else {
				up->next = rec->next;
				free(rec);
			}
                        return 1;
		}

		/* Move to the next record */
		up = rec;
		rec = rec->next;
	}

	if (rec == NULL)
		return 0;
	return 1;
}

/* Purpose: Set handle function for header record
 * In     : *header: the header object
 *          *n: the name string
 *          func: the handle function for header record
 * Return : 0: error
 *          1: ok
 */
static int ht_header_setfunc(ht_header_t *header, char *n, ht_handler_t func)
{
	ht_hdrrec_t *rec;

        if ((header == NULL) || (n == NULL) || (func == NULL)) {
                return 0;
        }

	/* find name       */
	rec = header->head;
	while (rec != NULL) {
		if (strcmp(rec->name, n) == 0) { /* found */
			rec->func = func;
			break;
		}
		rec = rec->next;
	}

	return 1;
}

/* Purpose: find a header into the current header list
 * In     : *header: the header object
 *        : *n: the name string
 * Return : NULL: error
 *          the value string for the name
 */
static char *ht_header_find(ht_header_t *header, char *n)
{
	ht_hdrrec_t *rec;

        if ((header == NULL) || (n == NULL)) {
		return NULL;
        }
	rec = header->head;
	while (rec != NULL) {
		if (strcmp(rec->name, n) == 0)
			return (char *)rec->val;

		rec = rec->next;
	}

	return NULL;
}

/* Purpose: Destroy all the header record in header object
 * In     : *header: the header object
 * Return : 0: error
 *          1: ok
 */
static int ht_header_destroy(ht_header_t *header)
{
	ht_hdrrec_t *rec, *next;

	rec = next = header->head;
	while (rec != NULL) {
		next = rec->next;
		free(rec);
		rec = next;
	}
	header->head = NULL;

	return 1;
}

/* Purpose: handle GET method
 *          1. download file
 *          2. List the directory info.
 *          3. execute a CGI program
 * In     : *arg: the argument
 * Return : NULL: error
 *          other: ok
 */
static void *ht_handle_get_method(void *arg)
{
	thread_data_t *data;
	const char *uri;
	int uritype;

	data = (thread_data_t *)arg;
	uri = data->uri;

	/* Set default page while URL only is IP address */
	if (strcmp(data->uri, "/") == 0) {
		data->uri = data->httpd->rootdef;
	}

	/* Get uri address type  */
	uritype = ht_get_uri_type(data);
	switch (uritype) {
	case HTTP_URL_TYPE_CGI:
		ht_cgi_do_get(data);
		break;
#if 0
	case HTTP_URL_TYPE_DIR:
		ht_read_dir(data);
		break;
#endif	
	case HTTP_URL_TYPE_FILE:
		ht_read_file(data, HTTP_REQ_NOT_HEAD);
		break;

	default:
		{
			ht_errtable_t *errtab;
			if(strcmp(uri, "/crossdomain.xml") == 0){
				data->sendlen = sprintf(data->sbuf, "HTTP/1.1 200 OK\r\nServer: vshttpd\r\nCache-Control: no-cache\r\nPragma: no-cache\r\nExpires: 0\r\nContent-length: 1253\r\nContent-type: text/xml;charset=UTF-8\r\nConnection: close\r\n\r\n<?xml version=\"1.0\"?><cross-domain-policy><allow-access-from domain=\"*\" /></cross-domain-policy>");
				(void)socket_write(data->sock, data->sbuf, data->sendlen, 1);
				break;
			}else{
				errtab = data->httpd->errtab;
				errtab->send_error(errtab, HTTP_NOT_IMPLEMENTED, data);
			}
			data->keepalive = THREAD_CON_CLOSE;
		}
		return NULL;
	}

	return (void *)1;
}

/* Purpose: handle POST method
 *          1. post a cgi form
 * In     : *arg: the argument
 * Return : NULL: error
 */
static void *ht_handle_post_method(void *arg)
{
	thread_data_t *data;
	const char *uri;
	int uritype;

	data = (thread_data_t *)arg;
	uri = data->uri;

	/* Get uri address type  */
	uritype = ht_get_uri_type(data);
	if (uritype == HTTP_URL_TYPE_CGI) {
		ht_cgi_do_post(data);
		return (void *)1;
	}
	
	return NULL;
}

/* Purpose: handle GET method
 *          1. get file head
 *          2. get directory head
 * In     : *arg: the argument
 * Return : NULL: error
 */
static void *ht_handle_head_method(void *arg)
{
	thread_data_t *data;
	const char *uri;
	int uritype;

	data = (thread_data_t *)arg;
	uri = data->uri;

	/* Get uri address type  */
	uritype = ht_get_uri_type(data);
	if (uritype == 2) {
		ht_read_file(data, HTTP_REQ_HEAD);
		return (void *)1;
	}

	return NULL;
}

/* Purpose: get the URL type to request
 * In     : *data: the thread data
 * Return : 0: error
 *          1: CGI
 *          2: file
 *          3: dir
 */
static int ht_get_uri_type(thread_data_t *data)
{
	struct stat st;
	const char *buf;
	int ret;

	if (strcmp(data->uri, "/") == 0)
		return HTTP_URL_TYPE_CGI;
	
	if (data->query != NULL) {
		return HTTP_URL_TYPE_CGI;
	}
	
	buf = data->uri + strlen(data->uri) - 3;
	if (strcmp(buf, data->httpd->cgisuffix) == 0) {
		return HTTP_URL_TYPE_CGI;
	}

	sprintf(data->sbuf, "%s%s", data->httpd->wkhome, data->uri);
	ret = stat(data->sbuf, &st);
	if (ret == -1) {
		return HTTP_URL_TYPE_ERROR;
	} if (S_ISDIR(st.st_mode)) {/* dir  */
		//return HTTP_URL_TYPE_DIR;
	}
	if (S_ISREG(st.st_mode)) {  /* file */
		return HTTP_URL_TYPE_FILE;
	}
	
	return 0; /* fail to other */
}

/*-----------------------------------------------------------------------------
  Thread data table operation
------------------------------------------------------------------------------*/
/* Purpose: Create the thread data object
 * In     : **tab: the pointer which store the thread table object
 *          max  : the Max. thread data table in socket table
 * Return : NULL : error
 *          the thread data table object
 */
thdatatab_t *thdatatab_new(thdatatab_t **tab, int max)
{
	thdatatab_t *tmp;
	int i;

        if ((tab == NULL) || (max <= 0)) {
		return NULL;
        }
	
	if ((tmp = calloc(1, sizeof(thdatatab_t))) == NULL) {
		return NULL;
        } else {
		*tab = tmp;
        }
	
	/* Initialize data         */
	INIT_LIST_HEAD(&tmp->list);
	pthread_mutex_init(&tmp->lock, NULL);
	
	/* Create thread data pool */
	for (i = 0; i < max; i++) {
		thread_data_t *th;

		if ((th = thdata_new()) == NULL) {
			goto TH_ERR;
		}
		list_add(&th->node, &tmp->list);
	}

	/* Initialize ways         */
	tmp->delete = thdatatab_delete;
	tmp->tfree  = thdatatab_free;
	tmp->alloc  = thdatatab_alloc;

	return tmp;

TH_ERR:
	thdatatab_delete(tmp);
	return NULL;
}

/* Purpose : Destroy the thread pool data
 * In      : *tab: the thread data table object
 * Return  : void
 */
static void thdatatab_delete(thdatatab_t *tab)
{
	thread_data_t *rec = NULL, *tmp;

	pthread_mutex_lock(&tab->lock);
	
	if (!list_empty(&tab->list)) {
		list_for_each_entry_safe(rec, tmp, &tab->list, node) {
			list_del(&rec->node);
			thdata_clear(rec);
			thdata_delete(rec);
		}
	}

	pthread_mutex_unlock(&tab->lock);
	free(tab);
}

/* Purpose    : Allocate a thread data
 * Parameters : *tab: the thread data table object
 * Return     : NULL: Error
 *              the thread data object
 */
static thread_data_t *thdatatab_alloc(thdatatab_t *tab)
{
	thread_data_t *rec = NULL;

	pthread_mutex_lock(&tab->lock);
	
	if (!list_empty(&tab->list)) {
		list_for_each_entry(rec, &tab->list, node) {
			if (!rec->used) {
				thdata_init(rec);
				rec->used = 1;
				break;
			}
		}
	}
	
	pthread_mutex_unlock(&tab->lock);
	if (rec && rec->used)
		return rec;
	else
		return NULL;
}

/* Purpose    : Free a thread data
 * Parameters : *tab: the thread data table object
 *              *data: the thread data
 * Return     : 1: Ok
 *              0: Error
 */
static int thdatatab_free(thdatatab_t *tab, thread_data_t *data)
{
	thread_data_t *rec = NULL;
	int found = 0;

        if ((tab == NULL) || (data == NULL)) {
                return 0;
        }
	pthread_mutex_lock(&tab->lock);
	list_for_each_entry(rec, &tab->list, node) {
		if (rec == data) {
			thdata_clear(data);
			rec->used = 0;
			found = 1;
			break;
		}
	}
	pthread_mutex_unlock(&tab->lock);
	if (!found) {
                return 0;
	}
        return 1;
}

/* Purpose: Allocate the data memory space for thread data
 *          1. Allocate receiving header object
 *          2. Allocate receiving message object
 * In     : *tmp: the thread data pool
 * Return : 0: error
 *          1: ok
 */
static thread_data_t *thdata_new(void)
{
	thread_data_t *tmp;
	
        if ((tmp = calloc(1, sizeof(thread_data_t))) == NULL) {
		return NULL;
        }

	INIT_LIST_HEAD(&tmp->node);
	tmp->sock = -1;
	tmp->used = 0;
	
	/* Request   */
	tmp->rhead = ht_header_new(&tmp->rhead);
	if (tmp->rhead == NULL) {
		goto NEW_ERR;
	}
	return tmp;

NEW_ERR:
	if (tmp)
		thdata_delete(tmp);
	return NULL;
}

/* Purpose: Initialize thread data
 * In     : *tmp: the thread data pointer
 * Return : void
 */
static void thdata_init(thread_data_t *tmp)
{
	/* Remote socket information */
	memset(&tmp->rmt_ip, 0, sizeof(net_usa_t));

	/* http request   */
	tmp->recvlen = 0;
	tmp->curpos = 0;
	time(&tmp->birth_time);

	/* Request        */
	tmp->method = NULL;
	tmp->uri = NULL;
	tmp->query = NULL;
	tmp->major_ver = -1;
	tmp->minor_ver = -1;

	/* Respone        */
	tmp->status = -1;
	tmp->sendlen = 0;
	if (tmp->rhead != NULL)
		tmp->rhead->destroy(tmp->rhead);
}

/* Purpose: Free memory space allocated by thread data
 *          1. receiving header list
 *          2. receiving message body
 * In     : *t: the thread data pointer
 * Return : void
 */
static void thdata_clear(thread_data_t *t)
{
	/* Remote socke information */
	if (t->sock != -1) {
		close(t->sock);
		t->sock = -1;
	}
	
	/* For http request   */
	t->birth_time = 0;

	/* Request            */
	t->keepalive = 0;
	if (t->rhead != NULL)
		t->rhead->destroy(t->rhead);
}

/* Purpose: Free the object of thread data
 *          1. Free receiving header object
 *          2. Free receiving message object
 * In     : *tmp: the thread data pointer
 * Return : void
 */
static void thdata_delete(thread_data_t *tmp)
{
        if (tmp == NULL) {
		return;
        }

        if ((tmp->used) && (tmp->sock != -1)) {
		close(tmp->sock);
		tmp->sock = -1;
        }

	if (tmp->rhead != NULL) {
		tmp->rhead->destroy(tmp->rhead);
		tmp->rhead->delete(tmp->rhead);
	}
	
	free(tmp);
}

/*-----------------------------------------------------------------------------
  Socket table operation
------------------------------------------------------------------------------*/
/* Purpose: Create the socket table object
 * In     : **tab: the pointer which store the socket table object
 *          max  : the Max. socket number in socket table
 * Return : NULL: error
 *          the socket table object
 */
ht_socktab_t *ht_socktab_new(ht_socktab_t **tab, int max)
{
	ht_socktab_t *tmp;

        if ((tab == NULL) || (max <= 0)) {
		return NULL;
        }
	
	if ((tmp = calloc(1, sizeof(ht_socktab_t))) == NULL) {
		return NULL;
	} else
		*tab = tmp;

	tmp->maxnum = max;
	INIT_LIST_HEAD(&tmp->head);
	pthread_mutex_init(&tmp->lock, NULL);

	tmp->delete   = ht_socktab_delete;
	tmp->add      = ht_socktab_add;
	tmp->get      = ht_socktab_get;
	tmp->is_empty = ht_socktab_is_empty;

	return tmp;
}

/* Purpose: Destroy the socket table object
 * In     : *tab: the socket table object
 * Return : void
 */
static void ht_socktab_delete(ht_socktab_t *tab)
{
	ht_sock_t *rec, *tmp;

	/* Delete socket list  */
	pthread_mutex_lock(&tab->lock);
	if (!list_empty(&tab->head)) {
		list_for_each_entry_safe(rec, tmp, &tab->head, node) {
			list_del(&rec->node);
			if (rec->sock != -1)
				close(rec->sock);
			free(rec);
		}
	}
	pthread_mutex_unlock(&tab->lock);

	free(tab);
}

/* Purpose: Add the fd into the socket list
 * In     : *tab: the socket table object
 *          fd: the socket file descriptor to add
 * Return : 0: Error
 *          1: Ok
 */
static int ht_socktab_add(ht_socktab_t *tab, int fd)
{
	ht_sock_t *rec;
        int ret = 0;

	pthread_mutex_lock(&tab->lock);
	if (tab->num >= tab->maxnum) {
		goto HT_ERR;
	}

	if (!list_empty(&tab->head)) {
		rec = list_entry((&tab->head)->next, ht_sock_t, node);
		if(rec->sock == fd){
			ret = 1;
			goto HT_ERR;
		}
	}

	if ((rec = calloc(1, sizeof(ht_sock_t))) == NULL) {
		goto HT_ERR;
	} else {
		rec->sock = fd;
		INIT_LIST_HEAD(&rec->node);
	}

	/* Add the record into the end of list  */
	list_add_tail(&rec->node, &tab->head);
	tab->num++;
        ret = 1;
	
HT_ERR:
	pthread_mutex_unlock(&tab->lock);
	return ret;
}

/* Purpose: Get the fd from the socket list
 * In     : *tab: the socket table object
 * Return : -1: socket list is empty
 *          >0: the valid socket fd
 */
static int ht_socktab_get(ht_socktab_t *tab)
{
	ht_sock_t *rec;
	int retfd = -1;
	
	/* Get the first socket record from socket list  */
	pthread_mutex_lock(&tab->lock);
	if (!list_empty(&tab->head)) {
		rec = list_entry((&tab->head)->next, ht_sock_t, node);
		list_del(&rec->node);
		retfd = rec->sock;
		tab->num--;
		free(rec);
	}
	pthread_mutex_unlock(&tab->lock);

	return retfd;
}

/* Purpose: If the socket list is empty
 * In     : *tab: the socket table object
 * Return : 0: Isn't empty
 *          1: Empty
 */
static int ht_socktab_is_empty(ht_socktab_t *tab)
{
	return list_empty(&tab->head);
}

