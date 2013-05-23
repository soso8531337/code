#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
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
#include <zlib.h>

#include "tr_socket.h"
#include "tr_util.h"
#include "common.h"
#include "json.h"

extern char sses[256];
extern int sockfd;
extern int tr_init(int *sockfd);
extern pthread_mutex_t work_mutex;


int tr_send_post_getlimit(int s, int *num, int *down, int *up)
{
        MCEMT_DBG("enter tr_send_post_getlimit\n");
        char *str = "{\"method\":\"session-get\"}";
        char *tstr = NULL;
        struct json_object *args = NULL, *tmp_obj = NULL;
        char *retbuf = (char*)calloc(4*1024, sizeof(char));
        char *buf = (char*)calloc(1024, sizeof(char));
        char *tsbuf = (char*)calloc(4*1024, sizeof(char)); 
        char *uncompr = (char*)calloc(4*1024, sizeof(char));
        int n = 0, ret = 0;
        if(!retbuf||!buf||!tsbuf||!str||!uncompr){
                MCEMT_DBG("malloc err\n");
                ret = -1;
                goto TRERR;
        }
        int len = strlen(str);
        snprintf(buf, 1024,
                "POST /transmission/rpc HTTP/1.1\r\n"
                "Host: 127.0.0.1:/9091\r\n"
                "User-Agent: tr\r\n"
                "Accept: application/json, text/javascript, */*; q=0.01\r\n"
                "Accept-Language: zh-cn,zh;q=0.8,en-us;q=0.5,en;q=0.3\r\n"
                "Accept-Encoding: gzip, deflate\r\n"
                "Connection: keep-alive\r\n"
                "Content-Type: json; charset=UTF-8\r\n"
                "X-Transmission-Session-Id: %s\r\n"
                "X-Requested-With: XMLHttpRequest \r\n" 
                "Referer: http://127.0.0.1:9091/transmission/web/\r\n"
                "Pragma: no-cache\r\n"
                "Content-Length: %d\r\n"
                "Cache-Control: no-cache\r\n\r\n"        
                "%s", sses, len, str);
        MCEMT_DBG("%s\n", buf);

        pthread_mutex_lock(&work_mutex); 
        if(send(s, buf, strlen(buf), 0) < 0){
                MCEMT_DBG("send failure\n");
                ret = -1;
                goto TRERR;
        }else{
                MCEMT_DBG("send success\n");
        }       
        
        memset(retbuf, 0, sizeof(retbuf));
        n = receive_data(s, retbuf, 16*1024, 60000);
        pthread_mutex_unlock(&work_mutex);
        MCEMT_DBG("%s\n", retbuf);
        if(n == 0){
                ret = -1;
                goto TRERR;
        }
        char *l = strstr(retbuf, "Content-Length: ");
        if(l==NULL){
                goto TRERR;
        }        
        l+=16;
        n = atoi(l);
        
        /* Remove the HTML header */
        //printf("the retbuf is %d\n", strlen(retbuf)); 
        tstr = strstr(retbuf, "\r\n\r\n");
        if(tstr){
                tstr += 4;
                /* Delete the empty lines */
                while (*tstr == '\n') tstr++;
        }
         
        memcpy(tsbuf, tstr, n);
        tsbuf[n] = '\0';
        
        inf(tsbuf, n, uncompr, 16*1024);
        MCEMT_DBG("uncompr: %s\n", uncompr);
        
        args = json_tokener_parse(uncompr);
        if(args == NULL){
                MCEMT_DBG("parse error\n");
                ret = -1;
                goto TRERR;
        }              
        tmp_obj = json_object_object_get(args, "result");
        if(tmp_obj==NULL){
                ret = -1;
                goto TRERR;
        }
        
        *num = json_object_get_int(json_object_object_get(args, "download-queue-size"));
        *down = json_object_get_int(json_object_object_get(args, "speed-limit-down"));
        *up = json_object_get_int(json_object_object_get(args, "speed-limit-up"));
        
        TRERR:
        if(retbuf)
                free(retbuf);
        if(buf)
                free(buf); 
        if(tsbuf)
                free(tsbuf);
        if(uncompr)
                free(uncompr);
        if(args)
                json_object_put(args);
        
        MCEMT_DBG("the ret is %d\n", ret);
        return ret;     
        

}



int tr_send_post_setpath(int s, char *path)
{
        MCEMT_DBG("enter tr_send_post_setpath\n");
        char *str = (char*)calloc(128, sizeof(char));
        char *tstr = NULL;
        struct json_object *args = NULL, *tmp_obj = NULL;
        char *retbuf = (char*)calloc(4*1024, sizeof(char));
        char *buf = (char*)calloc(1024, sizeof(char));
        char *tsbuf = (char*)calloc(4*1024, sizeof(char)); 
        char *uncompr = (char*)calloc(4*1024, sizeof(char));
        int n = 0, ret = 0;
        if(!retbuf||!buf||!tsbuf||!str||!uncompr){
                MCEMT_DBG("malloc err\n");
                ret = -1;
                goto TRERR;
        }
        sprintf(str, "{\"method\":\"session-set\", \"arguments\":{\"download-dir\":\"%s\"}}", path);
        int len = strlen(str);
        snprintf(buf, 1024,
                "POST /transmission/rpc HTTP/1.1\r\n"
                "Host: 127.0.0.1:/9091\r\n"
                "User-Agent: tr\r\n"
                "Accept: application/json, text/javascript, */*; q=0.01\r\n"
                "Accept-Language: zh-cn,zh;q=0.8,en-us;q=0.5,en;q=0.3\r\n"
                "Accept-Encoding: gzip, deflate\r\n"
                "Connection: keep-alive\r\n"
                "Content-Type: json; charset=UTF-8\r\n"
                "X-Transmission-Session-Id: %s\r\n"
                "X-Requested-With: XMLHttpRequest \r\n" 
                "Referer: http://127.0.0.1:9091/transmission/web/\r\n"
                "Pragma: no-cache\r\n"
                "Content-Length: %d\r\n"
                "Cache-Control: no-cache\r\n\r\n"        
                "%s", sses, len, str);

        MCEMT_DBG("%s\n", buf);

        if(send(s, buf, strlen(buf), 0) < 0){
                MCEMT_DBG("send failure\n");
                ret = -1;
                goto TRERR;
        }else{
                MCEMT_DBG("send success\n");
        }       
        
        memset(retbuf, 0, sizeof(retbuf));
        n = receive_data(s, retbuf, 16*1024, 60000);
        MCEMT_DBG("%s\n", retbuf);
        if(n == 0){
                ret = -1;
                goto TRERR;
        }
        char *l = strstr(retbuf, "Content-Length: ");
        if(l==NULL){
                goto TRERR;
        }        
        l+=16;
        n = atoi(l);
        
        /* Remove the HTML header */
        //printf("the retbuf is %d\n", strlen(retbuf)); 
        tstr = strstr(retbuf, "\r\n\r\n");
        if(tstr){
                tstr += 4;
                /* Delete the empty lines */
                while (*tstr == '\n') tstr++;
        }
         
        memcpy(tsbuf, tstr, n);
        tsbuf[n] = '\0';
        
        inf(tsbuf, n, uncompr, 16*1024);
        MCEMT_DBG("uncompr: %s\n", uncompr);
        
        args = json_tokener_parse(uncompr);
        if(args == NULL){
                MCEMT_DBG("parse error\n");
                ret = -1;
                goto TRERR;
        }              
        tmp_obj = json_object_object_get(args, "result");
        if(tmp_obj==NULL){
                ret = -1;
                goto TRERR;
        }

        TRERR:
        if(retbuf)
                free(retbuf);
        if(buf)
                free(buf); 
        if(tsbuf)
                free(tsbuf);
        if(str)
                free(str);
        if(uncompr)
                free(uncompr);
        if(args)
                json_object_put(args);

        
        MCEMT_DBG("the ret is %d\n", ret);
        return ret;         
}


