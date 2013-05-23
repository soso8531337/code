#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
#include <ctype.h>
#include <pwd.h>

#include "common.h"

/*-----------------------------------------------------------------------------
  Base system call operation
------------------------------------------------------------------------------*/
/* Purpose: read the regular file, it will read 'len' length into
 *          '*buf', or reach the end of the file.
 * In     : fd: the file descriptor fd
 *          *buf: the reading buffer
 *          len: the max. length to read
 * Return : -1: error
 *          >=0: the reading length
 */
int read_n(int fd, char *buf, int len)
{
        int n = len;
/*add by wangsn 080322*/
	if((fd < 0)||(buf == NULL)||(len < 0)){
		return -1;
	}
	
    while(n > 0) {
         	int tlen;

         	if ((tlen = read(fd, buf, n)) < 0) {
				if (errno == EINTR)
					tlen = 0;
				else
					return -1;
			} else if (tlen == 0) /* Reach at the end of file */
				break;
			
        	n -= tlen;
      	 	buf += tlen;
   }

        return len - n;
}

/* Purpose: write the regular file, it will write 'len' length of
 *          '*buf', or reach the end of the file.
 * In     : fd: the file descriptor fd
 *          *buf: the writing buffer
 *          len: the max. length to write
 * Return: -1: error
 *        >=0: the writing length
 */
int write_n(int fd, const char *buf, int len)
{
        int n = len;
/*add by wangsn 080322*/
	if((fd < 0)||(buf == NULL)||(len < 0)){
		return -1;
	}

        while (n > 0) {
                int tlen;

                if ((tlen = write(fd, buf, len)) <= 0) {
			if ((tlen < 0) && (errno == EINTR))
				tlen = 0;
			else {
				return -1;
		}
		}
		
                n -= tlen;
                buf += tlen;
        }

        return len;
}

/* Purpose : Set the nonblock for file descriptor
 * In      : fd: the file descriptor
 * Return  : SUCCESS: 1
 *           FAILURE: 0
 */
int fd_noblock(int fd)
{
	int flag;

	/* Set the non blocking mode   */
	if ((flag = fcntl(fd, F_GETFL, 0)) == -1) {
		return FAILURE;
	}
	if (fcntl(fd, F_SETFL, flag | O_NONBLOCK) != 0) {
		return FAILURE;
	}

	return SUCCESS;
}

/* Purpose : Set the close exec flag for file descriptor
 * In      : fd: the file descriptor
 * Return  : SUCCESS: 1
 *           FAILURE: 0
 */
int fd_cloexec(int fd)
{
	if (fcntl(fd, F_SETFD, 1) != 0){
		return FAILURE;
	}

	return SUCCESS;
}

/* Purpose: Decode a url code string, for example '%','+' etc.
 * Note   : Enable the same string(share space) for source string
 *          and destination string
 * In     : *src: the source string
 *          src_len: the source string length
 *          *dst: the destination sting
 *          dst_len: the destination string length
 * Return : void
 */
/*
void str_decode_url(const char *src, int src_len, char *dst, int dst_len)
{
        int     i, j, a, b;
	if(src == NULL){
//		SYS_LOG(io_log, "%s\n", "arg err");
		return ;
	}
#define HEXTOI(x)  (isdigit(x) ? x - '0' : x - 'W')

        for (i = j = 0; i < src_len && j < dst_len - 1; i++, j++)
                switch (src[i]) {
                case '%':
                        if (isxdigit(((unsigned char *) src)[i + 1]) &&
                            isxdigit(((unsigned char *) src)[i + 2])) {
                                a = tolower(((unsigned char *)src)[i + 1]);
                                b = tolower(((unsigned char *)src)[i + 2]);
                                dst[j] = (HEXTOI(a) << 4) | HEXTOI(b);
                                i += 2;
                        } else {
                                dst[j] = '%';
                        }
                        break;
                case '+':
                        dst[j] = ' ';
                        break;
                default:
                        dst[j] = src[i];
                        break;
                }

        dst[j] = '\0';  
}
*/

