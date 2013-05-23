#ifndef _TR_H_
#define _TR_H_
#include "cgi.h"

typedef struct _cgi_protocol_t{
	const char *name;                 /* function name*/
	unsigned int  flag;         	  /* function flag*/
/*flag low 4 bits 1-15 class*/
#define PROTOCOL_CLASS_SYS		1	
#define PROTOCOL_CLASS_DLD		2
#define PROTOCOL_CLASS_NET		3
#define PROTOCOL_CLASS_SEC		4
#define PROTOCOL_CLASS_SERV		5
#define PROTOCOL_CLASS_STOR		6

/*bit 5 means if need login 0->no need login 1->need login*/
#define PROTOCOL_LOGIN	1<<4

/*bit 6 means set/get 1->SET 0->GET */
#define PROTOCOL_FUCTION	1<<5
#define   PRO_FLAG_GET_MODE(flag)	((flag) &= ~(PROTOCOL_FUCTION))
#define   PRO_FLAG_SET_MODE(flag)	((flag) |= PROTOCOL_FUCTION)

/*bit 7 means xml/json 1->json 0->xml default xml */
#define PROTOCOL_FORMAT	1<<6
#define   PRO_FLAG_XML_MODE(flag)	((flag) &= ~(PROTOCOL_FORMAT))
#define   PRO_FLAG_JSON_MODE(flag)	((flag) |= (PROTOCOL_FORMAT))

int (*handler)(cgi_t*, struct _cgi_protocol_t*, sys_outbuf_t*); /*function handler*/
}cgi_protocol_t, *cgi_protocol_tp;


extern int pro_tr_base_handler(cgi_t *p, cgi_protocol_t *d, sys_outbuf_t *buf);
extern int pro_dld_tr_handler(cgi_t *p, cgi_protocol_t *d, sys_outbuf_t *buf);
extern int pro_tr_sinfo_handler(cgi_t *p, cgi_protocol_t *d, sys_outbuf_t *buf);
extern int pro_tr_arg_handler(cgi_t *p, cgi_protocol_t *d, sys_outbuf_t *buf);
extern int pro_tr_record_handler(cgi_t *p, cgi_protocol_t *d, sys_outbuf_t *buf);
extern int pro_tr_tasks_handler(cgi_t *p, cgi_protocol_t *d, sys_outbuf_t *buf);
extern int pro_tr_url_handler(cgi_t *p, cgi_protocol_t *d, sys_outbuf_t *buf);
extern void *tr_monitor();


#endif