int tr_send_post_setdownlimit(int s, int down)
{
#if 0
        char *str =  (char*)calloc(128, sizeof(char)); 
        char *tstr = NULL;

        struct json_object *args = NULL, *tmp_obj = NULL;
        char *retbuf = (char*)calloc(4*1024, sizeof(char));
        char *buf = (char*)calloc(1024, sizeof(char));
        char *tsbuf = (char*)calloc(4*1024, sizeof(char)); 
        char *uncompr = (char*)calloc(4*1024, sizeof(char));
        char *tmpsess;
        int n = 0, ret = 0;
        if(!retbuf||!buf||!tsbuf||!str||!uncompr){
                MCEMT_DBG("malloc err\n");
                ret = -1;
                goto TRERR;
        }   

        sprintf(str, "{\"method\":\"session-set\", \"arguments\":{\"speed-limit-down\":%d, \"speed-limit-down-enabled\":true}}", down);
        int len = strlen(str);
        snprintf(buf, 1024,
                "POST /transmission/rpc HTTP/1.1\r\n"
                "Host: 127.0.0.1:/9091\r\n"
                "User-Agent: tr\r\n"
                "Accept: application/json, text/javascript, */*; q=0.01\r\n"
                "Accept-Language: zh-cn,zh;q=0.8,en-us;q=0.5,en;q=0.3\r\n"
                "Accept-Encoding: gzip, deflate\r\n"
                "Connection: keep-alive\r\n"
                "Content-Type: json; charset=UTF-8\r\n"
                "X-Transmission-Session-Id: %s\r\n"
                "X-Requested-With: XMLHttpRequest \r\n" 
                "Referer: http://127.0.0.1:9091/transmission/web/\r\n"
                "Pragma: no-cache\r\n"
                "Content-Length: %d\r\n"
                "Cache-Control: no-cache\r\n\r\n"        
                "%s", sses, len, str);

        MCEMT_DBG("%s\n", buf);

        if(send(s, buf, strlen(buf), 0) < 0){
                MCEMT_DBG("send failure\n");
                ret = -1;
                goto TRERR;
        }else{
                MCEMT_DBG("send success\n");
        }       
        
        memset(retbuf, 0, sizeof(retbuf));
        n = receive_data(s, retbuf, 16*1024, 60000);
        MCEMT_DBG("%s\n", retbuf);
        if(n == 0){
                ret = -1;
                goto TRERR;
        }
        char *l = strstr(retbuf, "Content-Length: ");
        if(l==NULL){
                goto TRERR;
        }        
        l+=16;
        n = atoi(l);
        
        /* Remove the HTML header */
        //printf("the retbuf is %d\n", strlen(retbuf)); 
        tstr = strstr(retbuf, "\r\n\r\n");
        if(tstr){
                tstr += 4;
                /* Delete the empty lines */
                while (*tstr == '\n') tstr++;
        }
         
        memcpy(tsbuf, tstr, n);
        tsbuf[n] = '\0';
        
        inf(tsbuf, n, uncompr, 16*1024);
        MCEMT_DBG("uncompr: %s\n", uncompr);
        
        args = json_tokener_parse(uncompr);
        if(args == NULL){
                MCEMT_DBG("parse error\n");
                ret = -1;
                goto TRERR;
        }              
        tmp_obj = json_object_object_get(args, "result");
        if(tmp_obj==NULL){
                ret = -1;
                goto TRERR;
        }
        const char *re = json_object_get_string(tmp_obj);
        MCEMT_DBG("result: %s\n", re);
        TRERR:
        if(retbuf)
                free(retbuf);
        if(buf)
                free(buf); 
        if(tsbuf)
                free(tsbuf);
        if(str)
                free(str);
        if(uncompr)
                free(uncompr);
        if(args)
                json_object_put(args);
      
        
        MCEMT_DBG("the ret is %d\n", ret);
#endif
        return 0;
}



int tr_send_post_setlimit(void* limitt)
{
#if 0
        MCEMT_DBG("enter tr_send_post_setlimit\n");
        char *tstr = NULL;
        struct json_object *args = NULL, *tmp_obj = NULL;
        char *retbuf = (char*)calloc(4*1024, sizeof(char));
        char *buf = (char*)calloc(1024, sizeof(char));
        char *str =  (char*)calloc(128, sizeof(char));
        char *tsbuf = (char*)calloc(4*1024, sizeof(char)); 
        char *uncompr = (char*)calloc(4*1024, sizeof(char));
        char *tmpsess;
        int n = 0, ret = 0;
        limit_info limit;
        limit = *(limit_info*)limitt;
        if(!retbuf||!buf||!tsbuf||!str||!uncompr){
                MCEMT_DBG("malloc err\n");
                ret = -1;
                goto TRERR;
        }
        sprintf(str, "{\"arguments\":{\"download-queue-size\":%d,\"speed-limit-down\":%d,\"speed-limit-down\":%d},\"method\":\"session-set\"}", limit.task_num, limit.uspeed, limit.dspeed);
        int len = strlen(str);
        snprintf(buf, 1024,
                "POST /transmission/rpc HTTP/1.1\r\n"
                "Host: 127.0.0.1:/9091\r\n"
                "User-Agent: tr\r\n"
                "Accept: */*\r\n"
                "Connection: keep-alive\r\n"
                "Content-Type: json; charset=UTF-8\r\n"
                "X-Transmission-Session-Id: %s\r\n"
                "Content-Length: %d\r\n"
                "Cache-Control: no-cacher\n"               
                "%s", sses, len, str);

        MCEMT_DBG("%s\n", buf);
        MCEMT_DBG("#############################\n");
        MCEMT_DBG("#############################\n");
        MCEMT_DBG("#############################\n");        
        MCEMT_DBG("#############################\n");
        MCEMT_DBG("#############################\n");
        pthread_mutex_lock(&work_mutex); 
        if(send(limit.s, buf, strlen(buf), 0) < 0){
                MCEMT_DBG("send failure\n");
                ret = -1;
                pthread_mutex_unlock(&work_mutex);         
                goto TRERR;
        }else{
                MCEMT_DBG("send success\n");
        }       
        n = receive_data(limit.s, retbuf, 4*1024, 3600);
        pthread_mutex_unlock(&work_mutex);
        MCEMT_DBG("%s\n", retbuf);
        if(n == 0){
                ret = -1;
                goto TRERR;
        }
        char *l = strstr(retbuf, "Content-Length: ");
        if(l==NULL){
                goto TRERR;
        }        
        l+=16;
        n = atoi(l);
        MCEMT_DBG("the n is %d\n", n);
        
        /* Remove the HTML header */
        //printf("the retbuf is %d\n", strlen(retbuf)); 
        tstr = strstr(retbuf, "\r\n\r\n");
        if(tstr){
                tstr += 4;
                /* Delete the empty lines */
                while (*tstr == '\n') tstr++;
        }
         
        memcpy(tsbuf, tstr, n);
        tsbuf[n] = '\0';
    //    inf(tsbuf, n, uncompr, 4*1024);
        
     //   MCEMT_DBG("uncompr: %s\n", uncompr);
        args = json_tokener_parse(tsbuf);
        if(args == NULL){
                MCEMT_DBG("parse error\n");
                ret = -1;
                goto TRERR;
        }              
        tmp_obj = json_object_object_get(args, "result");
        if(tmp_obj==NULL){
                ret = -1;
                goto TRERR;
        }
        const char *re = json_object_get_string(tmp_obj);
        MCEMT_DBG("result: %s\n", re);

        TRERR:
        if(retbuf)
                free(retbuf);
        if(buf)
                free(buf); 
        if(tsbuf)
                free(tsbuf);
        if(str)
                free(str);
        if(uncompr)
                free(uncompr);
        if(args)
                json_object_put(args);
        
        MCEMT_DBG("the ret is %d\n", ret);
#endif         
        return 0;
}
      
        
                