void str_decode_url(const char *src, int src_len, char *dst, int dst_len)
{
        int     i, j, a, b;
	if(src == NULL){
		return ;
	}
#define HEXTOI(x)  (isdigit(x) ? x - '0' : x - 'W')

        for (i = j = 0; i < src_len && j < dst_len - 1; i++, j++)
                switch (src[i]) {
                case '%':
                        if (isxdigit(((unsigned char *) src)[i + 1]) &&
                            isxdigit(((unsigned char *) src)[i + 2])) {
                                a = tolower(((unsigned char *)src)[i + 1]);
                                b = tolower(((unsigned char *)src)[i + 2]);
                                dst[j] = (HEXTOI(a) << 4) | HEXTOI(b);
                                i += 2;
                        } else {
                                dst[j] = '%';
                        }
                        break;
                case '+':
                        dst[j] = ' ';
                        break;
                default:
                        dst[j] = src[i];
                        break;
                }

        dst[j] = '\0';  
}


void str_encode_url(const char *src, int src_len, char **dst)
{
	char encoded[4096];
	unsigned char c;
	int  i,j = 0;

	if(src == NULL)
		return;
	memset(encoded, 0, 4096);
	for(i=0; i < src_len; i++){
		c = src[i];
		if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
			(c >= '0' && c <= '9') ||c == '.' || c == '-' || c == '_'){
			// allowed characters in a url that have non special meaning
			encoded[j] = c;
			j++;
			continue;
		}
		if(c == ' '){
			encoded[j] = '+';
			j++;
			continue;
		}
		j += sprintf(&encoded[j], "%%%x", c);
	}

	*dst = strdup(encoded);
}

char *str_memstr(void *s, int slen, void *mstr, int mlen)
{
	unsigned char *start = (unsigned char *)s;
	unsigned char *dst = (unsigned char *)mstr;
	
/* Add by wangsn 080320 */
	if ((s == NULL) || (mstr == NULL) || (slen < 0) || (mlen < 0)){
		return NULL;
	}

	while (start < ((unsigned char *)s + slen)) {
		if (*start == *((unsigned char *)dst)) {
			if (memcmp(start, mstr, mlen) == 0) {
				return (char *)start;
			}
		}
		start++;
	}

	return NULL;
}

/* Purpose: read any len length of buffer from fd
 * In     : fd: the buffer fd
 *          *base: the buffer base address
 *          len: the length of buffer
 *          *stop: the stop flag to read,
 *                 NULL: isn't stop flag
 *          timeout: the Max. time to wait read(Unit: second)
 *                   -1: waiting forever
 * Return: -1: Fail to receive data
 *         -2: The peer haved shutdown
 *         -3: the waiting timeout
 *         >0: other: read length
 */
int socket_read_anylen(int fd, char *base, int len, const char *stop, int timeout)
{
	int n = len;
	char *buf = base;
	if (base == NULL) {
		return -1;
	}
	
	while (n > 0) {
		int tret;
		fd_set readfd, exceptfd;
		struct timeval tv;

		FD_ZERO(&readfd);
		FD_ZERO(&exceptfd);
		FD_SET(fd, &readfd);
		FD_SET(fd, &exceptfd);

		if (timeout == -1) {
			tret = select(fd+1, &readfd, 0, &exceptfd, NULL);
		} else {
			tv.tv_sec = timeout; /* second */
			tv.tv_usec = 0;      /* us     */
			tret = select(fd+1, &readfd, 0, &exceptfd, &tv);
		}
		if (tret == 0) { /* Timeout */
			if(len > n)
				break;
			
			return -3;
		} else if (tret == -1) { /* Error */
			if (errno == EINTR)
				continue;
			else {
				return -1;
			}
		} /* else: receive data or except handle */
		if (FD_ISSET(fd, &exceptfd)) {
			return -1;
		}
		
		tret = recv(fd, buf, n, 0);
		if (tret < 0) {
			if ((errno == EINTR) || (errno == EWOULDBLOCK) 
			    || (errno == EAGAIN)) {
				continue;
			} else {
				return -1;
			}
		} else if (tret == 0) {  /* the peer haved shutdown, terminate */
			return -2;
		} else { /* Prepare to the next reading */
			n -= tret;
			buf += tret;
		}
		
		/* Check stop flag         */
		if (stop != NULL) {
			*buf = '\0';
			/* Got a stop flag */
			if (str_memstr(base, buf - base, (void *)stop,
				       strlen(stop)) != NULL) 
				break;
		}
	}

	return len - n;
}

/* Purpose: read the len length of buffer from fd
 * In     : fd: the buffer fd
 *          *base: the buffer base address
 *          len: the length of buffer
 *          *stop: the stop flag to read,
 *                 NULL: isn't stop flag
 *          timeout: the Max. time to wait read(Unit: second)
 *                   -1: waiting forever
 * Return: -1: Fail to receive data
 *         -2: The peer haved shutdown
 *         -3: the waiting timeout
 *         >0: other: read length
 */
