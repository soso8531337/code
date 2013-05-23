/******************************************************************************
* Copyright (C) 2011 by IOVST
* 
* File: xl.c
*
* Date: 2012-07-06
*
* Author: hulong 
*
* Descriptor: xl
*   
* Version: 0.1
*	
*
******************************************************************************/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>


#include "xml.h"
#include "xl.h"
#include "tr_util.h"
#include "json.h"
#include "tr_util.h"
#include "tr_socket.h"

extern char sses[256];
extern int sockfd;

int tr_init(int *sockfd)
{
        int n = 0, ret = 0;
        char *str = "{\"method\":\"session-get\"}";
        char *sse = NULL;
        char *retbuf = (char*)calloc(16*1024, sizeof(char));
        char *buf = (char*)calloc(1024, sizeof(char));

        if((retbuf==NULL)||(buf==NULL)){
                MCEMT_DBG("malloc failure\n");
                ret = -1;
                goto TRERR;
        }
        *sockfd = tr_opensock("127.0.0.1");
        MCEMT_DBG("the sockfd is %d\n", *sockfd);
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
                "X-Requested-With: XMLHttpRequest\r\n"
                "Referer: http://127.0.0.1:9091/transmission/web/\r\n"
                "Pragma: no-cache\r\n"
                "Content-Length: %d\r\n"
                "Cache-Control: no-cache\r\n\r\n"	 
                "%s", sses, len, str);
                MCEMT_DBG("%s\n", buf);
                MCEMT_DBG("buflen is %d\n", strlen(buf));
                if(send(*sockfd, buf, strlen(buf), 0)<0){
                	MCEMT_DBG("send failure\n");       
        		ret = -1;
                        goto TRERR;
        	}else{
                       	MCEMT_DBG("send success\n" );                        
                }
       
                memset(retbuf, 0, sizeof(retbuf));
                n = receive_data(*sockfd, retbuf, 16*1024, 60000);
                if (n == 0){
                        MCEMT_DBG("receive_data's size is 0\n");
                        ret = -1;
        	        goto TRERR;
                }
                MCEMT_DBG("%s\n", retbuf);
                if((sse=strstr(retbuf, "X-Transmission-Session-Id")) != NULL){
                        sse += 27;
                        strncpy(sses, sse, 48);
                        MCEMT_DBG("get session-id success\n");
                }       
TRERR:
        if(retbuf)
                free(retbuf);
        if(buf)
                free(buf);
        return ret;

}                      



int pro_dld_tr_handler(cgi_t *p, cgi_protocol_t *d, sys_outbuf_t *buf)
{
        MCEMT_DBG("pro_dld_tr_handler start\n");
	int status, num, co = 0, down, up;
        int ret_val;
	char str[128];

        cgi_rec_t *rec;
	rec = p->cgirec;

      	if((d->flag&PROTOCOL_FUCTION) == 0){
		/*get msg*/
		xml_add_elem(XML_LABEL, "support", "1", buf);
                status = 1;
		sprintf(str, "%d", status);
		xml_add_elem(XML_LABEL, "status", str, buf);
		if(status){
                        if(tr_send_post_getlimit(sockfd, &num, &down, &up))
                                MCEMT_DBG("get limint failure\n");
                                                
			sprintf(str, "%d", num);
			xml_add_elem(XML_LABEL, "num", str, buf);

			sprintf(str, "%d", co);
			xml_add_elem(XML_LABEL, "connect", str, buf);

			sprintf(str, "%d", up);
			xml_add_elem(XML_LABEL, "uprate", str, buf);

			sprintf(str, "%d", down);
			xml_add_elem(XML_LABEL, "downrate", str, buf);
		}

	}else{
		/*set msg*/
		        char *name;
		        int icount=0, task_num=0, task_co=0, uspeed=0, dspeed=0;
		        if ((name = rec->get_val(rec, PRO_DLD_XL_STATUS)) != NULL){
			        icount = atoi(name);
			/*1->start 2->stop*/
			        if(icount == PRO_SERV_ACTION_START){
			                ret_val = tr_init(&sockfd);
		                        if(ret_val == 0)
			                        MCEMT_DBG("tr init success\n");			            		                  
                                }else if(icount == PRO_SERV_ACTION_STOP){
                                        if(sockfd)
                                                close(sockfd);				
			        }else{
			               
				        return PRO_BASE_ARG_ERR;                     
                                }
		        }
		if ((name = rec->get_val(rec, PRO_DLD_XL_NUM)) != NULL) {
			task_num = atoi(name);             
			if((task_num< 1) || (task_num > 10))
				return PRO_BASE_ARG_ERR;
		}

		if ((name = rec->get_val(rec, PRO_DLD_XL_CONNECT)) != NULL) {
			int port_len,j;
                        port_len = strlen(name);
                        for(j=0;j<port_len;j++){
                                if(!isdigit(name[j]))
                                        return PRO_BASE_ARG_ERR;
                        }
       
                        task_co = atoi(name);
			if((task_co < 1) || (task_co > 100))
				return PRO_BASE_ARG_ERR;
		}
		if ((name = rec->get_val(rec, PRO_DLD_XL_UPRATE)) != NULL) {
			uspeed = atoi(name);
			if((uspeed < 1) || (uspeed > 2048))
				return PRO_BASE_ARG_ERR;                      
		}

		if ((name = rec->get_val(rec, PRO_DLD_XL_DOWNRATE)) != NULL) {
			dspeed = atoi(name);
			if((dspeed < 1) || (dspeed > 2048))
				return PRO_BASE_ARG_ERR;				
		}
                if(tr_send_post_setlimit(sockfd, task_num, task_co, uspeed, dspeed))
                        MCEMT_DBG("set limit failure\n");
                     
		{
			FILE *fp;
			fp = popen("/etc/init.d/etcsync", "w");
			if(fp != NULL)
				pclose(fp);
		}

	}        
        MCEMT_DBG("pro_dld_xl_handler finish\n");
	return 0;
}