int tr_send_post_remove(int id)
{
        MCEMT_DBG("enter tr_send_post_remove\n");
        char *tstr = NULL;
        struct json_object *args = NULL, *tmp_obj = NULL;
        char *retbuf = (char*)calloc(4*1024, sizeof(char));
        char *buf = (char*)calloc(1024, sizeof(char));
        char *tsbuf = (char*)calloc(4*1024, sizeof(char)); 
        char *str =  (char*)calloc(128, sizeof(char)); 
        char *uncompr = (char*)calloc(4*1024, sizeof(char));
        int n = 0, ret = 0;
        if(!retbuf||!buf||!tsbuf||!str||!uncompr){
                MCEMT_DBG("malloc err\n");
                ret = -1;
                goto TRERR;
        } 
        sprintf(str, "{\"method\":\"torrent-remove\",\"arguments\":{\"ids\":[%d]}}", id);
        int len = strlen(str);
        snprintf(buf, 1024,
                "POST /transmission/rpc HTTP/1.1\r\n"
                "Host: 127.0.0.1:/9091\r\n"
                "User-Agent: tr\r\n"
                "Accept: application/json, text/javascript, */*; q=0.01\r\n"
                "Accept-Language: zh-cn,zh;q=0.8,en-us;q=0.5,en;q=0.3\r\n"
                "Accept-Encoding: gzip, deflate\r\n"
                "Connection: keep-alive\r\n"
                "Content-Type: json; charset=UTF-8\r\n"
                "X-Transmission-Session-Id: %s\r\n"
                "X-Requested-With: XMLHttpRequest \r\n" 
                "Referer: http://127.0.0.1:9091/transmission/web/\r\n"
                "Pragma: no-cache\r\n"
                "Content-Length: %d\r\n"
                "Cache-Control: no-cache\r\n\r\n"        
                "%s", sses, len, str);
        MCEMT_DBG("%s\n", buf);
        pthread_mutex_lock(&work_mutex); 
        if(send(sockfd, buf, strlen(buf), 0) < 0){
                MCEMT_DBG("send failure\n");
                ret = -1;
                pthread_mutex_unlock(&work_mutex); 
                goto TRERR;
        }else{
                MCEMT_DBG("send success\n");
        }       
        pthread_mutex_unlock(&work_mutex);         
        memset(retbuf, 0, sizeof(retbuf));
        n = receive_data(sockfd, retbuf, 16*1024, 600);
        MCEMT_DBG("%s\n", retbuf);
        if(n == 0){
                ret = -1;
                goto TRERR;
        }
        char *l = strstr(retbuf, "Content-Length: ");
        if(l==NULL){
                goto TRERR;
        }        
        l+=16;
        n = atoi(l);
        
        /* Remove the HTML header */
        //printf("the retbuf is %d\n", strlen(retbuf)); 
        tstr = strstr(retbuf, "\r\n\r\n");
        if(tstr){
                tstr += 4;
                /* Delete the empty lines */
                while (*tstr == '\n') tstr++;
        }
         
        memcpy(tsbuf, tstr, n);
        tsbuf[n] = '\0';
        
        inf(tsbuf, n, uncompr, 16*1024);
        MCEMT_DBG("uncompr: %s\n", uncompr);
        
        args = json_tokener_parse(uncompr);
        if(args == NULL){
                MCEMT_DBG("parse error\n");
                ret = -1;
                goto TRERR;
        }              
        tmp_obj = json_object_object_get(args, "result");
        if(tmp_obj==NULL){
                ret = -1;
                goto TRERR;
        }
        
        TRERR:
        if(retbuf)
                free(retbuf);
        if(buf)
                free(buf); 
        if(tsbuf)
                free(tsbuf);
        if(str)
                free(str);
        if(uncompr)
                free(uncompr);
        if(args)
                json_object_put(args);
        if(tmp_obj)
                json_object_put(tmp_obj);
        return 0;
 }   


/*success 0
  failure -1*/
int tr_send_post_start(int s, int id)
{
        
        MCEMT_DBG("enter tr_send_post_start\n");
        char *tstr = NULL;
        struct json_object *args = NULL, *tmp_obj = NULL;
        char *retbuf = (char*)calloc(4*1024, sizeof(char));
        char *buf = (char*)calloc(1024, sizeof(char));
        char *tsbuf = (char*)calloc(4*1024, sizeof(char)); 
        char *str =  (char*)calloc(128, sizeof(char)); 
        char *uncompr = (char*)calloc(4*1024, sizeof(char));
        int n = 0, ret = 0;
        if(!retbuf||!buf||!tsbuf||!str||!uncompr){
                MCEMT_DBG("malloc err\n");
                ret = -1;
                goto TRERR;
        } 
      
        sprintf(str, "{\"method\":\"torrent-start\",\"arguments\":{\"ids\":[%d]}}", id);
        int len = strlen(str);
        snprintf(buf, 1024,
                "POST /transmission/rpc HTTP/1.1\r\n"
                "Host: 127.0.0.1:/9091\r\n"
                "User-Agent: tr\r\n"
                "Accept: application/json, text/javascript, */*; q=0.01\r\n"
                "Accept-Language: zh-cn,zh;q=0.8,en-us;q=0.5,en;q=0.3\r\n"
                "Accept-Encoding: gzip, deflate\r\n"
                "Connection: keep-alive\r\n"
                "Content-Type: json; charset=UTF-8\r\n"
                "X-Transmission-Session-Id: %s\r\n"
                "X-Requested-With: XMLHttpRequest \r\n" 
                "Referer: http://127.0.0.1:9091/transmission/web/\r\n"
                "Pragma: no-cache\r\n"
                "Content-Length: %d\r\n"
                "Cache-Control: no-cache\r\n\r\n"        
                "%s", sses, len, str);

        MCEMT_DBG("%s\n", buf);
        pthread_mutex_lock(&work_mutex); 
        if(send(s, buf, strlen(buf), 0) < 0){
                MCEMT_DBG("send failure\n");
                ret = -1;
                pthread_mutex_unlock(&work_mutex); 
                goto TRERR;
        }else{
                MCEMT_DBG("send success\n");
        }       
                
        memset(retbuf, 0, sizeof(retbuf));
        n = receive_data(s, retbuf, 16*1024, 60000);
        pthread_mutex_unlock(&work_mutex);
        MCEMT_DBG("%s\n", retbuf);
        if(n == 0){
                ret = -1;
                MCEMT_DBG("ret = -1\n");
                goto TRERR;
        }
        
        char *l = strstr(retbuf, "Content-Length: ");
        MCEMT_DBG("START\n");
        if(l==NULL){
                goto TRERR;
        }        
        l+=16;
        n = atoi(l);
        
        /* Remove the HTML header */
        //printf("the retbuf is %d\n", strlen(retbuf)); 
        tstr = strstr(retbuf, "\r\n\r\n");
        if(tstr){
                tstr += 4;
                /* Delete the empty lines */
                while (*tstr == '\n') tstr++;
        }
        MCEMT_DBG("START\n")
        memcpy(tsbuf, tstr, n);
        tsbuf[n] = '\0';
        MCEMT_DBG("START\n");
        inf(tsbuf, n, uncompr, 16*1024);
        MCEMT_DBG("uncompr: %s\n", uncompr);
        
        args = json_tokener_parse(uncompr);
        if(args == NULL){
                MCEMT_DBG("parse error\n");
                ret = -1;
                goto TRERR;
        }              
        tmp_obj = json_object_object_get(args, "result");
        if(tmp_obj==NULL){
                ret = -1;
                goto TRERR;
        }
    
        TRERR:
        if(retbuf)
                free(retbuf);
        if(buf)
                free(buf); 
        if(tsbuf)
                free(tsbuf);
        if(str)
                free(str);
        if(uncompr)
                free(uncompr);
        if(args)
                json_object_put(args);
        
        MCEMT_DBG("the ret is %d\n", ret);  
        return ret;
} 
/*success 0
 fail     -1*/