int socket_read(int fd, char *base, int len, const char *stop, int timeout)
{
	int n = len;
	char *buf = base;
	if (base == NULL) {
		return -1;
        }
	while (n > 0) {
		int tret;
		fd_set readfd, exceptfd;
		struct timeval tv;

		FD_ZERO(&readfd);
		FD_ZERO(&exceptfd);
		FD_SET(fd, &readfd);
		FD_SET(fd, &exceptfd);

		if (timeout == -1) {
			tret = select(fd+1, &readfd, 0, &exceptfd, NULL);
		} else {
			tv.tv_sec = timeout; /* second */
			tv.tv_usec = 0;      /* us     */
			tret = select(fd+1, &readfd, 0, &exceptfd, &tv);
		}
		if (tret == 0) { /* Timeout */
			return -3;
		} else if (tret == -1) { /* Error */
			if (errno == EINTR){
				continue;
			}else {
				return -1;
			}
		} /* else: receive data or except handle */
		if (FD_ISSET(fd, &exceptfd)) {
			return -1;
		}
		
		if (FD_ISSET(fd, &readfd)){
			tret = recv(fd, buf, n, 0);
			if (tret < 0) {
				if ((errno == EINTR) || (errno == EWOULDBLOCK) 
				    || (errno == EAGAIN)) {
					continue;
				} else {
					return -1;
				}
			} else if (tret == 0) {  /* the peer haved shutdown, terminate */
				return -2;
			} else { /* Prepare to the next reading */
				n -= tret;
				buf += tret;
			}
		
			/* Check stop flag         */
			if (stop != NULL) {
				*buf = '\0';
				/* Got a stop flag */
				if (str_memstr(base, buf - base, (void *)stop,
					       strlen(stop)) != NULL) 
					break;
			}
		}
	}
	return len - n;
}

/* Purpose: write the len length of buffer into fd
 * In     : fd: the buffer fd
 *          *buf: the buffer
 *          len: the length of buffer
 *          timeout: the Max. time to wait write 
 * Return :-1: Fail to write data
 *         -2: The peer haved shutdown
 *         -3: the waiting timeout
 *        >=0: other: written length
 */
int socket_write(int fd, const char *buf, int len, int timeout)
{
	int n = len;
        if (buf == NULL) {
		return -1;
        }

	while (n > 0) {
		int tret;
		fd_set writefd, exceptfd;
		struct timeval tv;

		FD_ZERO(&writefd);
		FD_ZERO(&exceptfd);
		FD_SET(fd, &writefd);
		FD_SET(fd, &exceptfd);

		if (timeout == -1) {
			tret = select(fd+1, 0, &writefd, &exceptfd, NULL);
		} else {
			tv.tv_sec = timeout; /* second */
			tv.tv_usec = 0;      /* us     */
			tret = select(fd+1, 0, &writefd, &exceptfd, &tv);
		}
		if (tret == 0) { /* Timeout */
			return -3;
		} else if (tret == -1) { /* Error */
			if (errno == EINTR)
				continue;
			else {
				return -1;
			}
		} /* else: send data or exception handler */
		if (FD_ISSET(fd, &exceptfd)) {
			return -1;
		}		

		tret = n < 0x2000 ? n : 0x2000;
		tret = send(fd, buf, tret, 0);
		if (tret == -1) {
			if ((errno == EINTR) || (errno == EWOULDBLOCK) 
			    || (errno == EAGAIN) || (errno == ENOBUFS )){
				continue;
			}else if (errno == ECONNRESET) {
				return -2;
			} else {
				return -1;
			}
		}
		n -= tret;
		buf += tret;
	}

	return len;
}


/*-----------------------------------------------------------------------------
  String operation
------------------------------------------------------------------------------*/
/* Purpose: Get starting address from a sub string form string
 *          The function is safe, don't change the source string content
 * In     : *src: the source string
 *        : *sub: the sub string
 *          delm: separator character
 * Return : the starting address of sub string
 *          NULL: Isn't sub string in the src string
 */
