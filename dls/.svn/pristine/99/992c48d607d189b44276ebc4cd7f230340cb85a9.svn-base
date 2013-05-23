/******************************************************************************
 * Copyright (C) 2007 by IOVST
 *
 * File: xml.c
 *
 * Date: 2007-07-27
 *
 * Author: Liu Yong
 *
 * Descriptor:
 *   mini-XML interface
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
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

/*-----------------------------------------------------------------------------
  Local header files 
------------------------------------------------------------------------------*/
#include "common.h"
#include "xml.h"

/*-----------------------------------------------------------------------------
  Functions
------------------------------------------------------------------------------*/
/* Purpose  : Add element into buffer
 * In       : flag:
 *             #define XML_ELEM_START   (1)  // For example: <user>  
 *             #define XML_ELEM_END     (2)  // For example: </user>
 *             #define XML_LABEL        (3)  // For example: <name>value</name>
 *            *elem: label or element name
 *            *val: the label value
 *            *buf: the buffer object
 * Return   : SUCCESS: 1
 *            FAILURE: 0
 */
int xml_add_elem(int flag, const char *elem, const char *val, sys_outbuf_t *buf)
{
	int len = 0;
	
	if (flag == XML_ELEM_START) {
		char str[256] = "";
		len = sprintf(str, "<%s>", elem);
		buf->output(buf, str, len);
	} else if (flag == XML_ELEM_END) {
		char str[256] = "";
		len = sprintf(str, "</%s>", elem);
		buf->output(buf, str, len);
	} else if (flag == XML_LABEL) {
		char *str = NULL;
		if(val != NULL)
			len = strlen(val);
		
		if ((str = malloc(len+256)) != NULL) {
			len = sprintf(str, "<%s>%s</%s>", elem, val, elem);
			buf->output(buf, str, len);
			free(str);
		}
	} else
		return FAILURE;

	return SUCCESS;
}

/* Purpose  : Add XML header
 * In       : flag:
 *            *buf: the buffer object
 * Return   : SUCCESS: 1
 *            FAILURE: 0
 */
int xml_add_header(sys_outbuf_t *buf)
{
	char str[128];

	strcpy(str, "<?xml version=\"1.0\" ?>");
	buf->output(buf, str, strlen(str));
	strcpy(str, "<root>");
	buf->output(buf, str, strlen(str));
	
	return SUCCESS;
}

/* Purpose  : Add XML end
 * In       : flag:
 *            *buf: the buffer object
 * Return   : SUCCESS: 1
 *            FAILURE: 0
 */
int xml_add_end(sys_outbuf_t *buf)
{
	char str[128];

	strcpy(str, "</root>");
	buf->output(buf, str, strlen(str));
	
	return SUCCESS;
}
