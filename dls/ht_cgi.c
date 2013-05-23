/******************************************************************************
 * 
 * File: ht_cgi.c
 *
 * Date: 2007-06-08
 *
 * Author: wang songnian
 *
 * Descriptor:
 *   
 * Note:
 *  the CGI handle program
 *
 * Version: 0.1
 *
 * Modified:
 *
 ******************************************************************************/
/*-----------------------------------------------------------------------------
  System header files 
------------------------------------------------------------------------------*/
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/*-----------------------------------------------------------------------------
  Local header files
------------------------------------------------------------------------------*/
#include "httpd.h"
#include "ht_def.h"

/*-----------------------------------------------------------------------------
  Data structures
------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
  Global variables and constants
------------------------------------------------------------------------------*/
#define HT_CGI_DO_GET        (0)
#define HT_CGI_DO_POST       (1)

/*-----------------------------------------------------------------------------
  Functions Declare
------------------------------------------------------------------------------*/
static int ht_cgi_do(thread_data_t *data, int way);
static void ht_cgi_fill_env(thread_data_t *data, cgi_t *cgi, int way);
static int ht_cgi_get_browser(thread_data_t *data, char *tmpbuf);
static int ht_cgi_rcv_msg_body(thread_data_t *data);

/*-----------------------------------------------------------------------------
  Functions
------------------------------------------------------------------------------*/
/* Purpose: execute CGI program for GET way
 * In     : *data: thread data pointer
 * Return : 1: ok
 *          0: error
 */
int ht_cgi_do_get(thread_data_t *data)
{
	return ht_cgi_do(data, HT_CGI_DO_GET);
}

/* Purpose: execute CGI program for POST way
 * In     : *data: thread data pointer
 * Return : 1: ok
 *          0: error
 */
int ht_cgi_do_post(thread_data_t *data)
{
	return ht_cgi_do(data, HT_CGI_DO_POST);
}

/* Purpose: Execute CGI program
 * In     : *data: thread data pointer
 *          way: HT_CGI_DO_GET: get way
 *               HT_CGI_DO_POST: post way
 * Return : 1: ok
 *          0: error
 */
static int ht_cgi_do(thread_data_t *data, int way)
{
	ht_errtable_t *errtab;
	cgi_tab_t *cgitab;
	const char *s;
	cgi_t *cgi = NULL;
	int len;

        if (data == NULL) {
                return 0;
        }
	errtab = data->httpd->errtab;
	cgitab = data->httpd->cgi;

        if ((errtab == NULL) || (cgitab == NULL)) {
                return 0;
        }

	/* Set default page while URL only is IP address */
	if (strcmp(data->uri, "/") == 0) {
		data->uri = data->httpd->rootdef;
	}

	/* Allocate the cgi object */
	if ((cgi = cgitab->alloc(cgitab)) == NULL) {
		errtab->send_error(errtab, HTTP_INTER_SERVER_ERR, data);
		goto CGI_ERR;
	}
	/* Set session table object*/
	cgi->stab = cgitab->stab;

	/* Copy the session string */
	s = data->rhead->find(data->rhead, HTTP_COOKIE_XHDR);
	if (s != NULL)
		strcpy(cgi->sessbuf, s);
	else
		cgi->sessbuf[0]='\0';

	if (way == HT_CGI_DO_GET) { /* GET way */
		/* Copy query string  */
		if (data->query == NULL)
			cgi->recbuf[0]='\0';
		else {
			len = strlen(data->query);
			strncpy(cgi->recbuf, data->query, len);

			/* Fill a '&' for cgi parse */
			cgi->recbuf[len++] = '&';
			cgi->recbuf[len] = '\0';
			cgi->reclen = len;
		}
	} else {/* POST way           */
		/* Copy query string  */
		if (data->query != NULL) {
			len = strlen(data->query);
			strncpy(cgi->recbuf, data->query, len);
			
			/* Fill a '&' for cgi parse */
			cgi->recbuf[len++] = '&';
			cgi->recbuf[len] = '\0';
			cgi->reclen = len;
		}

		if (!ht_cgi_rcv_msg_body(data)) {
			goto CGI_ERR;
		}
		
		/* Copy message body  */
		len = data->recvlen - data->curpos;
		if (len > 0) {
			memcpy(&cgi->recbuf[cgi->reclen],
			       &data->buf[data->curpos], len);
			cgi->reclen += len;
			cgi->recbuf[cgi->reclen] = '\0';
		}
	}

	/* Fill the environment    */
	ht_cgi_fill_env(data, cgi, way);

	/* Set thread data         */
	cgi->sockfd = data->sock;
	data->sock = -1;
		
	/* Run cgi execution file(code) */
	cgitab->handler((void *)cgi);

CGI_ERR:
	/* Free the cgi object     */
	if (cgi != NULL) {
		cgitab->cfree(cgitab, cgi);
	}
	
	data->keepalive = THREAD_CON_CLOSE;
	
	return 1;
}