char *str_substr(const char *src, const char *sub, char delm)
{
        const char *start, *end;
        int len, slen;
/*add by wangsn 080322*/
	if((src == NULL)||(sub == NULL)||delm == 0){
		return NULL;
	}

        slen = strlen(sub);
        start = src;
        while (*start != '\0') {
                /* Find a sub string according to delm */
                for (len = 0, end = start;
                     (*end != delm) && (*end != '\0');
                     end++, len++);
                if ((slen == len) && (strncmp(start, sub, slen) == 0)) {
                        return (char *)start;
                }

                /* Search the next sub string  */
                if (*end == delm)
                        start = end+1;
                else
                        start = end;
        }
        return NULL;
}

/* Purpose: Trim the left ' ' or '\t' character.
 * In     : *str: the source string 
 * Return : the starting address after trim.
 */
char *str_ltrim(char *str)
{
	if(str != NULL)/*add by wangsn 08032*/
		while ((*str == ' ') || (*str == '\t')) str++;
	
	return str;
}

/* Purpose: Trim the right ' ' or '\t' character
 * In     : *str: the source string
 * Return : the starting address after trim.
 */
char *str_rtrim(char *str)
{
	int len;

	if(str == NULL){/*add by wangsn 08032*/
		return NULL;
	}

	if (strlen(str) == 0)
		return str;
	
	/* Kill ' ' and '\t' */
	len = strlen(str) - 1;
	while ((str[len] == ' ') || (str[len] == '\t')) {
		if (len)
			len--;
		else {
			str[0] = '\0';
			return str;
		}
	}
	str[len+1] = '\0';
	
	return str;
}

/* Purpose: Trim the right '\n'
 * In     : *str: the souce string
 * Return : the starting address after string handled
 */
char *str_kill_lf(char *str)
{
	int len;

	if(str == NULL){/*add by wangsn 08032*/
		return NULL;
	}

	if (strlen(str) == 0)
		return str;

	len = strlen(str) - 1;
	/* Kill '\n'   */
	while (str[len] == '\n') {
		if (len > 0)
			len--;
		else {
			str[0] = '\0';
			return str;
		}
	}
	str[len+1] = '\0';
	
	return str;
}

/* Purpose: Trim the right ' ', '\t' and '\n'
 * In     : *str: the source string
 * Return : the starting address after string handled
 */
char *str_rtrim_lf(char *str)
{
	int len;

	if(str == NULL){/*add by wangsn 08032*/
		return NULL;
	}

	if (strlen(str) == 0)
		return str;
	
	len = strlen(str) - 1;
	/* Kill ' ','\t','\n' */
	while ((str[len] == ' ') || (str[len] == '\t') || (str[len] == '\n')) {
		if (len > 0)
			len--;
		else {
			str[0] = '\0';
			return str;
		}
	}
	str[len+1] = '\0';

	return str;
}

/* Purpose: Trim the left and the right ' ' or '\t' character
 * In     : *str: the source string
 * Return : the starting address after string trim
 */
char *str_lrtrim(char *str)
{
	char *tmp;
/*add  by wangsn 080320*/
	if(str == NULL){
		return NULL;
	}
	
	tmp = str_ltrim(str);
	tmp = str_rtrim(tmp);

	return tmp;
}

/* Purpose   : Test if the str is empty string
 * Parameters: *str: the string
 * Return    : 1: Empty string
 *             0: Isn't emptry
 *            -1: err argument
 */
int str_empty(const char *str)
{
/*add  by wangsn 080320*/
	if(str == NULL){
		return -1;
	}
	return str[0] == '\0' ? 1 : 0;
}

/*-----------------------------------------------------------------------------
  Configure file class
------------------------------------------------------------------------------*/
/* Purpose: Delete the configure file object
 * In     : *conf: the configure file object
 * Return : void
 */
static void sys_conf_delete(sys_conf_t *conf)
{
	sys_confn_t *rec, *tmp;

	if (conf == NULL)
		return;
	
	if (conf->file != NULL)
		free(conf->file);
	
	if (!list_empty(&conf->head)) {
		list_for_each_entry_safe(rec, tmp, &conf->head, node) {
			list_del(&rec->node);
			if (rec->val != NULL)
				free(rec->val);
			free(rec);
		}
	}
	
	free(conf);
}


/* Purpose: Read the configure file
 * Note   : 1. If have list, free it, 
 *          2. read configure file again.
 * In     : *conf: the configure file object
 * Return : 0: Failure
 *          1: Success
 */
