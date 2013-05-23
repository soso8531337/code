/******************************************************************************
 * Copyright (C) 2007 by IOVST
 *
 * File: xml.h
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
#include "common.h"
/*-----------------------------------------------------------------------------
  Local header files 
------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
  Function declaration
------------------------------------------------------------------------------*/
#define XML_ELEM_START   (1)  /* For example: <user>  */
#define XML_ELEM_END     (2)  /* For example: </user> */
#define XML_LABEL        (3)  /* For example: <name>value</name> */


extern int xml_add_elem(int flag, const char *elem, const char *val, sys_outbuf_t *buf);
extern int xml_add_header(sys_outbuf_t *buf);
extern int xml_add_end(sys_outbuf_t *buf);
