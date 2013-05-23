#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
#include <iconv.h>
#include <assert.h>
#include <sys/stat.h> 
#include <fcntl.h>
#include <pthread.h>

#include "json.h"

int sockfd;

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



int receive_data(int s, char *buf, int length, int timeout)
{
        MCEMT_DBG("receive_data starts\n");
        int n;
        struct pollfd fds[1]; /* for the poll */
        MCEMT_DBG("#########\n");
        fds[0].fd = s;
        fds[0].events = POLLIN;
        MCEMT_DBG("#########\n");
        n = poll(fds, 1, timeout);

        MCEMT_DBG("#########\n");
        if (n < 0) {
                fprintf(stderr, "poll error");
                return 0;
        } else if (n == 0) {
                return 0;
        }
        MCEMT_DBG("#########\n");
        n = recv(s, buf, length, 0);
        MCEMT_DBG("#########\n");
        if (n < 0) {
                fprintf(stderr, "recv");
                return 0;
        }
        MCEMT_DBG("receive_data finishes\n");
        return n;
}



int tr_opensock(const char *host)
{
        int s;
        struct hostent *hp;
        struct sockaddr_in dest;
        struct hostent hostinfo;
        char   dns_buff[8192];
        int rc;
 
        gethostbyname_r(host, &hostinfo, dns_buff, 8192, &hp, &rc);
        if (hp == NULL) {
                herror(host);
                return 0;
        }
        if (hp->h_addr == NULL)
                return 0;
        memcpy(&dest.sin_addr, hp->h_addr, sizeof(dest.sin_addr));
        memset(dest.sin_zero, 0, sizeof(dest.sin_zero));
        s = socket(PF_INET, SOCK_STREAM, 0);
        if(s < 0) {
                perror("socket");
                return 0;
        }

        dest.sin_family = AF_INET;
        dest.sin_port = htons(9005);
        
	if(connect(s, (struct sockaddr *)&dest, sizeof(struct sockaddr_in)) < 0){
                perror("connect");
                close(s);
                return 0;
        }
 
       return s;
}
        
/*in: 
        s is socket,
 out:
 success:
        return 0;
 error:
        return -1;
*/
int xl_getsysinfo(int s, char *bind_key)
{
        MCEMT_DBG("enter xl_getsysinfo\n");
        struct json_object *args = NULL, *obj = NULL;
        int n = 0, ret = 0;
        char *retbuf = NULL, *tbuf = NULL; 

        char str[]={
                "GET /getsysinfo HTTP/1.1\r\n"
                "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
                "Accept-Language: zh-cn,zh;q=0.8,en-us;q=0.5,en;q=0.3\r\n"
                "Connection: keep-alive\r\n"
                "Cache-Control: max-age=0\r\n\r\n"
                };

        
        while((retbuf = (char*)calloc(1024, sizeof(char))) ==NULL);
        while((tbuf = (char*)calloc(1024, sizeof(char))) == NULL);
        

        if(send(s, str, strlen(str), 0) < 0){
                MCEMT_DBG("send failure\n");
                ret = -1;
                goto XL_ERR;
        }else{
                MCEMT_DBG("send success\n");
        }

        n = receive_data(s, retbuf, 1024, 60000);
        if(n == 0){
                ret = -1;
                goto XL_ERR;
        }
        char *tstr = NULL;
        tstr = strstr(retbuf, "\r\n\r\n");
        if(tstr){
		tstr += 4;
		/* Delete the empty lines */
		while (*tstr == '\n') 
                        tstr++;
	}
        
        MCEMT_DBG("%s\n", tstr);
        
        args = json_tokener_parse(tstr);
        if(args == NULL){
                ret = -1;
                goto XL_ERR;
        }
                
        obj = json_object_array_get_idx(args, 4);
        if(obj != NULL){
                strcpy(bind_key, json_object_get_string(obj));
        }
        printf("%s\n", bind_key);
                     
XL_ERR:
        if(retbuf)
                free(retbuf);

        if(tbuf)
                free(tbuf);
        if(args)
                json_object_put(args);
                
        return ret;
}       


int main()
{
        sockfd = tr_opensock("127.0.0.1");
        char bindkey[32];
        xl_getsysinfo(sockfd, bindkey);
        printf("%s\n", bindkey);
}