int pro_tr_base_handler(cgi_t *p, cgi_protocol_t *d, sys_outbuf_t *buf)
{
        MCEMT_DBG("pro_tr_base_handler start\n");

	cgi_rec_t *rec;
	int interval, num, down, up;
	char str[128];
        
        rec = p->cgirec;
	
        if((d->flag&PROTOCOL_FUCTION) == 0){
		/*get msg*/
		interval = 10;
		sprintf(str, "%d", interval);
		xml_add_elem(XML_LABEL, "interval", str, buf);

                tr_send_post_getlimit(sockfd, &num, &down, &up);
		sprintf(str, "%d", down);
		xml_add_elem(XML_LABEL, "lowspeed", str, buf);

		interval = 600;
		sprintf(str, "%d", interval);
		xml_add_elem(XML_LABEL, "lowtime", str, buf);

	}else{
		/*set msg*/
		char *c_time, *dspeed, *timeout;
		int itime, idspeed,itimeout;
		int isync = 0;
				
		if ((c_time = rec->get_val(rec, PRO_DLD_BASE_INTERVAL)) != NULL && c_time[0] != '\0') {
			itime = atoi(c_time);
			if((itime < 2) || (itime > 30))
				return PRO_BASE_ARG_ERR;			
			isync++;
		}
		if ((dspeed = rec->get_val(rec, PRO_DLD_BASE_LOWSPEED)) != NULL && dspeed[0] != '\0') {
			idspeed = atoi(dspeed);
                        if(tr_send_post_setdownlimit(sockfd, idspeed))
                                MCEMT_DBG("set down limit failure\n");
			isync++;
		}

		if ((timeout = rec->get_val(rec, PRO_DLD_BASE_LOWTIME)) != NULL && timeout[0] != '\0') {
			itimeout = atoi(timeout);
			isync++;
		}

		if(isync){
			FILE *fp;
			fp = popen("/etc/init.d/etcsync", "w");
			if(fp != NULL)
				pclose(fp);
		}
		

	}
        MCEMT_DBG("pro_tr_base_handler finish\n");
	return 0;
}