static int sys_conf_read(sys_conf_t *conf)
{
	FILE *fp = NULL;
	char line[LINE_MAX], *tstr;
	sys_confn_t *rec, *tmp;
	
	/* Open configure file     */
	if (conf->file == NULL)
		return FAILURE;
	
	if ((fp = fopen(conf->file, "r")) == NULL)
		return FAILURE;

	/* Delete all records in the list */
	if (!list_empty(&conf->head)) {
		list_for_each_entry_safe(rec, tmp, &conf->head, node) {
			list_del(&rec->node);
			if (rec->val != NULL)
				free(rec->val);
			free(rec);
		}
	}
	INIT_LIST_HEAD(&conf->head);

	/* Get the variables list again */
	while(fgets(line, LINE_MAX-1, fp) != NULL) {
		char *t;
		
		tstr = str_ltrim(line);
		if ((*tstr == '#') || (*tstr == '\n') || (*tstr == ';'))
			continue;
		
		/* Get the variable name */
		if ((t = strchr(tstr, '=')) == NULL)
			continue;
		else
			*t++ = '\0';

		/* Generate a conf record */
		if ((rec = calloc(1, sizeof(sys_confn_t))) == NULL)
			goto READ_ERR;
		INIT_LIST_HEAD(&rec->node);

		/* Get the name    */
		tstr = str_rtrim(tstr);
		strncpy(rec->name, tstr, SYS_CONF_NAMELEN);
		rec->name[SYS_CONF_NAMELEN-1] = '\0';
		/* Get the value   */
		t = str_lrtrim(t);
		t = str_kill_lf(t);
		if ((rec->val = strdup(t)) == NULL)
			goto READ_ERR;

		list_add_tail(&rec->node, &conf->head);
	}
	fclose(fp);
	return SUCCESS;

READ_ERR:
	/* Delete all records in the list */
	if (!list_empty(&conf->head)) {
		list_for_each_entry_safe(rec, tmp, &conf->head, node) {
			list_del(&rec->node);
			if (rec->val != NULL)
				free(rec->val);
			free(rec);
		}
	}
	INIT_LIST_HEAD(&conf->head);

	if (fp != NULL)
		fclose(fp);
	return FAILURE;
}

/* Purpose: 1. Set the configure file name
 *          2. Call sys_conf_read() to re-build configure file list
 * In     : *conf: the configure file object
 *          *f: the configure file name
 * Return : 0: Failure
 *          1: Success
 */
static int sys_conf_fname(sys_conf_t *conf, const char *f)
{
	struct stat st;

	if (f == NULL)
		return FAILURE;

	/* Free the configure file memory space */
	if (conf->file != NULL) {
		free(conf->file);
		conf->file = NULL;
	}
	
	if (stat(f, &st) == -1) {
		int fd;
		fd = open(f, O_RDWR|O_CREAT, S_IRUSR|S_IWUSR);
		if (fd != -1) {
			close(fd);
		} else
			return FAILURE;
	}

	if ((conf->file = strdup(f)) == NULL)
		return FAILURE;

	/* Re-read configure file               */
	return sys_conf_read(conf);
}

/* Purpose: Write the list into the configure file
 * In     : *conf: the configure file object
 * Return : 0: Failure
 *          1: Success
 */
static int sys_conf_sync(sys_conf_t *conf)
{
	FILE *fp = NULL;
	char line[LINE_MAX];
	sys_confn_t *rec;
	
	/* Open configure file  */
	if (conf->file == NULL)
		return FAILURE;

	if ((fp = fopen(conf->file, "w+")) == NULL)
		return FAILURE;	
	
	/* Write the variables list */
	list_for_each_entry(rec, &conf->head, node) {
		sprintf(line, "%s=%s\n", rec->name, rec->val);
		fputs(line, fp);
	}
	
	fclose(fp);
	return SUCCESS;
}

/* Purpose: Find the variable from the list
 * Note   :  If the configure file type is SYS_CONF_MLDONKEY,
 *           the function will allocate memory space for variable value,
 *           the caller need free the space
 * In     : *conf: the configure file object
 *          *r: the variable name
 * Return : NULL: Miss the '*r' variable name
 *          the value starting address
 */
static char *sys_conf_find(sys_conf_t *conf, const char *r)
{
	sys_confn_t *rec;

	/* If mldonkey configure file, ignore the way */
	if (conf->file == NULL)
		return NULL;
	
	/* Find the variable  */
	list_for_each_entry(rec, &conf->head, node) {
		if (strcmp(r, rec->name) == 0)
			return rec->val;
	}

	return NULL;
}