int tr_send_post_stop(int s, int id)
{
        MCEMT_DBG("enter tr_send_post_stop\n");
        char *tstr = NULL;
        struct json_object *args = NULL, *tmp_obj = NULL;
        char *retbuf = (char*)calloc(4*1024, sizeof(char));
        char *buf = (char*)calloc(1024, sizeof(char));
        char *tsbuf = (char*)calloc(4*1024, sizeof(char)); 
        char *str =  (char*)calloc(128, sizeof(char)); 
        char *uncompr = (char*)calloc(4*1024, sizeof(char));
        int n = 0, ret = 0;
        if(!retbuf||!buf||!tsbuf||!str||!uncompr){
                MCEMT_DBG("malloc err\n");
                ret = -1;
                goto TRERR;
        } 
      
        sprintf(str, "{\"method\":\"torrent-stop\",\"arguments\":{\"ids\":[%d]}}", id);
        int len = strlen(str);
        snprintf(buf, 1024,
                "POST /transmission/rpc HTTP/1.1\r\n"
                "Host: 127.0.0.1:/9091\r\n"
                "User-Agent: tr\r\n"
                "Accept: application/json, text/javascript, */*; q=0.01\r\n"
                "Accept-Language: zh-cn,zh;q=0.8,en-us;q=0.5,en;q=0.3\r\n"
                "Accept-Encoding: gzip, deflate\r\n"
                "Connection: keep-alive\r\n"
                "Content-Type: json; charset=UTF-8\r\n"
                "X-Transmission-Session-Id: %s\r\n"
                "X-Requested-With: XMLHttpRequest \r\n" 
                "Referer: http://127.0.0.1:9091/transmission/web/\r\n"
                "Pragma: no-cache\r\n"
                "Content-Length: %d\r\n"
                "Cache-Control: no-cache\r\n\r\n"	 
                "%s", sses, len, str);

        MCEMT_DBG("%s\n", buf);
        pthread_mutex_lock(&work_mutex);
        if(send(s, buf, strlen(buf), 0) < 0){
                MCEMT_DBG("send failure\n");
		ret = -1;
                pthread_mutex_unlock(&work_mutex); 
                goto TRERR;
	}else{
               	MCEMT_DBG("send success\n");
        }       
                
        memset(retbuf, 0, sizeof(retbuf));
        n = receive_data(s, retbuf, 16*1024, 60000);
        pthread_mutex_unlock(&work_mutex);
        MCEMT_DBG("%s\n", retbuf);
        if(n == 0){
	        ret = -1;
                goto TRERR;
        }
        MCEMT_DBG("STOP\n");
        char *l = strstr(retbuf, "Content-Length: ");
        if(l==NULL){
                goto TRERR;
        }        
	l+=16;
	n = atoi(l);
        MCEMT_DBG("STOP\n");
	/* Remove the HTML header */
	//printf("the retbuf is %d\n", strlen(retbuf)); 
	tstr = strstr(retbuf, "\r\n\r\n");
	if(tstr){
		tstr += 4;
		/* Delete the empty lines */
		while (*tstr == '\n') tstr++;
	}

        MCEMT_DBG("STOP\n");
	memcpy(tsbuf, tstr, n);
        tsbuf[n] = '\0';
        MCEMT_DBG("STOP\n");
        inf(tsbuf, n, uncompr, 16*1024);
        MCEMT_DBG("uncompr: %s\n", uncompr);
        
        args = json_tokener_parse(uncompr);
        if(args == NULL){
                MCEMT_DBG("parse error\n");
                ret = -1;
                goto TRERR;
        }              
        tmp_obj = json_object_object_get(args, "result");
        if(tmp_obj==NULL){
                ret = -1;
                goto TRERR;
        }
        
TRERR:
        MCEMT_DBG("STOP\n");
        if(retbuf)
                free(retbuf);
        if(buf)
                free(buf); 
        if(tsbuf)
                free(tsbuf);
        if(str)
                free(str);
        if(uncompr)
                free(uncompr);
        if(args)
                json_object_put(args);
     
        MCEMT_DBG("the ret is %d\n", ret);  
        return ret;
} 


/*success 0
 fail     -1*/

int tr_send_post_get_session(int s)
{ 
        MCEMT_DBG("enter tr_send_post_get_session\n");
        char *str = "{\"method\":\"session-get\"}";
        char *tstr = NULL, *tmpsess;
        struct json_object *args = NULL, *tmp_obj = NULL;
        char *retbuf = (char*)calloc(16*1024, sizeof(char));
        char *buf = (char*)calloc(1024, sizeof(char));
        char *tsbuf = (char*)calloc(16*1024, sizeof(char));
        char *uncompr = (char*)calloc(16*1024, sizeof(char));
     
        int n = 0, ret = 0;
        if(!retbuf||!buf||!tsbuf||!uncompr){
                MCEMT_DBG("malloc err\n");
                ret = -1;
                goto TRERR;
        }      
        int len = strlen(str);
        snprintf(buf, 1024,
                "POST /transmission/rpc HTTP/1.1\r\n"
                "Host: 127.0.0.1:/9091\r\n"
                "User-Agent: tr\r\n"
                "Accept: application/json, text/javascript, */*; q=0.01\r\n"
                "Accept-Language: zh-cn,zh;q=0.8,en-us;q=0.5,en;q=0.3\r\n"
                "Accept-Encoding: gzip, deflate\r\n"
                "Connection: keep-alive\r\n"
                "Content-Type: json; charset=UTF-8\r\n"
                "X-Transmission-Session-Id: %s\r\n"
                "X-Requested-With: XMLHttpRequest \r\n" 
                "Referer: http://127.0.0.1:9091/transmission/web/\r\n"
                "Pragma: no-cache\r\n"
                "Content-Length: %d\r\n"
                "Cache-Control: no-cache\r\n\r\n"	 
                "%s", sses, len, str);

        MCEMT_DBG("%s\n", buf);
        pthread_mutex_lock(&work_mutex);
        if(send(s, buf, strlen(buf), 0) < 0){
                MCEMT_DBG("send failure\n");
		ret = -1;
                pthread_mutex_unlock(&work_mutex);        
                goto TRERR;
	}else{
               	MCEMT_DBG("send success\n");
      
        }       
                
        memset(retbuf, 0, sizeof(retbuf));
        n = receive_data(s, retbuf, 16*1024, 60000);
        pthread_mutex_unlock(&work_mutex);
        MCEMT_DBG("%s\n", retbuf);
        if(n == 0){
	        ret = -1;
                goto TRERR;
        }
        while((tmpsess=strstr(retbuf, "X-Transmission-Session-Id")) != NULL){

                        tmpsess += 27;
                        strncpy(sses, tmpsess, 48);
                        ret = -1;
                        goto TRERR;
        }       
	char *l = strstr(retbuf, "Content-Length: ");
        if(l==NULL){
                goto TRERR;
        }        
	l+=16;
	n = atoi(l);
	/* Remove the HTML header */
	//printf("the retbuf is %d\n", strlen(retbuf)); 
	tstr = strstr(retbuf, "\r\n\r\n");
	if(tstr){
		tstr += 4;
		/* Delete the empty lines */
		while (*tstr == '\n') tstr++;
	}
         
	memcpy(tsbuf, tstr, n);
        tsbuf[n] = '\0';

        inf(tsbuf, n, uncompr, 16*1024);
	MCEMT_DBG("uncompr: %s\n", uncompr);

        if (uncompr == NULL){
               MCEMT_DBG("uncompr is NULL\n");
               ret = -1;
               goto TRERR;
        }                       
        
                      
        args = json_tokener_parse(uncompr);
        if(args == NULL){
                MCEMT_DBG("parse error\n");
                ret = -1;
                goto TRERR;
        }              
        tmp_obj = json_object_object_get(args, "result");
        if(tmp_obj==NULL){
                ret = -1;
                goto TRERR;
        }
        
TRERR:
        if(retbuf)
                free(retbuf);
        if(buf)
                free(buf); 
        if(tsbuf)
                free(tsbuf);
        if(uncompr)
                free(uncompr);
        if(args)
                json_object_put(args);
        MCEMT_DBG("the ret is %d\n", ret);     
        return ret;
                     
}    


