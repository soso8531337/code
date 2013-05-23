/******************************************************************************
 * Copyright (C) 2007 by IOVST
 * 
 * File: ht_err.c
 *
 * Date: 2007-06-08
 *
 * Author: Liu Yong, <liuyong@iovst.com>
 *
 * Descriptor:
 *   
 * Note:
 *  The http error program
 *
 * Version: 0.1
 *
 * Modified:
 *
 ******************************************************************************/

/*-----------------------------------------------------------------------------
  System header files 
------------------------------------------------------------------------------*/
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

/*-----------------------------------------------------------------------------
  Local header files
------------------------------------------------------------------------------*/
#include "ht_def.h"
#include "httpd.h"

/*-----------------------------------------------------------------------------
  Global variables and constants
------------------------------------------------------------------------------*/
/* Error table
 * See RFC2612, Chapter 10(Status Code Definitions) 
 */
#define HT_ERRTAB_NUM          (sizeof(ht_err_table)/sizeof(ht_err_table[0]))
static ht_err_t ht_err_table[] = {
	/* Informational 1xx */
/*
	{HTTP_CONTINUE, "Continue", NULL},
	{HTTP_SWITCH_PROTOCOLS, "Switching Protocols", NULL},
*/
	/* Successful 2xx    */
	{HTTP_OK, "OK", NULL},
/*
	{HTTP_CREATED, "Created", NULL},
	{HTTP_ACCEPTED, "Accepted", NULL},
	{HTTP_NON_AUTO, "Non-Authoritative Information", NULL},
	{HTTP_NO_CONTENT, "No Content", NULL},
	{HTTP_RESET_CONTENT, "Reset Content", NULL},
*/
	{HTTP_PARTIAL_CONTENT, "Partial Content", NULL},

	/* Redirection 3xx   */
/*
	{HTTP_MULTI_CHOICES, "Multiple Choices", NULL},
	{HTTP_MOVE_PERM, "Moved Permanently", NULL},
*/
	{HTTP_FOUND, "Found", "Drectories must end with a slash"},
/*
	{HTTP_SEE_OTHER, "See Other", NULL},
	{HTTP_NOT_MODIFIED, "Not Modified", NULL},
	{HTTP_USE_PROXY, "Use Proxy", NULL},
	{HTTP_TEMP_REDIRECT, "Temporary Redirect", NULL},
*/
	
	/* Client Error 4xx  */
        {HTTP_BAD_REQ, "Bad request", "Unsupported method"},
	{HTTP_UNAUTH, "Unauthorized", ""},
/*
	{HTTP_PAYMENT_REQ, "Payment Required", NULL},
	{HTTP_FORBIDDEN, "Forbidden", NULL},
	{HTTP_NOT_FOUND, "Not Found", NULL},
	{HTTP_METHOD_NOT_ALLOW, "Method Not Allowed", NULL},
	{HTTP_NOT_ACCEPT, "Not Acceptable", NULL},
	{HTTP_PROXY_AUTH, "Proxy Authentication Required", NULL}
*/
	{HTTP_REQ_TIMEOUT, "Request", 
	 "Not request appeared within a reasonable time period"},
/*
	{HTTP_CONFLICT, "Conflict", NULL},
	{HTTP_GONE, "Gone", NULL},
	{HTTP_LENGTH_REQ, "Length Required", NULL},
	{HTTP_PRECON_FAILED, "Precondition Failed", NULL},
	{HTTP_REQ_ENTITY_LARGE, "Request Entity Too Large", NULL},
	{HTTP_REQ_URI_LONG, "Request-URI Too Large", NULL},
	{HTTP_UNSUP_MEDIA_TYPE, "Unsupported Media Type", NULL},
	{HTTP_REQ_RANGE_NOT_SAT, "Requested range not satisfiable", NULL},
	{HTTP_EXPECT_FAILED, "Expectation Failed", NULL},
*/
	/* Server Error 5xx */
	{HTTP_INTER_SERVER_ERR, "Internal Server Error",
	 "Internal Server Error"},
	{HTTP_NOT_IMPLEMENTED, "Not Implemented",
	 "The requested method is not recognized by this server"},
/*
	{HTTP_BAD_GATEWAY, "Bad Gateway", NULL},
	{HTTP_SERV_UNAVAIL, "Service Unavailable", NULL},
	{HTTP_GATEWAY_TIMEOUT, "Gateway Time-out", NULL},
*/
	{HTTP_VER_NOT_SUP, "HTTP Version not supported", NULL}
};