/* Purpose: Add the variable and value into the list
 * In     : *conf: the configure file object
 *          *r: the variable name
 *          *v: the value
 * Return : 0: Failure
 *          1: Success
 */
static int sys_conf_add(sys_conf_t *conf, const char *r, const char *v)
{
	sys_confn_t *rec;

	/* If mldonkey configure file, ignore the way */
	if (conf->file == NULL)
		return FAILURE;
	
	/* Generate a record  */
	if ((rec = calloc(1, sizeof(sys_confn_t))) == NULL)
		return FAILURE;
	/* Set variable name  */
	strncpy(rec->name, r, SYS_CONF_NAMELEN);
	rec->name[SYS_CONF_NAMELEN-1] = '\0';
	/* Set variable value */
	if ((rec->val = strdup(v)) == NULL) {
		free(rec);
		return FAILURE;
	}

	/* Add the record into the list */
	list_add_tail(&rec->node, &conf->head);
	return SUCCESS;
}

/* Purpose: Delete the variable and value from the list
 * In     : *conf: the configure file object
 *          *r: the variable name
 * Return : 0: Failure
 *          1: Success
 */
static int sys_conf_del(sys_conf_t *conf, const char *r)
{
	sys_confn_t *rec, *tmp;	
	int found = FAILURE;

	/* If mldonkey configure file, ignore the way */
	if (conf->file == NULL)
		return FAILURE;
	
	if (!list_empty(&conf->head)) {
		list_for_each_entry_safe(rec, tmp, &conf->head, node) {
			if (strcmp(rec->name, r) == 0) {
				list_del(&rec->node);
				if (rec->val != NULL)
					free(rec->val);
				free(rec);
				found = SUCCESS;
				break;
			}
		}
	}
	
	return found;
}

/* Purpose: Update the variable and value into the list
 * In     : *conf: the configure file object
 *          *r: the variable name
 *          *v: the value
 * Return : 0: Failure
 *          1: Success
 */
static int sys_conf_update(sys_conf_t *conf, const char *r, const char *v)
{
	sys_confn_t *rec;
	int found = FAILURE;

	if (conf->file == NULL)
		return FAILURE;
	
	/* Find the variable  */
	list_for_each_entry(rec, &conf->head, node) {
		if (strcmp(rec->name, r) == 0) {
			char *old = rec->val;
			rec->val = strdup(v);
			if (rec->val == NULL) {
				rec->val = old;
			} else {
				free(old);
				found = SUCCESS;
			}
			break;
		}
	}

	return found;
}

/* Purpose: Create a new configure file object
 * In     : **conf: the pointer which store the configure file object
 * Return : NULL: Error
 *          the valid configure file object
 */
sys_conf_t *sys_conf_new(sys_conf_t **conf)
{
	sys_conf_t *tmp;
	
	if ((tmp = calloc(1, sizeof(sys_conf_t))) == NULL) {
		*conf = NULL;
		return NULL;
	}
	INIT_LIST_HEAD(&tmp->head);

	tmp->delete = sys_conf_delete;

	tmp->fname  = sys_conf_fname;
	tmp->read   = sys_conf_read;
	tmp->sync   = sys_conf_sync;
	tmp->find   = sys_conf_find;
	tmp->add    = sys_conf_add;
	tmp->del    = sys_conf_del;
	tmp->update = sys_conf_update;
	
	return tmp;
}

/* Purpose    : Write string into output buffer
 * Parameters : 
 *    *buf    : the output buffer object
 *    *str    : the writing string
 *    len     : the length of writing string
 * Return     : SUCCESS (1)
 *              FAILURE (0)
 */
static int sys_outbuf_output(sys_outbuf_t *buf, const char *str, int len)
{
	const char *tstr = str;

	while (len > 0) {
                /* If leave some string, allocate a new buffer block */
		if ((buf->cur == NULL) || 
		    (buf->cur->len >= (buf->blksize - 1))) {
			sys_bufblk_t *blk;
			if ((blk = calloc(1, sizeof(sys_bufblk_t))) == NULL)
				return FAILURE;
			blk->buf = calloc(1,buf->blksize);
			if (blk->buf == NULL) {
				free(blk);
				return FAILURE;
			}
		
			slist_add_tail(buf->head, blk, sys_bufblk_t, next);
			buf->cur = blk;
		}

		if (buf->cur->len < (buf->blksize - 1)) {
			int cplen = buf->blksize - buf->cur->len - 1;
			cplen = (cplen > len) ? len : cplen;
			memcpy((void *)&buf->cur->buf[buf->cur->len],
			       (void *)tstr, cplen);
			buf->cur->len += cplen;
			buf->cur->buf[buf->cur->len] = '\0';

			tstr += cplen;
			len -= cplen;
			buf->bufsize += cplen;
		}
	}

	return SUCCESS;
}