int tr_send_post_get_session_status(int s, status_info_t *t)
{  
        MCEMT_DBG("enter tr_send_post_get_session_status\n");
        char *str = "{\"method\":\"session-stats\"}";
        char *tstr = NULL, *tmpsess;
        struct json_object *args = NULL, *tmp_obj = NULL, *tmpp_obj = NULL;
        char *retbuf = (char*)calloc(16*1024, sizeof(char));
        char *buf = (char*)calloc(1024, sizeof(char));
        char *tsbuf = (char*)calloc(16*1024, sizeof(char));
        char *uncompr = (char*)calloc(16*1024, sizeof(char));

        int n = 0, ret = 0;
        if(!retbuf||!buf||!uncompr||!tsbuf){
                MCEMT_DBG("malloc err\n");
                ret = -1;
                goto TRERR;
        }      
        int len = strlen(str);
        snprintf(buf, 1024,
                "POST /transmission/rpc HTTP/1.1\r\n"
                "Host: 127.0.0.1:/9091\r\n"
                "User-Agent: tr\r\n"
                "Accept: application/json, text/javascript, */*; q=0.01\r\n"
                "Accept-Language: zh-cn,zh;q=0.8,en-us;q=0.5,en;q=0.3\r\n"
                "Accept-Encoding: gzip, deflate\r\n"
                "Connection: keep-alive\r\n"
                "Content-Type: json; charset=UTF-8\r\n"
                "X-Transmission-Session-Id: %s\r\n"
                "X-Requested-With: XMLHttpRequest \r\n" 
                "Referer: http://127.0.0.1:9091/transmission/web/\r\n"
                "Pragma: no-cache\r\n"
                "Content-Length: %d\r\n"
                "Cache-Control: no-cache\r\n\r\n"        
                "%s", sses, len, str);
        pthread_mutex_lock(&work_mutex); 
        if(send(s, buf, strlen(buf), 0) < 0){
                MCEMT_DBG("send failure\n");
                ret = -1;
                pthread_mutex_unlock(&work_mutex); 
                goto TRERR;
        }else{
                MCEMT_DBG("send success\n");

        }       

        memset(retbuf, 0, sizeof(retbuf));
        n = receive_data(s, retbuf, 16*1024, 60000);
        pthread_mutex_unlock(&work_mutex);
        MCEMT_DBG("%s\n", retbuf);
        if(n == 0){
	        ret = -1;
                goto TRERR;
        }
        while((tmpsess=strstr(retbuf, "X-Transmission-Session-Id")) != NULL){
                        tmpsess += 27;
                        strncpy(sses, tmpsess, 48);
                        ret = -1;
                        goto TRERR;
        }       
	char *l = strstr(retbuf, "Content-Length: ");
        if(l==NULL){
                ret = -1;
                goto TRERR;
        }        
	l+=16;
	n = atoi(l);
	/* Remove the HTML header */
	//printf("the retbuf is %d\n", strlen(retbuf)); 
	tstr = strstr(retbuf, "\r\n\r\n");
	if(tstr){
		tstr += 4;
		/* Delete the empty lines */
		while (*tstr == '\n') tstr++;
	}
         
	memcpy(tsbuf, tstr, n);
        tsbuf[n] = '\0';

        inf(tsbuf, n, uncompr, 16*1024);
	MCEMT_DBG("uncompr: %s\n", uncompr);

        if (uncompr == NULL){
               MCEMT_DBG("uncompr is NULL\n");
               ret = -1;
               goto TRERR;
        }                       
        
                      
        args = json_tokener_parse(uncompr);
        if(args == NULL){
                MCEMT_DBG("parse error\n");
                ret = -1;
                goto TRERR;
        }              
        tmp_obj = json_object_object_get(args, "result");
        if(tmp_obj==NULL){
                ret = -1;
                goto TRERR;
        }
        
        tmp_obj = json_object_object_get(args, "arguments");
        if(tmp_obj==NULL){
                ret = -1;
                goto TRERR;
        }
        
        tmpp_obj = json_object_object_get(tmp_obj, "downloadSpeed");
        if(tmpp_obj==NULL){
                ret = -1;
                goto TRERR;
        }
        t->downloadspeed = json_object_get_int(tmpp_obj);
        tmpp_obj = json_object_object_get(tmp_obj, "uploadSpeed");
        if(tmpp_obj==NULL){
                ret = -1;
                goto TRERR;
        }
        t->uploadspeed = json_object_get_int(tmpp_obj);
        
TRERR:
        if(retbuf)
                free(retbuf);
        if(buf)
                free(buf); 
        if(tsbuf)
                free(tsbuf);
        if(uncompr)
                free(uncompr);
        if(args)
                json_object_put(args);
        MCEMT_DBG("the ret is %d\n", ret); 
        return ret;
                          
}    

