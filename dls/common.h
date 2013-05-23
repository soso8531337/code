#ifndef _COMMON_H
#define _COMMON_H                  1
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iconv.h>
#include <stdio.h>

#ifdef MEM_LEAK
#include "memleak.h"
#endif
#include "list.h"

#ifndef LINE_MAX    /* The Max. length of input line         */
#define LINE_MAX              (2048)
#endif

#define FAILURE                0
#define SUCCESS                1

/*
 * Unified socket address
 */
typedef struct _net_usa_t {
	socklen_t len;
	union {
		struct sockaddr sa;
		struct sockaddr_in sin;
		struct sockaddr_in6 sin6;
	};
} net_usa_t, *net_usa_tp;

/*
 * Configure file class
 */
/* Configure file record  */
typedef struct _sys_confn_t {
	struct list_head node; /* Record node */

#define SYS_CONF_NAMELEN       (64)
	char name[SYS_CONF_NAMELEN];
	char *val;
} sys_confn_t, *sys_confn_tp;

typedef struct _sys_conf_t {
	struct list_head head;  /* Configure option list */

	char *file;   /* Configure file name  */

/* Ways */
	void (*delete)(struct _sys_conf_t *conf);
	int (*fname)(struct _sys_conf_t *conf, const char *f);
	int (*read)(struct _sys_conf_t *conf);
	int (*sync)(struct _sys_conf_t *conf);
	char *(*find)(struct _sys_conf_t *conf, const char *n);
	int (*add)(struct _sys_conf_t *conf, const char *n, const char *v);
	int (*del)(struct _sys_conf_t *conf, const char *n);
	int (*update)(struct _sys_conf_t *conf, const char *n, const char *v);
} sys_conf_t, sys_conf_tp;

sys_conf_t *sys_conf_new(sys_conf_t **conf);

/* 
 * Output buffer class
 */
/* Store output data into buffer */
typedef struct _sys_bufblk_t {
	struct _sys_bufblk_t *next;
	char *buf;
	long len;
} sys_bufblk_t, *sys_bufblk_tp;
	

typedef struct _sys_outbuf_t {
	long bufsize;  /* Buffer size              */
	int blksize;   /* Buffer block size        */
	sys_bufblk_t *head; /* buffer list         */
	sys_bufblk_t *cur;  /* current used buffer */
	
/* Way */
	void (*del)(struct _sys_outbuf_t *);

	int (*output)(struct _sys_outbuf_t *, const char *str, int len);
	void (*clear)(struct _sys_outbuf_t *);
	void (*set_blksize)(struct _sys_outbuf_t *, int blksize);
	long (*get_bufsize)(struct _sys_outbuf_t *);
	int (*print)(struct _sys_outbuf_t *, int fd);

} sys_outbuf_t, *sys_outbuf_tp;

void str_decode_url(const char *src, int src_len, char *dst, int dst_len);
void str_encode_url(const char *src, int src_len, char **dst);
int read_n(int fd, char *buf, int len);
int write_n(int fd, const char *buf, int len);
int socket_read_anylen(int fd, char *base, int len, const char *stop, int timeout);
int socket_read(int fd, char *base, int len, const char *stop, int timeout);
int socket_write(int fd, const char *buf, int len, int timeout);
int fd_noblock(int fd);
int fd_cloexec(int fd);
sys_outbuf_t *sys_outbuf_new(sys_outbuf_t **buf);
#define STR_UNIT_BLOCK       (0x01)
#define STR_UNIT_SECTOR      (0x02)
#define STR_UNIT_BYTE        (0x04)
extern void str_unit_to_cap(long long num, int unit, char *sbuf, int slen);
int str_to_utf8(char *str, char **outbuf);
extern void str_to_lower(char *str);
extern void str_to_upper(char * str);
int str_to_ascii(char *pname, char *utf8name);
char *user_name(int uid);
int dm_daemon(void);


#define DEBUG
#if defined(DEBUG)
#define MCEMT_DBG(fmt, arg...)                          \
{                                                       \
        char *fstr = (char *)strrchr((char *)__FILE__, '/');    \
        if (fstr == NULL) {                             \
                fprintf(stderr, "(%s,%s,%d)"fmt, __FILE__, __FUNCTION__, __LINE__, ##arg);  \
        } else {                                                \
                fprintf(stderr, "(%s,%s,%d)"fmt, fstr+1, __FUNCTION__, __LINE__, ##arg);  \
        } \
}       
#else   
#define MCEMT_DBG(fmt, arg...)
#endif
#endif