/*-----------------------------------------------------------------------------
  Functions
------------------------------------------------------------------------------*/
static void ht_errtable_delete(ht_errtable_t *table);
static void ht_err_send_error(ht_errtable_t *table, int code, thread_data_t *data);
static const char *ht_err_get_name(ht_errtable_t *table, int code);
static const char *ht_err_get_info(ht_errtable_t *table, int code);

/*-----------------------------------------------------------------------------
  Functions
------------------------------------------------------------------------------*/
/* Purpose: Create a error table object
 * In     : **table: error table
 * Return : NULL: error
 *          thread error table object
 */
ht_errtable_t *ht_errtable_new(ht_errtable_t **table)
{
	ht_errtable_t *tmp = NULL;

        if (table == NULL) {
                return NULL;
        }

/* Allocate memory space */
	tmp = (ht_errtable_t *)calloc(1, sizeof(ht_errtable_t));
        if (tmp == NULL) {
		return NULL;
        } else
		*table = tmp;
	
/* Initialize data fields */
	tmp->records = ht_err_table;
	
/* Initialize Ways        */
	tmp->delete = ht_errtable_delete;
	tmp->send_error = ht_err_send_error;
	tmp->get_name = ht_err_get_name;
	tmp->get_info = ht_err_get_info;

	return tmp;
}

/* Purpose: Destroy the error table object
 * In     : *table: the error table object
 * Return : void
 */
static void ht_errtable_delete(ht_errtable_t *table)
{
	if (table != NULL)
		free(table);
}

/* Purpose: Send error header to web client
 * In     : *table: the error table object
 *          code: error code
 *          *data: the thread data object
 * Return : void
 */
static void ht_err_send_error(ht_errtable_t *table, int code, thread_data_t *data)
{
	ht_err_t *err;
	int i;

        if ((table == NULL) || (data == NULL)) {
                return;
        }

	err = table->records;
	data->sendlen = 0;
	
	/* Find the error record     */
	for (i = 0; i < HT_ERRTAB_NUM; i++) {
		if (err[i].code == code)
			break;
	}
	if (i >= HT_ERRTAB_NUM) {
		return;
	}
	
	/* Fill the time            */
	data->sendlen = sprintf(data->sbuf,
				"HTTP/1.1 %d %s\r\n\r\n", 
				code, err[i].name);
	
	/* Fill the buffer into web client by socket  */
	(void)socket_write(data->sock, data->sbuf, data->sendlen, 1);
}

/* Purpose: Get the name of the error code
 * In     : *table: the error table object
 * Return : the name string address
 */
static const char *ht_err_get_name(ht_errtable_t *table, int code)
{
	ht_err_t *tmp;

	tmp = table->records;
	while (tmp->name != NULL) {
		if (tmp->code == code) {
			return tmp->name;
		}
		tmp++;
	}

	return NULL;
}

/* Purpose: Get the descriptor information of the error code
 * In     : *table: the error table object
 * Return : the descriptor information address
 */
static const char *ht_err_get_info(ht_errtable_t *table, int code)
{
	ht_err_t *tmp;

	tmp = table->records;
	while (tmp->name != NULL) {
		if (tmp->code == code) {
			return tmp->info;
		}
		tmp++;
	}	

	return NULL;
}