int tr_send_post_get_info(int s, task_info_t **t, int *tl)
{
        MCEMT_DBG("enter tr_send_post_get_session\n");
        char *str = "{\"method\":\"torrent-get\",\"arguments\":{\"fields\":[\"id\",\"name\",\"addedDate\",\"totalSize\",\"isFinished\",\"isStalled\",\"peersConnected\",\"peersGettingFromUs\",\"peersSendingToUs\",\"percentDone\",\"queuePosition\",\"rateDownload\",\"rateUpload\",\"recheckProgress\",\"seedRatioMode\",\"status\",\"downloadedEver\",\"uploadedEver\",\"uploadRatio\",\"secondsDownloading\",\"leftUntilDone\"]}}";
        char *tstr = NULL, *tmpsess;
        struct json_object *args, *tmp_obj, *tmpp_obj;
        char *retbuf = (char*)calloc(16*1024, sizeof(char));
        char *buf = (char*)calloc(1024, sizeof(char));
        char *tsbuf = (char*)calloc(16*1024, sizeof(char));
        char *uncompr = (char*)calloc(16*1024, sizeof(char));
        int ret = 0, n = 0;
       
        if(!retbuf||!buf||!tsbuf||!uncompr){
                MCEMT_DBG("malloc err\n");
                ret = -1;
                goto TRERR;
        }     
             
        int len = strlen(str);
        snprintf(buf, 1024,
                "POST /transmission/rpc HTTP/1.1\r\n"
                "Host: 127.0.0.1:/9091\r\n"
                "User-Agent: tr\r\n"
                "Accept: application/json, text/javascript, */*; q=0.01\r\n"
                "Accept-Language: zh-cn,zh;q=0.8,en-us;q=0.5,en;q=0.3\r\n"
                "Accept-Encoding: gzip, deflate\r\n"
                "Connection: keep-alive\r\n"
                "Content-Type: json; charset=UTF-8\r\n"
                "X-Transmission-Session-Id: %s\r\n"
                "X-Requested-With: XMLHttpRequest \r\n" 
                "Referer: http://127.0.0.1:9091/transmission/web/\r\n"
                "Pragma: no-cache\r\n"
                "Content-Length: %d\r\n"
                "Cache-Control: no-cache\r\n\r\n"	 
                "%s", sses, len, str);

        MCEMT_DBG("%s\n", buf);
        pthread_mutex_lock(&work_mutex);
        if(send(s, buf, strlen(buf), 0) < 0){
                MCEMT_DBG("send failure\n");
                tr_init(&sockfd);
		ret = -1;
                pthread_mutex_unlock(&work_mutex);
                goto TRERR;
	}else{
               	MCEMT_DBG("send success\n");
      
        }       
                
        memset(retbuf, 0, sizeof(retbuf));
        n = receive_data(s, retbuf, 16*1024, 60000);
        pthread_mutex_unlock(&work_mutex);
        MCEMT_DBG("%s\n", retbuf);
        if (n == 0){
	        ret = -1;
                goto TRERR;
        }       
        while((tmpsess=strstr(retbuf, "X-Transmission-Session-Id")) != NULL){
                tmpsess += 27;
                strncpy(sses, tmpsess, 48);
                ret = -1;
                goto TRERR;
        }       
	char *l = strstr(retbuf, "Content-Length: ");
	l+=16;
	n = atoi(l);
	/* Remove the HTML header */
	//printf("the retbuf is %d\n", strlen(retbuf)); 
	tstr = strstr(retbuf, "\r\n\r\n");
	if(tstr){
		tstr += 4;
		/* Delete the empty lines */
		while (*tstr == '\n') 
                        tstr++;
	}
        
       	memcpy(tsbuf, tstr, n);
        tsbuf[n] = '\0';

        inf(tsbuf, n, uncompr, 16*1024);
	MCEMT_DBG("uncompr: %s\n", uncompr);

        if (uncompr == NULL){
               MCEMT_DBG("uncompr is NULL\n");
               ret = -1;
               goto TRERR;
        }                       
        args = json_tokener_parse(uncompr);
        if(args == NULL){
                MCEMT_DBG("parse error\n");
                ret = -1;
                goto TRERR;
        }              
        tmp_obj = json_object_object_get(args, "result");
        if(tmp_obj == NULL){
                ret = -1;
                goto TRERR;
        }
      
        tmp_obj = json_object_object_get(json_object_object_get(args, "arguments"), "torrents");
  
        if(tmp_obj == NULL){
                ret = -1;
                MCEMT_DBG("ret = -1\n");
                goto TRERR;
        }
        *tl  = json_object_array_length(tmp_obj);
        *t = (task_info_t*)calloc(*tl, sizeof(task_info_t));

        if(*t == NULL){
                ret = -1;
                MCEMT_DBG("ret = -1\n");
                goto TRERR;
        }
        int i = 0;
        for(i = 0; i < *tl; i++) {
	        tmpp_obj = json_object_array_get_idx(tmp_obj, i);
		MCEMT_DBG("json_object_array_get_idx(tmp_obj, i);\n");
		if(tmpp_obj != NULL){
			(*t+i)->start_time = json_object_get_int(json_object_object_get(tmpp_obj,"addedDate"));
                        (*t+i)->finished_time = json_object_get_int(json_object_object_get(tmpp_obj,"doneDate"));
			(*t+i)->task_id = json_object_get_int(json_object_object_get(tmpp_obj,"id"));	
                        (*t+i)->task_state = json_object_get_int(json_object_object_get(tmpp_obj,"status"));
                        (*t+i)->file_size = json_object_get_int64(json_object_object_get(tmpp_obj,"totalSize"));
                        (*t+i)->download_data_size = json_object_get_int(json_object_object_get(tmpp_obj,"downloadedEver"));
                        (*t+i)->file_name = strdup(json_object_get_string(json_object_object_get(tmpp_obj,"name")));
                        (*t+i)->connecting_pipe_num = json_object_get_int(json_object_object_get(tmpp_obj,"peersConnected"));
                        (*t+i)->downloading_pipe_num = json_object_get_int(json_object_object_get(tmpp_obj,"peersSendingToUs"));  
                        (*t+i)->failed_code = json_object_get_int(json_object_object_get(tmpp_obj,"error"));
                        (*t+i)->left_data_size = json_object_get_int(json_object_object_get(tmpp_obj,"leftUntilDone"));
                        (*t+i)->dl_speed = json_object_get_double(json_object_object_get(tmpp_obj,"rateDownload"));
                        (*t+i)->utime = json_object_get_int(json_object_object_get(tmpp_obj,"secondsDownloading"));
                        (*t+i)->isFinished = json_object_get_boolean(json_object_object_get(tmpp_obj,"isFinished"));
                        (*t+i)->isStalled =  json_object_get_boolean(json_object_object_get(tmpp_obj,"isStalled"));
                        (*t+i)->status =  json_object_get_int(json_object_object_get(tmpp_obj,"status"));
                        (*t+i)->percentDone = json_object_get_double(json_object_object_get(tmpp_obj,"percentDone"));
          
                   
                } else 
			break;                  
        }
TRERR:
        if(retbuf)
                free(retbuf);
        if(buf)
                free(buf); 
        if(tsbuf)
                free(tsbuf);
        if(uncompr)
                free(uncompr);
        if(args)
                json_object_put(args);
        MCEMT_DBG("the ret is %d\n", ret);
        return ret;
          
}