/* Purpose    : Free block list in the buffer object
 * Parameters : 
 *    *buf    : the output buffer object
 * Return     : void
 */
static void sys_outbuf_clear(sys_outbuf_t *buf)
{
	sys_bufblk_t *head, *tmp;
	
	head = buf->head;
	while (head != NULL) {
		tmp = head;
		head = head->next;
		
		if (tmp->buf)
			free(tmp->buf);
		free(tmp);
	}

	buf->head = NULL;
	buf->cur  = NULL;
	buf->bufsize = 0;
}

/* Purpose    : Set the block size
 * Parameters : 
 *    *buf    : the output buffer object
 *    blksize : the block size
 * Return     : void
 */
static void sys_outbuf_set_blksize(sys_outbuf_t *buf, int blksize)
{
	buf->blksize = blksize;
}

/* Purpose    : Get the buffer length in the buffer
 * Parameters : 
 *    *buf    : the output buffer object
 * Return     : the buffer length(Unit: Byte)
 */
static long sys_output_get_bufsize(sys_outbuf_t *buf)
{
	return buf->bufsize;
}

/* Purpose    : Output the block list in the buffer object
 *              to 'fd' file or socket.
 * Parameters : 
 *    *buf    : the output buffer object
 *    fd      : the file descriptor to write
 * Return     : SUCCESS: 1
 *              FAILURE: 0
 */
static int sys_outbuf_print(sys_outbuf_t *buf, int fd)
{
	int n=0;
	sys_bufblk_t *head;
	head = buf->head;
	while (head != NULL) {
		n = socket_write(fd, head->buf, head->len, 1);
		if(n < 0){
			return FAILURE;
		}
		head = head->next;
	}

	return SUCCESS;
}

/* Purpose    : Free the block list and destroy buffer object
 * Parameters : 
 *    *buf    : the output buffer object
 * Return     : void
 */
static void sys_outbuf_del(sys_outbuf_t *buf)
{
	if (buf) {
		buf->clear(buf);
		free(buf);
	}
}

/* Purpose    : Create a outputing buffer object
 * Parameters : 
 *    **buf   : the pointer to store output buffer object
 * Return     : the buffer object
 */
sys_outbuf_t *sys_outbuf_new(sys_outbuf_t **buf)
{
	sys_outbuf_t *tmp;
	
	tmp = calloc(1, sizeof(sys_outbuf_t));
	if (tmp == NULL)
		return NULL;
	else
		*buf = tmp;
	
	/* Ways   */
	tmp->del     = sys_outbuf_del;

	tmp->output  = sys_outbuf_output;
	tmp->clear   = sys_outbuf_clear;
	tmp->set_blksize = sys_outbuf_set_blksize;
	tmp->get_bufsize = sys_output_get_bufsize;
	tmp->print   = sys_outbuf_print;

	/* default value  */
	tmp->blksize = 4096;

	return tmp;
}

void str_to_upper(char *str)
{
	if(str == NULL){
		return;
	}
	
	while (*str != '\0') {
		if (islower(*str))
			*str = toupper(*str);
		str++;
	}
}

static int str_iconv1(char *fcode, char *sstr, int slen, char **dstr, int *dlen)
{
	iconv_t cd = (iconv_t)(-1);
	char *inbuf, *outbuf;
	char *dst = NULL;
	size_t len, inbyteleft, outbyteleft;
        if(fcode == NULL || sstr == NULL) {
		return 0;
        }
	len = outbyteleft = slen * (size_t) 4U + (size_t) 1U;
        if ((dst = calloc(len, (size_t)1U)) == NULL) {
		return 0;
        } else
		outbuf = dst;

	str_to_upper(fcode);
	if ((cd = iconv_open("UTF-8", fcode)) == (iconv_t)(-1)) {
		free(dst);
		return 0;
	}
	inbuf = sstr;
	inbyteleft = slen;

	len = iconv(cd, &inbuf, &inbyteleft, &outbuf, &outbyteleft);
	if (len == -1) {
		free(dst);
		iconv_close(cd);
		return 0;
	} else {
		*dlen = strlen(dst);
	}
	
	*dstr = dst;
	(*dstr)[*dlen] = '\0';

	iconv_close(cd);

	return 1;
}