/* Purpose: receive message body
 * In     : *data: thread data pointer
 * Return : 0: error
 *          1: ok
 */
static int ht_cgi_rcv_msg_body(thread_data_t *data)
{
	int len;
	char *s;

	/* If need to receive more header data    */
	s = data->rhead->find(data->rhead, HTTP_CONT_LEN_EHDR);
	if (s == NULL) {
		return 1;
	} else {
		len = atoi(s);
	}

	s = data->rhead->find(data->rhead, HTTP_CONT_TYPE_EHDR);
	if (s == NULL) {
		return 1;
	}
	if (strncmp(s, HTTP_CONT_MULTIPART_TYPE,
		    strlen(HTTP_CONT_MULTIPART_TYPE)) != 0) {
		/* Get the length to read from file       */
		len = len - (data->recvlen - data->curpos);
		if (len > (THREAD_RCVBUF_LEN - data->recvlen + 1)) {
		} else if (len > 0) {
			s = (char *)&data->buf[data->recvlen];
			len = socket_read(data->sock, s, len, NULL, 1);
			if (len < 0) {
				return 0;
			} else  {
				data->recvlen += len;
			}

		}
	}
	//printf("socket read len %d   %s\n", data->recvlen, s);
	return 1;
}

/* Purpose: Prepare CGI environment
 * In     : *data: thread data object pointer
 *          *cgi: the cgi object pointer
 *          way: HT_CGI_DO_GET: get way
 *               HT_CGI_DO_POST: post way
 * Return: void
 */
static void ht_cgi_fill_env(thread_data_t *data, cgi_t *cgi, int way)
{
	char *buf, *s;
	int len;
	httpd_t *h;
	char tmpbuf[LINE_MAX];
	net_usa_t saddr;

	buf = cgi->envbuf;
	len = 0;
	/* CGI_SCRIPT_NAME  */
	len += sprintf(buf+len, "%s=%s\t", CGI_SCRIPT_NAME, data->uri+1);

	/* GATEWAY_INTERFACE */
	len += sprintf(buf+len, "%s=%s\t", CGI_GATEWAY_INTERFACE,
		       CGI_GATWAY_INTERFACE_VAL);
        
	/* SERVER_PROTOCOL   */
	len += sprintf(buf+len, "%s=%s\t", CGI_SERVER_PROTOCOL, 
		       CGI_SERVER_PROTOCOL_VAL);
        
	/* REQUEST_METHOD    */
	if (way == HT_CGI_DO_GET) {
		len += sprintf(buf+len, "%s=%s\t", CGI_REQUEST_METHOD,
			       "GET");
	} else {
		len += sprintf(buf+len, "%s=%s\t", CGI_REQUEST_METHOD,
			       "POST");
	}
        
	/* SERVER_NAME      */
	h = data->httpd;
	memset(&saddr, 0, sizeof(saddr));
	saddr.len = sizeof(struct sockaddr_in);
	getsockname(data->sock, &saddr.sa, &saddr.len);
	inet_ntop(AF_INET, (void *)&saddr.sin.sin_addr,
		  tmpbuf, sizeof(struct sockaddr_in));
	len += sprintf(buf+len, "%s=%s\t", CGI_SERVER_NAME, tmpbuf);

	/* SERVER_PORT      */
	len += sprintf(buf+len, "%s=%hu\t", CGI_SERVER_PORT, 
		       h->server_port);

	/* REMOTE_ADDR      */
	inet_ntop(AF_INET, (void *)&data->rmt_ip.sin.sin_addr,
		  tmpbuf, sizeof(struct sockaddr_in));
	len += sprintf(buf+len, "%s=%s\t", CGI_REMOTE_ADDR, tmpbuf);

	/* REMOTE_IDENT     */
	tmpbuf[0] = '\0';
	if (ht_cgi_get_browser(data, tmpbuf))
		len += sprintf(buf+len, "%s=%s\t", CGI_REMOTE_IDENT, tmpbuf);

	/* CONTENT_TYPE     */
	s = data->rhead->find(data->rhead, HTTP_CONT_TYPE_EHDR);
	if (s) {
		len += sprintf(buf+len, "%s=%s\t", CGI_CONTENT_TYPE, s);
	}
	
	/* CONTENT_LENGTH   */
	s = data->rhead->find(data->rhead, HTTP_CONT_LEN_EHDR);
	if (s) {
		len += sprintf(buf+len, "%s=%s\t", CGI_CONTENT_LENGTH, s);
	}

	buf[len] = '\0';

	cgi->envlen = len;
}