int tr_send_post_get_info_re(int s, task_info_t **t, int *tl)
{
        MCEMT_DBG("enter tr_send_post_get_session\n");
        char *str = "{\"method\":\"torrent-get\",\"arguments\":{\"fields\":[\"id\",\"name\",\"addedDate\",\"totalSize\",\"isFinished\",\"isStalled\",\"peersConnected\",\"peersGettingFromUs\",\"peersSendingToUs\",\"percentDone\",\"queuePosition\",\"rateDownload\",\"rateUpload\",\"recheckProgress\",\"seedRatioMode\",\"status\",\"downloadedEver\",\"uploadedEver\",\"uploadRatio\"]}}";
        char *tstr = NULL;
        struct json_object *args = NULL, *tmp_obj = NULL, *tmpp_obj = NULL;
        char *retbuf = (char*)calloc(16*1024, sizeof(char));
        char *buf = (char*)calloc(1024, sizeof(char));
        char *tsbuf = (char*)calloc(16*1024, sizeof(char));
        char *uncompr = (char*)calloc(16*1024, sizeof(char));
        int ret = 0, n = 0;
       
        if(!retbuf||!buf||!tsbuf||!uncompr){
                MCEMT_DBG("malloc err\n");
                ret = -1;
                goto TRERR;
        }     
             
        int len = strlen(str);
        snprintf(buf, 1024,
                "POST /transmission/rpc HTTP/1.1\r\n"
                "Host: 127.0.0.1:/9091\r\n"
                "User-Agent: tr\r\n"
                "Accept: application/json, text/javascript, */*; q=0.01\r\n"
                "Accept-Language: zh-cn,zh;q=0.8,en-us;q=0.5,en;q=0.3\r\n"
                "Accept-Encoding: gzip, deflate\r\n"
                "Connection: keep-alive\r\n"
                "Content-Type: json; charset=UTF-8\r\n"
                "X-Transmission-Session-Id: %s\r\n"
                "X-Requested-With: XMLHttpRequest \r\n" 
                "Referer: http://127.0.0.1:9091/transmission/web/\r\n"
                "Pragma: no-cache\r\n"
                "Content-Length: %d\r\n"
                "Cache-Control: no-cache\r\n\r\n"	 
                "%s", sses, len, str);

        MCEMT_DBG("%s\n", buf);
        pthread_mutex_lock(&work_mutex);
        if(send(s, buf, strlen(buf), 0) < 0){
                MCEMT_DBG("send failure\n");
		ret = -1;
                pthread_mutex_unlock(&work_mutex);
                goto TRERR;
	}else{
               	MCEMT_DBG("send success\n");
      
        }       
                
        memset(retbuf, 0, sizeof(retbuf));
        n = receive_data(s, retbuf, 16*1024, 60000);
        pthread_mutex_unlock(&work_mutex);
        MCEMT_DBG("%s\n", retbuf);
        if (n == 0){
	        ret = -1;
                goto TRERR;
        }       
	char *l = strstr(retbuf, "Content-Length: ");
	l+=16;
	n = atoi(l);
	/* Remove the HTML header */
	//printf("the retbuf is %d\n", strlen(retbuf)); 
	tstr = strstr(retbuf, "\r\n\r\n");
	if(tstr){
		tstr += 4;
		/* Delete the empty lines */
		while (*tstr == '\n') tstr++;
	}
        
       	memcpy(tsbuf, tstr, n);
        tsbuf[n] = '\0';

        inf(tsbuf, n, uncompr, 16*1024);
	MCEMT_DBG("uncompr: %s\n", uncompr);

        if (uncompr == NULL){
               MCEMT_DBG("uncompr is NULL\n");
               ret = -1;
               goto TRERR;
        }                       
        args = json_tokener_parse(uncompr);
        if(args == NULL){
                MCEMT_DBG("parse error\n");
                ret = -1;
                goto TRERR;
        }              
        tmp_obj = json_object_object_get(args, "result");
        if(tmp_obj == NULL){
                ret = -1;
                goto TRERR;
        }

        tmp_obj = json_object_object_get(args, "arguments");
        if(tmp_obj == NULL){
              ret = -1;
              MCEMT_DBG("get arguments err\n");
              goto TRERR;
        }
        tmp_obj = json_object_object_get(tmp_obj, "torrents");
        if(tmp_obj == NULL){
                ret = -1;
                MCEMT_DBG("get torrents err\n");
                goto TRERR;
        }       
    
        *tl  = json_object_array_length(tmp_obj);
        *t = (task_info_t*)calloc(*tl, sizeof(task_info_t));
        if(*t == NULL){
                ret = -1;
                goto TRERR;
        }
 
        int i = 0;
        for(i = 0; i < *tl; i++) {
              
	        tmpp_obj = json_object_array_get_idx(tmp_obj, i);
                
		MCEMT_DBG("json_object_array_get_idx(args, i);\n");
		if (tmpp_obj != NULL) {
			(*t+i)->start_time = json_object_get_int(json_object_object_get(tmpp_obj,"addedDate"));
                        (*t+i)->finished_time = json_object_get_int(json_object_object_get(tmpp_obj,"doneDate"));
			(*t+i)->task_id = json_object_get_int(json_object_object_get(tmpp_obj,"id"));	
                        (*t+i)->task_state = json_object_get_int(json_object_object_get(tmpp_obj,"status"));
			(*t+i)->file_size = json_object_get_double(json_object_object_get(tmpp_obj,"totalSize"));
			(*t+i)->download_data_size = json_object_get_double(json_object_object_get(tmpp_obj,"downloadedEver"));
                        (*t+i)->file_name = strdup(json_object_get_string(json_object_object_get(tmpp_obj,"name")));
                        (*t+i)->connecting_pipe_num = json_object_get_int(json_object_object_get(tmpp_obj,"peersConnected"));
                        (*t+i)->downloading_pipe_num = json_object_get_int(json_object_object_get(tmpp_obj,"peersSendingToUs"));  
                        (*t+i)->failed_code = json_object_get_int(json_object_object_get(tmpp_obj,"error"));
                        (*t+i)->left_data_size = json_object_get_double(json_object_object_get(tmpp_obj,"leftUntilDone"));
                        (*t+i)->dl_speed = json_object_get_double(json_object_object_get(tmpp_obj,"rateDownload"));
                        (*t+i)->utime = json_object_get_int(json_object_object_get(tmpp_obj,"secondsDownloading"));
                   
                }else{
                        break;
                }     
        }
             
TRERR:
        if(retbuf)
                free(retbuf);
        if(buf)
                free(buf);
        if(tsbuf)
                free(tsbuf);
        if(uncompr)
                free(uncompr);
        if(args)
                json_object_put(args);

        MCEMT_DBG("the ret is %d\n", ret);  
        return ret;
       
}


int tr_send_post_get_info_num(int s, task_info_t **t, int *tl, int *task)
{
        MCEMT_DBG("enter tr_send_post_get_session\n");
        char *tstr = NULL;
        struct json_object *args = NULL, *tmp_obj = NULL, *tmpp_obj = NULL;
        char *retbuf = (char*)calloc(16*1024, sizeof(char));
        char *buf = (char*)calloc(1024, sizeof(char));
        char *tsbuf = (char*)calloc(16*1024, sizeof(char));
        char *uncompr = (char*)calloc(16*1024, sizeof(char));
        char *str = (char*)calloc(1024, sizeof(char));
        char tmps[16];
        int ret = 0, n = 0;
        int i=1;
       
        if(!retbuf||!buf||!tsbuf||!uncompr||!str){
                MCEMT_DBG("malloc err\n");
                ret = -1;
                goto TRERR;
        }     
        sprintf(str, "%s", "{\"method\":\"torrent-get\",\"arguments\":{\"fields\":[\"id\",\"name\",\"addedDate\",\"totalSize\",\"isFinished\",\"isStalled\",\"peersConnected\",\"peersGettingFromUs\",\"peersSendingToUs\",\"percentDone\",\"queuePosition\",\"rateDownload\",\"rateUpload\",\"recheckProgress\",\"seedRatioMode\",\"status\",\"downloadedEver\",\"uploadedEver\",\"uploadRatio\"],\"ids\":[");
        while(task[i] != -1){
                MCEMT_DBG("while\n")
                sprintf(tmps, ",%d", task[i]);
                strcat(str, tmps);
                i++;
                }
        strcat(str, "]}}");
        int len = strlen(str);
        snprintf(buf, 1024,
                "POST /transmission/rpc HTTP/1.1\r\n"
                "Host: 127.0.0.1:/9091\r\n"
                "User-Agent: tr\r\n"
                "Accept: application/json, text/javascript, */*; q=0.01\r\n"
                "Accept-Language: zh-cn,zh;q=0.8,en-us;q=0.5,en;q=0.3\r\n"
                "Accept-Encoding: gzip, deflate\r\n"
                "Connection: keep-alive\r\n"
                "Content-Type: json; charset=UTF-8\r\n"
                "X-Transmission-Session-Id: %s\r\n"
                "X-Requested-With: XMLHttpRequest \r\n" 
                "Referer: http://127.0.0.1:9091/transmission/web/\r\n"
                "Pragma: no-cache\r\n"
                "Content-Length: %d\r\n"
                "Cache-Control: no-cache\r\n\r\n"	 
                "%s", sses, len, str);

        MCEMT_DBG("%s\n", buf);
        pthread_mutex_lock(&work_mutex);
        if(send(s, buf, strlen(buf), 0) < 0){
                MCEMT_DBG("send failure\n");
		ret = -1;
                pthread_mutex_unlock(&work_mutex);
                goto TRERR;
	}else{
               	MCEMT_DBG("send success\n");
        }       
                
        memset(retbuf, 0, sizeof(retbuf));
        n = receive_data(s, retbuf, 16*1024, 60000);
        pthread_mutex_unlock(&work_mutex);
        MCEMT_DBG("%s\n", retbuf);
        if (n == 0){
	        ret = -1;
                goto TRERR;
        }       
	char *l = strstr(retbuf, "Content-Length: ");
	l+=16;
	n = atoi(l);
	/* Remove the HTML header */
	//printf("the retbuf is %d\n", strlen(retbuf)); 
	tstr = strstr(retbuf, "\r\n\r\n");
	if(tstr){
		tstr += 4;
		/* Delete the empty lines */
		while (*tstr == '\n') 
                        tstr++;
	}
        
       	memcpy(tsbuf, tstr, n);
        tsbuf[n] = '\0';

        inf(tsbuf, n, uncompr, 16*1024);
	MCEMT_DBG("uncompr: %s\n", uncompr);

        if (uncompr == NULL){
               MCEMT_DBG("uncompr is NULL\n");
               ret = -1;
               goto TRERR;
        }                       
        args = json_tokener_parse(uncompr);
        if(args == NULL){
                MCEMT_DBG("parse error\n");
                ret = -1;
                goto TRERR;
        }              
        tmp_obj = json_object_object_get(args, "result");
        if(tmp_obj == NULL){
                ret = -1;
                goto TRERR;
        }
       
        tmp_obj = json_object_object_get(json_object_object_get(args, "arguments"), "torrents");
        if(tmp_obj == NULL){
                ret = -1;
                goto TRERR;
        }
        *tl  = json_object_array_length(tmp_obj);
        *t = (task_info_t*)calloc(*tl, sizeof(task_info_t));
        if(*t == NULL){
                ret = -1;
                goto TRERR;
        }
        for(i = 0; i < *tl; i++) {
	        tmpp_obj = json_object_array_get_idx(args, i);
		MCEMT_DBG("json_object_array_get_idx(args, i);\n");
		if (tmpp_obj != NULL) {
			(*t+i)->start_time = json_object_get_int(json_object_object_get(tmpp_obj,"addedDate"));
                        (*t+i)->finished_time = json_object_get_int(json_object_object_get(tmpp_obj,"doneDate"));
			(*t+i)->task_id = json_object_get_int(json_object_object_get(tmpp_obj,"id"));	
                        (*t+i)->task_state = json_object_get_int(json_object_object_get(tmpp_obj,"status"));
			(*t+i)->file_size = json_object_get_double(json_object_object_get(tmpp_obj,"totalSize"));
			(*t+i)->download_data_size = json_object_get_double(json_object_object_get(tmpp_obj,"downloadedEver"));
                        (*t+i)->file_name = strdup(json_object_get_string(json_object_object_get(tmpp_obj,"name")));
                        (*t+i)->connecting_pipe_num = json_object_get_int(json_object_object_get(tmpp_obj,"peersConnected"));
                        (*t+i)->downloading_pipe_num = json_object_get_int(json_object_object_get(tmpp_obj,"peersSendingToUs"));  
                        (*t+i)->failed_code = json_object_get_int(json_object_object_get(tmpp_obj,"error"));
                        (*t+i)->left_data_size = json_object_get_double(json_object_object_get(tmpp_obj,"leftUntilDone"));
                        (*t+i)->dl_speed = json_object_get_double(json_object_object_get(tmpp_obj,"rateDownload"));
                        (*t+i)->utime = json_object_get_int(json_object_object_get(tmpp_obj,"secondsDownloading"));
                        (*t+i)->isFinished = json_object_get_boolean(json_object_object_get(tmpp_obj,"isFinished"));
                        (*t+i)->isStalled =  json_object_get_boolean(json_object_object_get(tmpp_obj,"isStalled"));
                   
                } else 
			goto TRERR;                  
        }
TRERR:
        if(retbuf)
                free(retbuf);
        if(buf)
                free(buf); 
        if(tsbuf)
                free(tsbuf);
        if(uncompr)
                free(uncompr);
        if(args)
                json_object_put(args);
        MCEMT_DBG("the ret is %d\n", ret);   
        return ret;
}       