int str_to_utf8(char *str, char **outbuf)
{
	int dlen;

	if (str_iconv1("UTF-8", str, strlen(str), outbuf, &dlen)) {
                MCEMT_DBG("UTF-8\n");
		return 1;
	} else if (str_iconv1("GBK", str, strlen(str), outbuf, &dlen)) {
	        MCEMT_DBG("GBK\n");
		return 1;
	} else if (str_iconv1("GB2312", str, strlen(str), outbuf, &dlen)) {
                MCEMT_DBG("GB2312\n");
                return 1;
	} else if (str_iconv1("BIG-5", str, strlen(str), outbuf, &dlen)) {
                MCEMT_DBG("BIG-5\n");
                return 1;
	} else {
		return 0;
	}
}

void str_unit_to_cap( long long num, int unit, char *sbuf, int slen)
{
        long long cap = 0;
	double f;
	if(sbuf == NULL){
		return;
	}
	if (unit & STR_UNIT_BLOCK)  /* Blocks */
		cap = num << 10;
	else if (unit & STR_UNIT_SECTOR) /* Sector */
		cap = num << 5;
	else if (unit & STR_UNIT_BYTE)
		cap = num;
	else 
		return; /*add by wangsn*/

	if (cap < 1024) { /* B  */
		snprintf(sbuf, slen, "%lldB", cap);
		return;
	}

	cap = cap/1024;  /* KB */
	if (cap < 1024) { 
		snprintf(sbuf, slen, "%lldKB", cap);
		return;
	}

	cap = cap/1024;  /* MB */
	if (cap < 1024) {
		snprintf(sbuf, slen, "%lldMB", cap);
		return;
	}

	f = (double)cap/(double)1024;
	cap = cap/1024; /* GB  */
	if (cap < 1024) {
		snprintf(sbuf, slen, "%3.2fGB", f);
		return;
	}

	f = (double)cap/(double)1024;
	cap = cap/1024; /* TB */
	snprintf(sbuf, slen, "%3.3fTB", f);

	return;
}

void str_to_lower(char *str)
{
        if(str == NULL){
                return;
        }
 
        while (*str != '\0') {
                if (isupper(*str))
                        *str = tolower(*str);
                str++;
        }
}

static char char_get_val(char ch)
{
        char retval = 'A';

        if ((ch > 0) && (ch < 9)) {
                retval = ch + '0';
        }
        if ((ch > 0xA) && (ch < 0xF)) {
                retval = ch - 0xA + 'A';
        }

        return retval;
}

/* Purpose: Get the 1-9, A-F ASCII
 * In     : *pname: the source string
 *          *utf8name: the destination string
 * Return : 0: FAILURE
 *          1: SUCCESS
 */
int str_to_ascii(char *pname, char *utf8name)
{
        int i, j;

        for (i = j = 0; (pname[i] != '\0') && (i < 32); i++) {
                utf8name[j++] = char_get_val(pname[i]&0x0F);
                utf8name[j++] = char_get_val((pname[i]&0xF0)>>4);
        }
        utf8name[j] = '\0';

        return (int)1;
}

char *user_name(int uid)
{
	struct passwd *passwd = NULL;
	static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

	pthread_mutex_lock(&mutex);     /* lock   */
	/* get uid   */
	passwd = getpwuid(uid);
	if (passwd != NULL){
		pthread_mutex_unlock(&mutex);   /* unlock */
		return passwd->pw_name;
	}
	pthread_mutex_unlock(&mutex);   /* unlock */
	
	return NULL;
}

int dm_daemon(void)
{
	int pid, i;
	
	switch(fork())
	{
		/* fork error */
	case -1:
		exit(1);
	
		/* child process */
	case 0:
		/* obtain a new process group */
		if((pid = setsid()) < 0) {
			exit(1);
		}

		/* close all descriptors */
		for (i = getdtablesize(); i >= 0; --i) 
			close(i);

		i = open("/dev/null", O_RDWR); /* open stdin */
		dup(i); /* stdout */
		dup(i); /* stderr */

		umask(000);

		//umask(027);

		return SUCCESS;

		/* parent process */
	default:
		exit(0);
	}

	return FAILURE;
}