/* Purpose: Get the browser information
 * In     : *data  : thread data object pointer
 *          *tmpbuf: the tmp buffer
 * Return : SUCCESS  (1)
 *          FAILURE  (0)
 */
static int ht_cgi_get_browser(thread_data_t *data, char *tmpbuf)
{
	char *s;
	int pos = 0;
	
	/* If need to receive more header data    */
	s = data->rhead->find(data->rhead, HTTP_USER_AGENT_RHDR);
	if (s == NULL) {
                return 0;
	}

	if (strstr(s, "Windows") != NULL) {
		/* Detect the operation system    */
		pos += sprintf(tmpbuf+pos, "OS:[Windows]-");
		
		/* Detect the browser             */
		pos += sprintf(tmpbuf+pos, "Browser:[");
		if (strstr(s, "Firefox") != NULL)
			pos += sprintf(tmpbuf+pos, "Firefox");
		else if (strstr(s, "TencentTraveler") != NULL)
			pos += sprintf(tmpbuf+pos, "TencentTraveler");
		else if (strstr(s, "MAXTHON") != NULL)
			pos += sprintf(tmpbuf+pos, "MAXTHON");
		else if (strstr(s, "MSIE 6.0") != NULL)
			pos += sprintf(tmpbuf+pos, "(IE6.0)");
		else if (strstr(s, "MSIE 7.0") != NULL)
			pos += sprintf(tmpbuf+pos, "(IE7.0)");
		pos += sprintf(tmpbuf+pos, "]");

		return SUCCESS;
	}
	
	if (strstr(s, "Linux") != NULL) {
		/* Detect the operation system    */
		pos += sprintf(tmpbuf+pos, "OS:[Linux");
		if (strstr(s, "Fedora") != NULL)
			pos += sprintf(tmpbuf+pos, "(Fedora)");
		else if (strstr(s, "Ubuntu") != NULL)
			pos += sprintf(tmpbuf+pos, "(Ubuntu)");
		else if (strstr(s, "Red Hat") != NULL)
			pos += sprintf(tmpbuf+pos, "(Red Hat)");
		pos += sprintf(tmpbuf+pos, "]-");

		/* Detect the browser             */
		pos += sprintf(tmpbuf+pos, "Browser[");
		if (strstr(s, "Firefox") != NULL)
			pos += sprintf(tmpbuf+pos, "Firefox");
		else if (strstr(s, "ELinks") != NULL)
			pos += sprintf(tmpbuf+pos, "ELinks");
		else if (strstr(s, "Mozilla") != NULL)
			pos += sprintf(tmpbuf+pos, "Mozilla");
		pos += sprintf(tmpbuf+pos, "]");

		return SUCCESS;
	}
	

	pos += sprintf(tmpbuf+pos, "OS:[Unknown]-Browser:[Unknown]");
	return SUCCESS;
}