/*success 0
  fail    -1
 */ 
int tr_send_post_add(int s, const char *file_path)
{
        MCEMT_DBG("enter tr_send_post_add\n");
        int ret = 0, n = 0;
        char *bufh=NULL, *bufm=NULL, *buft=NULL, *rdbuf=NULL, *tstr=NULL, *retbuf=NULL, *tsbuf=NULL;
        struct json_object *args=NULL, *tmp_obj=NULL;
        bufh = (char*)calloc(1024, sizeof(char));
        bufm = (char*)calloc(1024, sizeof(char));
        buft = (char*)calloc(1024, sizeof(char));
        rdbuf = (char*)calloc(500*1024, sizeof(char));

        if((bufh==NULL)||(bufm==NULL)||(buft==NULL)||(rdbuf==NULL)){
                MCEMT_DBG("malloc err\n");
                ret = -1;
                goto TRERR;
        }      
        
        int rc = 0, fd = 0;
        fd = open(file_path, O_RDONLY);
        if(fd < 0){
                MCEMT_DBG("open file error\n");
                ret = -1;
                goto TRERR;
        }        
        rc = read(fd, rdbuf, 500*1024); 
        close(fd);

        snprintf(bufm, 1024,
                "-----------------------------265001916915724\r\n"
                "Content-Disposition: form-data; name=\"torrent_files[]\"\r\n"
                "filename=\"%s\"\r\n"
                "Content-Type: application/octet-stream\r\n\r\n"
		, file_path);
        snprintf(buft, 1024,
                "-----------------------------265001916915724\r\n"
                "Content-Disposition: form-data; name=\"X-Transmission-Session-Id\"\r\n\r\n"
                "%s\r\n"
                "-----------------------------265001916915724--\r\n"
                , sses);
        MCEMT_DBG("%s\n", buft);
        
        int size = strlen(bufm)+strlen(buft)+rc;
        snprintf(bufh, 1024, 
                "POST /transmission/upload?paused=false HTTP/1.1\r\n"
                "Host: 192.168.1.40:9091\r\n"
                "User-Agent: tr\r\n"
                "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
                "Accept-Language: zh-cn,zh;q=0.8,en-us;q=0.5,en;q=0.3\r\n"
                "Accept-Encoding: gzip, deflate\r\n"
                "Connection: Keep-Alive\r\n"
                "Referer: http://192.168.1.40:9091/transmission/web/\r\n"
                "Content-Type: multipart/form-data; boundary=---------------------------265001916915724\r\n"
                "Content-Length: %d\r\n\r\n"       
                , size);
        MCEMT_DBG("the socket is %d\n", s);
        MCEMT_DBG("the content-length is %d\n", size);
        pthread_mutex_lock(&work_mutex);
	if((send(s, bufh, strlen(bufh), 0) < 0)||(send(s, bufm, strlen(bufm), 0) < 0)||(send(s, rdbuf, rc, 0) < 0)||(send(s,buft, strlen(buft), 0)< 0)){
        	MCEMT_DBG("send failure\n");
		ret = -1;
                pthread_mutex_unlock(&work_mutex);
                goto TRERR;
	}else{
               	MCEMT_DBG("send success\n");
      
        }
        retbuf = (char*)calloc(16*1024, sizeof(char));
        tsbuf = (char*)calloc(16*1024, sizeof(char));
        if((retbuf==NULL)||(tsbuf==NULL)){
                MCEMT_DBG("malloc failure\n");
                ret = -1;
                pthread_mutex_unlock(&work_mutex);
                goto TRERR;
        }       
        
        n = receive_data(s, retbuf, 16*1024, 60000);
        pthread_mutex_unlock(&work_mutex);
        if (n == 0){
	        MCEMT_DBG("receive_data failure\n");
                ret = -1;
                goto TRERR;
        }
   
	MCEMT_DBG("%s\n", retbuf);
	MCEMT_DBG("the n is %d\n", n);
 
	char *l = strstr(retbuf, "Content-Length: ");
	l+=16;
	n = atoi(l);
	/* Remove the HTML header */
	//printf("the retbuf is %d\n", strlen(retbuf)); 
	tstr = strstr(retbuf, "\r\n\r\n");
	if(tstr){
		tstr += 4;
		/* Delete the empty lines */
		while (*tstr == '\n') tstr++;
	}
	memcpy(tsbuf, tstr, n);
        tsbuf[n] = '\0';
                      
        args = json_tokener_parse(tsbuf);
        if(args == NULL){
                MCEMT_DBG("parse error\n");
                ret = -1;
                goto TRERR;
        }              
        tmp_obj = json_object_object_get(args, "msg");
        if(tmp_obj == NULL){
                MCEMT_DBG("get json obj err\n");
                ret = -1;
                goto TRERR;
        }     
TRERR:
        if(bufh)
                free(bufh);
        if(bufm)
                free(bufm);
        if(buft)
                free(buft);
        if(rdbuf)
                free(rdbuf);
        if(retbuf)
                free(retbuf);
        if(tsbuf)
                free(tsbuf);
        if(args)
                json_object_put(args);
        if(tmp_obj)
                json_object_put(tmp_obj);
        MCEMT_DBG("the ret is %d\n", ret);
        return ret;        
       
}

