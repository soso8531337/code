/******************************************************************************
* Copyright (C) 2011 by IOVST
* 
* File: xl.c
*
* Date: 2012-07-06
*
* Author: hulong 
*
* Descriptor: download
*   
* Version: 0.1
*	
*
******************************************************************************/


#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
#include <assert.h>
#include <sys/stat.h> 
#include <fcntl.h>
#include <pthread.h>

#include "xl_util.h"
#include "common.h"
#include "tr_socket.h"
#include "json.h"



int xl_set_maxtask(int num)
{
        MCEMT_DBG("enter xl_set_maxtask\n");
        int ret = 0, n = 0;
        int sockf;
        char *retbuf, *str;
        struct json_object *args = NULL;
        int result;

        retbuf = (char*)calloc(1024, sizeof(char));
        str = (char*)calloc(128, sizeof(char));
        sprintf(str, "GET /setrunningtaskslimit?runningtaskscount=%d HTTP/1.1\r\n\r\n", num);

        sockf = xl_opensock("127.0.0.1");
	if(sockf < 0)
		return -1;
        
        if(send(sockf, str, strlen(str), 0) < 0){
                MCEMT_DBG("send failure\n");
                ret = -1;
                goto XL_ERR;
        }else{
                MCEMT_DBG("send success\n");
        }

        n = receive_data(sockf, retbuf, 1024, 60000);
        if(n == 0){
                ret = -1;
                goto XL_ERR;
        }
        char *tstr = NULL;
        tstr = strstr(retbuf, "result: "); 
              
        result = atoi(tstr);
        MCEMT_DBG("the result is %d\n", result);
        
        if(result==0)
                ret = 0;
        else 
                ret = -1;
XL_ERR:
        if(str)
                free(str);
        if(retbuf)
                free(retbuf);          
        if(args)
                json_object_put(args);
        close(sockf);
        return ret;
}
       


int xl_unbind()
{
        MCEMT_DBG("enter xl_unbind\n");
        int ret = 0, n = 0;
        int sockf;
        char *retbuf = NULL;
        int result;
        struct json_object *args = NULL, *obj = NULL;

        char str[]={
                "GET /unbind HTTP/1.1\r\n\r\n"           
                };
      
        sockf = xl_opensock("127.0.0.1");
	if(sockf < 0)
		return -1;
        
        retbuf = (char*)calloc(1024, sizeof(char));
        if(!retbuf){
                ret = -1;
                goto XL_ERR;
        }
    
        if(send(sockf,str, strlen(str), 0) < 0){
                MCEMT_DBG("send failure\n");
                ret = -1;
                goto XL_ERR;
        }else{
                MCEMT_DBG("send success\n");
        }

        n = receive_data(sockf, retbuf, 1024, 60000);
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
        
        args = json_tokener_parse(tstr);        
        obj = json_object_array_get_idx(args, 0);
        result = json_object_get_int(obj);
        
        if(result==0)
                ret = 0;
        else 
                ret = -1;
XL_ERR:
        if(retbuf)
                free(retbuf);
        if(args)
                json_object_put(args);
        if(obj)
                json_object_put(obj);

        close(sockf);
        return ret;        
}       
       


/*in:
        s is socket, dspeed is limit of down speed, uspeed is limit of up speed
  out:
  success:
        return 0;
  error:
        return -1;
*/

int xl_set_limit_speed(int dspeed, int uspeed)
{
        MCEMT_DBG("enter xl_set_limit\n");
        int ret = 0, n = 0;
        struct json_object *args = NULL;
        char *str, *retbuf;
        int sockf;
        int result;
                    
        retbuf = (char*)calloc(1024, sizeof(char));          
        str = (char*)calloc(128, sizeof(char));

        if(!retbuf||!str){
                ret = -1;
                return ret;
         }
                         
        sprintf(str, "GET /speedlimit?download_speed=%d&upload_speed=%d HTTP/1.1\r\n\r\n", dspeed, uspeed);
        
        sockf = xl_opensock("127.0.0.1");
	if(sockf < 0)
		return -1;
        if(send(sockf, str, strlen(str), 0) < 0){
                MCEMT_DBG("send failure\n");
                ret = -1;
                goto XL_ERR;
        }else{
                MCEMT_DBG("send success\n");
        }

        n = receive_data(sockf, retbuf, 1024, 60000);
        if(n == 0){
                ret = -1;
                goto XL_ERR;
        }
        char *tstr = NULL;
        tstr = strstr(retbuf, "result: "); 
        
       
        result = atoi(tstr);
        MCEMT_DBG("the result is %d\n", result);
        
        if(result==0)
                ret = 0;
        else 
                ret = -1;
XL_ERR:
        if(str)
                free(str);
        if(retbuf)
                free(retbuf);          
        if(args)
                json_object_put(args);
        close(sockf);
        return ret;
}



/*in: 
        s is socket,
 out:
 success:
        return 0;
 error:
        return -1;
*/
int xl_getsysinfo(char *bind_key, int *bind_ok, int *net_ok, int *license_ok, int *disk_ok)
{
        MCEMT_DBG("enter xl_getsysinfo\n");
        struct json_object *args = NULL, *obj = NULL;
        int n = 0, ret = 0;
        char *retbuf = NULL, *tbuf = NULL; 
        int sockf;

        char str[]={
                "GET /getsysinfo HTTP/1.1\r\n\r\n"              
                };

	sockf = xl_opensock("127.0.0.1");
    	if(sockf < 0) return -1;

        retbuf = (char*)calloc(1024, sizeof(char));
        tbuf = (char*)calloc(1024, sizeof(char));
        if(!retbuf||!tbuf){
                ret = -1;
                goto XL_ERR;
        }
                            		
        if(send(sockf, str, strlen(str), 0) < 0){
		printf(" sockfd: %d,  errno(%d), info: %s\n", sockf, errno, strerror(errno));
                ret = -1;
                goto XL_ERR;
        }else{
                printf("send success\n");
        }

        n = receive_data(sockf, retbuf, 1024, 60000);
	printf("%s\n", retbuf);
        if(n == 0){
                ret = -1;
		printf("%d\n", n);
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
        
        args = json_tokener_parse(tstr);

        obj = json_object_array_get_idx(args, 4);
        if(obj){
                strcpy(bind_key, json_object_get_string(obj));
        }
        obj = json_object_array_get_idx(args, 3);
        if(obj){
                *bind_ok = json_object_get_int(obj);
        }
        obj = json_object_array_get_idx(args, 2);
        if(obj){
                *license_ok = json_object_get_int(obj);
        }
        obj = json_object_array_get_idx(args, 1);
        if(obj){
                *net_ok = json_object_get_int(obj);
        }
        obj = json_object_array_get_idx(args, 5);
        if(obj){
                *disk_ok = json_object_get_int(obj);
        }
                        
XL_ERR:
        if(retbuf)
                free(retbuf);

        if(tbuf)
                free(tbuf);
        if(args)
                json_object_put(args);
        close(sockf);
                
        return ret;
}       