static void tr_task_to_simple_xml(task_info_t *task_info, sys_outbuf_t *buf)
{
	char str[4096], *outbuf = NULL;
	
	xml_add_elem(XML_ELEM_START, "record", NULL, buf);
        MCEMT_DBG("xl_task_to_simple_xml starts\n");
	str_to_utf8(task_info->file_name, &outbuf);
        char *pname = NULL;
        str_encode_url(outbuf, strlen(outbuf), &pname);
        str_to_upper(pname);
        MCEMT_DBG("file_name is %s\n", pname);
	if(outbuf&&pname) { 	
		xml_add_elem(XML_LABEL, "name", pname, buf);
                xml_add_elem(XML_LABEL, "nickname", pname, buf);
		free(outbuf);
                free(pname);
	} else {
		xml_add_elem(XML_LABEL, "name", NULL, buf);
                xml_add_elem(XML_LABEL, "nickname", "NULL", buf);
                if(outbuf)
                        free(outbuf);
                if(pname)
                        free(pname);
                       
	}
        
	sprintf(str, "%d", task_info->task_id);
	xml_add_elem(XML_LABEL, "id", str, buf);
    
        MCEMT_DBG("%lld\n", task_info->file_size);
    
	str_unit_to_cap(task_info->file_size, STR_UNIT_BYTE, str, 128);
       
	xml_add_elem(XML_LABEL, "size", str, buf);

	sprintf(str, "%lld", task_info->download_data_size);
	xml_add_elem(XML_LABEL, "csize", str, buf);

                                 
	unsigned long long utime;
	unsigned long long hour, min, sec;

	utime = task_info->utime;
	hour = utime / 3600;
	min = utime % 3600;
	sec = min % 60;
	min = min / 60;
	sprintf(str, "%llu:%llu:%llu", hour, min, sec);
	xml_add_elem(XML_LABEL, "utime", str, buf);

        sprintf(str, "%.2f", task_info->percentDone*100);
	xml_add_elem(XML_LABEL, "percent", str, buf);

	if (task_info->dl_speed == 0) {
		strcpy(str, "-:-:-");
	} else {
		unsigned long long lsize = task_info->left_data_size;
		unsigned long long hour, min, sec;
		sec = lsize / task_info->dl_speed;
		hour = sec / 3600;
		if (hour < 99) {
			min = sec % 3600;
			sec = min % 60;
			min = min / 60;
			sprintf(str, "%llu:%llu:%llu", hour, min, sec);
		} else {
			strcpy(str, "-:-:-");
		}
	}
	xml_add_elem(XML_LABEL, "ltime", str, buf);

	str_unit_to_cap(task_info->dl_speed, STR_UNIT_BYTE, str, 128);
	xml_add_elem(XML_LABEL, "speed", str, buf);

	sprintf(str, "%d/%d", task_info->downloading_pipe_num /* clients */,
					task_info->connecting_pipe_num /* sources */);
	xml_add_elem(XML_LABEL, "seed", str, buf);

	sprintf(str, "%d", task_info->failed_code);
	xml_add_elem(XML_LABEL, "error", str, buf);

	xml_add_elem(XML_ELEM_END, "record", NULL, buf);
	MCEMT_DBG("exit xl_task_to_simple_xml\n");

}




static void tr_task_to_xml(task_info_t *task_info, sys_outbuf_t *buf)
{
        MCEMT_DBG("tr_task_to_xml starts\n");
	
	char str[4096], *outbuf = NULL;
	
	xml_add_elem(XML_ELEM_START, "record", NULL, buf);
	sprintf(str, "%d", task_info->task_id);
	xml_add_elem(XML_LABEL, "id", str, buf);
	
	if (task_info->file_name == NULL) {
		MCEMT_DBG("task_info->file_name is NULL\n");
	} 

        str_to_utf8(task_info->file_name, &outbuf);
        char *pname = NULL;
        str_encode_url(outbuf, strlen(outbuf), &pname);
        str_to_upper(pname);
	if(outbuf&&pname) { 	
		xml_add_elem(XML_LABEL, "name", pname, buf);
                xml_add_elem(XML_LABEL, "nickname", pname, buf);
		free(outbuf);
                free(pname);
	} else {
		xml_add_elem(XML_LABEL, "name", NULL, buf);
                xml_add_elem(XML_LABEL, "nickname", "NULL", buf);
                if(outbuf)
                        free(outbuf);
                if(pname)
                        free(pname);
        }
    
	xml_add_elem(XML_LABEL, "type", "bt", buf);

	sprintf(str, "%lld", task_info->file_size);
	xml_add_elem(XML_LABEL, "size", str, buf);

	sprintf(str, "%lld", task_info->download_data_size);
	xml_add_elem(XML_LABEL, "csize", str, buf);

       	unsigned long long utime;
	unsigned long long hour, min, sec;

	utime = task_info->utime;
        hour = utime / 3600;
	min = utime % 3600;
	sec = min % 60;
	min = min / 60;
	sprintf(str, "%llu:%llu:%llu", hour, min, sec);
        xml_add_elem(XML_LABEL, "utime", str, buf);

        sprintf(str, "%.2f", task_info->percentDone*100);
	xml_add_elem(XML_LABEL, "percent", str, buf);

	if(task_info->dl_speed != 0){
		sprintf(str, "%d/%d", task_info->downloading_pipe_num, task_info->connecting_pipe_num);
		xml_add_elem(XML_LABEL, "seed", str, buf);

		unsigned long long lsize = task_info->left_data_size;
		unsigned long long hour, min, sec;

		sec = lsize / task_info->dl_speed;
		hour = sec / 3600;
		if(hour < 99) {
			min = sec % 3600;
			sec = min % 60;
			min = min / 60;
			sprintf(str, "%llu:%llu:%llu", hour, min, sec);
		}else{
			strcpy(str, "-:-:-");
		}
	
		xml_add_elem(XML_LABEL, "ltime", str, buf);

		str_unit_to_cap(task_info->dl_speed, STR_UNIT_BYTE, str, 128);
		xml_add_elem(XML_LABEL, "rate", str, buf);
		
	}else{
		xml_add_elem(XML_LABEL, "seed", "0/0", buf);
		xml_add_elem(XML_LABEL, "ltime", "-:-:-", buf);
		xml_add_elem(XML_LABEL, "rate", "0B", buf);
        }
        
     
        if(task_info->percentDone == 1)
                sprintf(str, "%s", "C"); 
        else if(task_info->status == 0)
                sprintf(str, "%s", "S");
        else    
                sprintf(str, "%s", "R");
        
	xml_add_elem(XML_LABEL, "status", str, buf);

	sprintf(str, "%d", task_info->failed_code);
	xml_add_elem(XML_LABEL, "error", str, buf);

	xml_add_elem(XML_ELEM_END, "record", NULL, buf);
        MCEMT_DBG(" tr_task_to_xml finish\n");
	
}




static int tr_get_list_task(int state, sys_outbuf_t *buf)
{
	MCEMT_DBG("xl_get_list_task start\n");
	int ret_val = 0, length = 0, i = 0;
	task_info_t *task_info = NULL;

        if(tr_send_post_get_info(sockfd, &task_info, &length)){
                MCEMT_DBG("get torrent info failure\n");
                return -1;
        }
        switch(state){
                case 0:
                        for(i=0;i<length;i++){
                                if(task_info[i].status == 4){
                                        tr_task_to_xml(&task_info[i], buf);
                                        free(task_info[i].file_name);
                                }
                        }
                        break;
                case 1:
                        for(i=0;i<length;i++){
                                if((task_info[i].status == 0)||(task_info[i].status == 1)||(task_info[i].status == 2)){
                                        tr_task_to_xml(&task_info[i], buf);
                                        free(task_info[i].file_name);
                                }
                        }       
                        break; 
                case 2:
                        for(i=0;i<length;i++){
                                if(task_info[i].percentDone == 1){
                                        tr_task_to_xml(&task_info[i], buf);
                                        free(task_info[i].file_name);
                                }
                        }        
                        break;
               default:
                        break;
        }
                free(task_info);
        
	MCEMT_DBG("xl_get_list_task finish\n");
	return ret_val;
}



static int tr_commit(char *url)
{
	MCEMT_DBG("xl_commit start!\n");
		
	if(tr_send_post_add(sockfd, url)){
                MCEMT_DBG("add torrent failure\n");
                return -1;
        }
      
	MCEMT_DBG("xl_commit finish\n");
        return 0;

}



int pro_tr_sinfo_handler(cgi_t *p, cgi_protocol_t *d, sys_outbuf_t *buf)
{
        MCEMT_DBG(" pro_xl_sinfo_handler start\n");
	int ret_val = 0, length = 0;
        task_info_t *task_info = NULL;
        if(tr_send_post_get_info(sockfd, &task_info, &length)){
                MCEMT_DBG("get torrent info failure\n");
                return -1;
               
        }
        int i = 0;
        for(i=0;i<length;i++){
                tr_task_to_simple_xml(&task_info[i], buf);
                if(task_info[i].file_name)
                        free(task_info[i].file_name);
        }
        free(task_info);
        MCEMT_DBG("pro_xl_sinfo_handler finish\n");
	return ret_val;	
}



int pro_tr_arg_handler(cgi_t *p, cgi_protocol_t *d, sys_outbuf_t *buf)
{       char str[128];
        MCEMT_DBG(" pro_xl_arg_handler start\n");
        status_info_t status_info;
        if(tr_send_post_get_session_status(sockfd, &status_info)){
                MCEMT_DBG("get session status failure\n");
                return -1;
        }
        sprintf(str, "%d", status_info.uploadspeed);
        xml_add_elem(XML_LABEL, "up_rate", str, buf);

        sprintf(str, "%d", status_info.downloadspeed);
        xml_add_elem(XML_LABEL, "down_rate", str, buf);
        
        MCEMT_DBG(" pro_xl_arg_handler finish\n");
        return 0;
       
}



int pro_tr_record_handler(cgi_t *p, cgi_protocol_t *d, sys_outbuf_t *buf)
{
	MCEMT_DBG("###########################\n");
        MCEMT_DBG(" pro_tr_record_handler start !\n");
        MCEMT_DBG("###########################\n");
	cgi_rec_t *rec;
	rec = p->cgirec;
	char *name = NULL;
	task_info_t *task_info = NULL;
	int ret = 0, length = 0, z = 1;
        int task[255]={0};
	do {
		name = rec->get_val_no(rec, PRO_XL_RECORD_LIST, z);
		if(name == NULL)
			break;
		if(strcmp(name,"run") == 0){
			ret = tr_get_list_task(0, buf);
			ret = tr_get_list_task(1, buf);
		}else if(strcmp(name,"finish") == 0){
			ret = tr_get_list_task(2, buf);
		}else if(strcmp(name,"id") == 0){
			int i = 1;
			char *s;
                        while((s=rec->get_val_no(rec, PRO_XL_RECORD_ID, i)) != NULL){
                                task[i]=atoi(s);
                                i++;
                                MCEMT_DBG("while\n")
                        }   
                        task[i] = -1;
                        if(!tr_send_post_get_info_num(sockfd, &task_info, &length, task)){
                                 MCEMT_DBG("get torrent info failure\n");
                                 return -1;
                        }         
                        int j = 0;
                        for(j=0;i<length;i++){
                        tr_task_to_xml(&task_info[i], buf);
                        if(task_info[i].file_name)
                                free(task_info[i].file_name);
                        }
                        free(task_info);
                }   
                z++;
		MCEMT_DBG("while\n")
	}while(name);
        MCEMT_DBG("pro_tr_record_handler finish\n");
	return 0;
}




int pro_tr_tasks_handler(cgi_t *p, cgi_protocol_t *d, sys_outbuf_t *buf)
{
        MCEMT_DBG("pro_tr_tasks_handler start\n");

	cgi_rec_t *rec;
	int i;
	char *s, *action;
	rec = p->cgirec;
        pthread_t tid;
	int ret = 0, result = 0;
	if ((action= rec->get_val(rec, PRO_XL_TASKS_ACTION)) == NULL || action[0] == '\0') {
		return PRO_BASE_ARG_ERR;
	}

	i = 1;
	
	if(!strcmp(action, "run")){
		do{ 
			s = rec->get_val_no(rec, PRO_XL_TASKS_ID, i);
			if (s != NULL) {
                                usleep(100);
                                tr_send_post_start(sockfd, atoi(s));
				i++;
			}
                        MCEMT_DBG("while\n")
		}while(s);                        
	}else if(!strcmp(action, "stop")){
               do{ 
                	s = rec->get_val_no(rec, PRO_XL_TASKS_ID, i);
			if(s != NULL) {
                                usleep(100);
                        	tr_send_post_stop(sockfd, atoi(s));
                                i++;
                        }
                        MCEMT_DBG("while\n")
                }while(s);
	}else if(!strcmp(action, "delete")){
                do{                       
                	s = rec->get_val_no(rec, PRO_XL_TASKS_ID, i);
			if (s != NULL) {
                                pthread_create(&tid, NULL, tr_send_post_remove, (void*)(atoi(s)));
                               	i++;
                        }
                        MCEMT_DBG("while\n")
                  }while(s);
	}else if(!strcmp(action, "deleteall")){
                do{
                	s = rec->get_val_no(rec, PRO_XL_TASKS_ID, i);
			if (s != NULL) {
                        	pthread_create(&tid, NULL, tr_send_post_remove, (void*)(atoi(s)));
                                i++;
                        }
                        MCEMT_DBG("while\n")
                }while(s);
	}
        MCEMT_DBG("pro_tr_tasks_handler finish\n");
        return 0;
}




int pro_tr_url_handler(cgi_t *p, cgi_protocol_t *d, sys_outbuf_t *buf)
{
	MCEMT_DBG("pro_tr_url_handler start!\n");
    
	cgi_t *cgi = p;
	cgi_rec_t *rec;
	char *urllist = NULL, *url, *end, *path, *sindex;
	int overflow = 0;
	int uid = -1;
	int i,j;
	int iindex[256];
	int *fileindex = NULL;

	int num = 0;

	rec = cgi->cgirec;
	
	/* Read urllist    */
	if(cgi->flag & CGI_BUF_OVERFLOW){
		urllist = cgi->cgi_read_fvar(cgi, PRO_XL_URL);
		if ((urllist == NULL) || (urllist[0] == '\0')) {
			return PRO_DLD_TASK_URL_ERR;
		}
		overflow = 1;
	}else
		urllist = rec->get_val(rec, PRO_XL_URL);

	if(urllist == NULL){
		MCEMT_DBG(" urlist is NULL\n");
	}
	/* path            */
	path = rec->get_val(rec, PRO_XL_URL_PATH);
	char *tmp = NULL;	
	MCEMT_DBG("the path is :%s\n", path );
	
	if((path == NULL) || (path[0] == '\0'))
		return PRO_DLD_TASK_URL_ERR;

        if(tr_send_post_setpath(sockfd, path) == 0)
                MCEMT_DBG("set path success\n");
                
	/* Get url list    */
	if(urllist == 0) {
		MCEMT_DBG("urllist is NULL\n"); 
	}	
	url = urllist;
	while(*url != '\0') {
	        MCEMT_DBG("the num is %d\n", num);	
		if(num == 200)
			break;
		end = strchr(url, '\r');
		if(end)
			*end = '\0';
		
		char *torrent = NULL;
		char torrenturl[256];
                strcpy(torrenturl, url);
                //str_to_upper(torrenturl);
		if((torrent = strrchr(url, '.')) != NULL){
			if((strcmp(torrent, ".torrent") == 0) || (strcmp(torrent, ".TORRENT") == 0)){
				sprintf(torrenturl, "/data/HardDisk1/Volume1/.vst/.torrent/%s", url);
			}
		}
                MCEMT_DBG("torrenturl is %s\n", torrenturl);
		tr_commit(torrenturl);
		num++;
                
		if (end == NULL)
			break;
		else {
			url = end + 1;
			/* Skip '\r','\n',' ', '\t' */
			while (*url != '\0') {
				if ((*url == '\r') || (*url == '\n') ||
				    (*url == ' ') || (*url == '\t')){
					url++;
                                        MCEMT_DBG("while\n")
                                    }
				else /* Other code, break */
					break;
			}
		}
	}

	/* Free the list   */
	
	if (overflow && urllist)
		free(urllist);
        MCEMT_DBG("pro_tr_url_handler finish \n");
	return 0;
}

void *tr_monitor()
{
	MCEMT_DBG("tr_monitor start\n");
        MCEMT_DBG("getpid is %ld\n", getpid());  
	int ret_val =0;
	
        while(1){
                ret_val = tr_init(&sockfd);
		if(ret_val == 0){
			MCEMT_DBG("tr init success\n");
			break;
		}
                MCEMT_DBG("while\n")
	
		sleep(60);
	}
        
        while(1){
		sleep(10);
		printf("tr monitor\n");
	}	

	MCEMT_DBG("tr_monitor finish\n");
        
	return NULL;
}

