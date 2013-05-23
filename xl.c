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
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>


#include "xl.h"
#include "xml.h"
#include "lthread.h"
#include "xl_util.h"
#include "tr_socket.h"
#include "getlicense.h"

#define DLD_ETC_PATH "/etc/dld.cfg"
#define XL_INFO_SPEED_PATH "/etc/xlspeed"
#define XL_INFO_TASK_PATH "/etc/xltask"
#define XL_LICENSE_PATH "/etc/xllicense"

int unbind_flag;


extern pthread_t tid;
extern int xl_flag;
static int pro_change_to_tr(cgi_t *p, cgi_protocol_t *d, sys_outbuf_t *buf);
static int pro_change_to_xl(cgi_t *p, cgi_protocol_t *d, sys_outbuf_t *buf);

static int pro_xl_maxtasks_handler(cgi_t *p, cgi_protocol_t *d, sys_outbuf_t *buf);
static int pro_xl_speed_handler(cgi_t *p, cgi_protocol_t *d, sys_outbuf_t *buf);
static int pro_xl_system_handler(cgi_t *p, cgi_protocol_t *d, sys_outbuf_t *buf);


static cgi_protocol_t pro_xl_list[] =
{
        {PRO_XL_SELECT, PROTOCOL_CLASS_SYS|PROTOCOL_LOGIN, pro_change_to_tr},
	{"speed", PROTOCOL_CLASS_SYS|PROTOCOL_LOGIN, pro_xl_speed_handler},
	{PRO_XL_SYSTEM, PROTOCOL_CLASS_SYS|PROTOCOL_LOGIN, pro_xl_system_handler},
	{"maxtask", PROTOCOL_CLASS_SYS|PROTOCOL_LOGIN, pro_xl_maxtasks_handler},
	{NULL, 0, NULL},
};

static cgi_protocol_t pro_tr_list[] =
{
        {PRO_XL_SELECT, PROTOCOL_CLASS_SYS|PROTOCOL_LOGIN, pro_change_to_xl},
	{PRO_XL_SIMPLEINFO, PROTOCOL_CLASS_SYS|PROTOCOL_LOGIN, pro_tr_sinfo_handler},
	{PRO_XL_ARG, PROTOCOL_CLASS_SYS|PROTOCOL_LOGIN, pro_tr_arg_handler},
	{PRO_XL_RECORD, PROTOCOL_CLASS_SYS|PROTOCOL_LOGIN, pro_tr_record_handler},
	{PRO_XL_TASKS, PROTOCOL_CLASS_SYS|PROTOCOL_LOGIN, pro_tr_tasks_handler},
	{PRO_XL_URL, PROTOCOL_CLASS_SYS|PROTOCOL_LOGIN, pro_tr_url_handler},
	{PRO_DLD_BASE, PROTOCOL_CLASS_SYS|PROTOCOL_LOGIN, pro_tr_base_handler},
	{PRO_DLD_XL, PROTOCOL_CLASS_SYS|PROTOCOL_LOGIN, pro_dld_tr_handler},
	{NULL, 0, NULL},
};



#if 0
static void xl_task_to_simple_xml(task_info_t *task_info, sys_outbuf_t *buf)
{

        char *user = NULL;
	char str[4096], *outbuf = NULL;
	float pst;
	
	xml_add_elem(XML_ELEM_START, "record", NULL, buf);
        MCEMT_DBG("xl_task_to_simple_xml starts\n");
	//sprintf(str, "<![CDATA[%s]]>", task_info._file_name);
	//xml_add_elem(XML_LABEL, "name", str, buf);

	str_to_utf8(task_info->file_name, &outbuf);
        MCEMT_DBG("file_name is %s\n", outbuf);
	if(outbuf) { 	
		xml_add_elem(XML_LABEL, "name", outbuf, buf);
                xml_add_elem(XML_LABEL, "nickname", outbuf, buf);
		free(outbuf);
	} else {
		xml_add_elem(XML_LABEL, "name", NULL, buf);
                xml_add_elem(XML_LABEL, "nickname", "NULL", buf);
        }

	sprintf(str, "%d", task_info->task_id);
	xml_add_elem(XML_LABEL, "id", str, buf);	
//	sprintf(str, "%llu", task_info->file_size);
	str_unit_to_cap(task_info->file_size, STR_UNIT_BYTE, str, 128);
        MCEMT_DBG("task_info->file_size:  %s \n", str);
	xml_add_elem(XML_LABEL, "size", str, buf);

	sprintf(str, "%llu", task_info->download_data_size);
	xml_add_elem(XML_LABEL, "csize", str, buf);

	{                                  
		u32 utime;
		unsigned long long hour, min, sec;

		if(task_info->finished_time == 0)
			utime = (u32)time((time_t *)0) -task_info->start_time;
		else
			utime = task_info->finished_time -task_info->start_time;
		
		hour = utime / 3600;
		
		min = utime % 3600;
		sec = min % 60;
		min = min / 60;
		sprintf(str, "%llu:%llu:%llu", hour, min, sec);

		xml_add_elem(XML_LABEL, "utime", str, buf);
	}
//	if(task_info->task_state == 3) {
//		sprintf(str, "%.2f", 100.0);
//	}else
	if(task_info->file_size == 0)
		sprintf(str, "%.2f", (float)0);
	else{
		pst = task_info->file_size*0.9999;
                MCEMT_DBG("the task_info->download_data_size is %llu!\n",task_info->download_data_size);
		if(pst > task_info->download_data_size){
			MCEMT_DBG("pst > download_data_size \n");
			sprintf(str, "%.2f",(float)task_info->download_data_size/task_info->file_size*100);
		}else{
			MCEMT_DBG("pst < download_data_size \n");
			sprintf(str, "%.2f",(float)99.8);
		}	
	}
	xml_add_elem(XML_LABEL, "percent", str, buf);

//	if(task_info->task_state == ETS_TASK_RUNNING){
//		ETM_RUNNING_STATUS running_status;
//
//		memset(&running_status,0,sizeof(ETM_RUNNING_STATUS ));
//		etm_get_task_running_status(task_info->task_id,&running_status);

		

		if (task_info->dl_speed == 0) {
			strcpy(str, "-:-:-");
		} else {
			unsigned long long lsize = task_info->file_size - task_info->download_data_size;
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
/*		
	}else{
		xml_add_elem(XML_LABEL, "ltime", "-:-:-", buf);
		xml_add_elem(XML_LABEL, "speed", "0B", buf);
		xml_add_elem(XML_LABEL, "seed", "0/0", buf);
	}
*/
	if(user){
		xml_add_elem(XML_LABEL, "user", user, buf);
		free(user);
	}else
		xml_add_elem(XML_LABEL, "user", "", buf);

	//not use
	xml_add_elem(XML_LABEL, "pri", "0", buf);

	//not use
	xml_add_elem(XML_LABEL, "enlarge", "0", buf);

	//is lixian
//	sprintf(str,"%d", etm_is_lixian_task(task_info->task_id));
//	xml_add_elem(XML_LABEL, "lixian", NULL, buf);

	//highspeedchanle
	xml_add_elem(XML_LABEL, "highspeed", "0", buf);

	sprintf(str, "%d", task_info->failed_code);
	xml_add_elem(XML_LABEL, "error", str, buf);

	xml_add_elem(XML_ELEM_END, "record", NULL, buf);
	MCEMT_DBG("exit xl_task_to_simple_xml\n");
	return;

        return;
}
#endif 



#if 0
static void xl_task_to_xml(task_info_t *task_info, sys_outbuf_t *buf)
{

        MCEMT_DBG("xl_task_to_xml starts\n");
	MCEMT_DBG("the task_info.file_size is %llu\n", task_info->file_size);
//	int ret_val = 0;
//	char data_buffer[65535];
//	u32 data_buffer_len = 65535;
	char *type = NULL, *user = NULL;
	char str[4096], *outbuf = NULL;
	float pst;
	
//	ret_val = etm_get_task_user_data(task_info.task_id,(void *) data_buffer,&data_buffer_len);
//	if(ret_val != 0)
//		return ;

//	xl_task_get_value(data_buffer, "TYPE", &type);
//	xl_task_get_value(data_buffer, "USER", &user);
//	xl_task_get_value(data_buffer, "URL", &url);
	
	xml_add_elem(XML_ELEM_START, "record", NULL, buf);
	sprintf(str, "%d", task_info->task_id);
	xml_add_elem(XML_LABEL, "id", str, buf);
	//sprintf(str, "<![CDATA[%s]]>", task_info._file_name);
	//xml_add_elem(XML_LABEL, "name", str, buf);
	if (task_info->file_name == NULL) {
		MCEMT_DBG("task_info->file_name is NULL\n");
	} 

	str_to_utf8(task_info->file_name, &outbuf);
	if(outbuf) { 	
		xml_add_elem(XML_LABEL, "name", outbuf, buf);
                xml_add_elem(XML_LABEL, "nickname", outbuf, buf);
		free(outbuf);
	} else {
		xml_add_elem(XML_LABEL, "name", NULL, buf);
                xml_add_elem(XML_LABEL, "nickname", "NULL", buf);
        }
	
	switch(task_info->task_type) {
		case 0:
			type = strdup("HTTP");
			break;
		case 1:
			type = strdup("BT");
			break;
		case 2:
			type = strdup("TCID");
			break;
		case 3:
			type = strdup("KANKAN");
			break;
		case 4:
			type = strdup("ED");
			break;
		case 5:
			type = strdup("FILE");
			break;
		default:
			break;
	}	
	
	if(type){
		xml_add_elem(XML_LABEL, "type", type, buf);
		free(type);
	}else
		xml_add_elem(XML_LABEL, "type", "Unknow", buf);

		sprintf(str, "%llu", task_info->created_file_size);
		xml_add_elem(XML_LABEL, "size", str, buf);

		sprintf(str, "%llu", task_info->download_data_size);
		xml_add_elem(XML_LABEL, "csize", str, buf);

	{                                  
		u32 utime;
		unsigned long long hour, min, sec;
		if(task_info->task_state == 0) {
			utime = 0;
		} else {
			if(task_info->finished_time == 0)
				utime = (u32)time((time_t *)0) -task_info->start_time;
			else
				utime = task_info->finished_time -task_info->start_time;
		}
		hour = utime / 3600;
		
		min = utime % 3600;
		sec = min % 60;
		min = min / 60;
		sprintf(str, "%llu:%llu:%llu", hour, min, sec);

		xml_add_elem(XML_LABEL, "utime", str, buf);
	}
	if(task_info->task_state == TS_TASK_SUCCESS) {
                sprintf(str, "%.2f", 100.0);	
	} else if(task_info->file_size == 0)
			sprintf(str, "%.2f", (float)0);
	else{
		pst = task_info->file_size*0.9999;
		MCEMT_DBG("the task_info->created_file_size is %llu!\n",task_info->file_size);
                MCEMT_DBG("the task_info->download_data_size is %llu!\n",task_info->download_data_size);
		MCEMT_DBG("pst is %f!\n", pst);
		if(pst > task_info->download_data_size){
			MCEMT_DBG("pst > download_data_size\n");
			sprintf(str, "%.2f", (float)task_info->download_data_size/task_info->file_size*100);
		}else{
			MCEMT_DBG("pst < download_data_size\n");
			sprintf(str, "%.2f", 99.8);
		}
	}
	xml_add_elem(XML_LABEL, "percent", str, buf);

	if(task_info->task_state == TS_TASK_RUNNING){
//		ETM_RUNNING_STATUS running_status;

//		memset(&running_status,0,sizeof(ETM_RUNNING_STATUS ));
//		etm_get_task_running_status(task_info->task_id,&running_status);

//		sprintf(str, "%d/%d", running_status._downloading_pipe_num /* clients */,
//						running_status._connecting_pipe_num /* sources */);
		sprintf(str, "%d/%d", task_info->downloading_pipe_num, task_info->connecting_pipe_num);
		xml_add_elem(XML_LABEL, "seed", str, buf);

		if (task_info->dl_speed == 0) {
			strcpy(str, "-:-:-");
		} else {
			unsigned long long lsize = task_info->file_size - task_info->download_data_size;
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
		xml_add_elem(XML_LABEL, "rate", str, buf);
		
	}else{
		xml_add_elem(XML_LABEL, "seed", "0/0", buf);
		xml_add_elem(XML_LABEL, "ltime", "-:-:-", buf);
		xml_add_elem(XML_LABEL, "rate", "0B", buf);
	}

	if (task_info->task_state == ETS_TASK_RUNNING) {
		sprintf(str, "%s", "R");
	} else if (task_info->task_state == ETS_TASK_WAITING) {
		sprintf(str, "%s", "W");
	} else if (task_info->task_state == ETS_TASK_PAUSED) {
		sprintf(str, "%s", "S");
	} else if (task_info->task_state == ETS_TASK_DELETED) {
		sprintf(str, "%s", "D");
	} else if (task_info->task_state == ETS_TASK_SUCCESS) {
		sprintf(str, "%s", "C");
	} else if (task_info->task_state == ETS_TASK_FAILED) {
		//sprintf(str, "%s", "S");
		sprintf(str, "%s", "F");
	}
	xml_add_elem(XML_LABEL, "status", str, buf);

	if(task_info->url){
		sprintf(str, "%s", task_info->url);
		xml_add_elem(XML_LABEL, "url", str, buf);
	}else
		xml_add_elem(XML_LABEL, "url", "NULL", buf);
	

	outbuf = NULL;
	str_to_utf8(task_info->file_path, &outbuf);	
	if(outbuf){     
                //      str_encode_url(outbuf, strlen(outbuf), &pname);
		MCEMT_DBG(" task->file_path is %s\n", outbuf);
       		xml_add_elem(XML_LABEL, "path", outbuf, buf);
              	free(outbuf);	
       	} else 
	xml_add_elem(XML_LABEL, "path", NULL, buf);

	if(user){
		xml_add_elem(XML_LABEL, "user", user, buf);
		free(user);
	}else
		xml_add_elem(XML_LABEL, "user", "", buf);

/*
	if(task_info->task_type== ETT_BT){
	//	sprintf(str, "%d", task_info->_bt_total_file_num);
        //      not use
       //         xml_add_elem(XML_LABEL, "f_sum", "0", buf);                      
	//	xml_add_elem(XML_LABEL, "f_sum", str, buf);
	//	xl_bt_to_xml(task_info->task_id, 0, buf);

	}else{
		char *pname = NULL;
		
		xml_add_elem(XML_LABEL, "f_sum", "1", buf);
		xml_add_elem(XML_ELEM_START, "flist", NULL, buf);
		xml_add_elem(XML_LABEL, "index", "0", buf);

		str_encode_url(task_info->file_name, strlen(task_info->file_name), &pname);
		if(pname != NULL){
			xml_add_elem(XML_LABEL, "file", pname, buf);
			free(pname);
		}else{
			xml_add_elem(XML_LABEL, "file", task_info->file_name, buf);
		}

		xml_add_elem(XML_ELEM_END, "flist", NULL, buf);
	}
*/

	//not use
	xml_add_elem(XML_LABEL, "pri", "0", buf);

	//not use
	xml_add_elem(XML_LABEL, "enlarge", "0", buf);

	//is lixian
//	sprintf(str,"%d", etm_is_lixian_task(task_info->task_id));
//	xml_add_elem(XML_LABEL, "lixian", NULL, buf);

	//highspeedchanle
	xml_add_elem(XML_LABEL, "highspeed", "0", buf);
	

	sprintf(str, "%d", task_info->failed_code);
	xml_add_elem(XML_LABEL, "error", str, buf);

	xml_add_elem(XML_ELEM_END, "record", NULL, buf);
        MCEMT_DBG(" xl_task_to_xml finish\n"); 
        return ;
}
#endif 



#if 0
static int  xl_get_list_task(ETM_TASK_STATE state, sys_outbuf_t *buf)
{        
	MCEMT_DBG("xl_get_list_task start\n");
	unsigned int  task_id_array[512];
	unsigned int id_array_size = 512,index=0,task_id=0;
	int ret_val = 0;
	task_info_t task_info;
	memset(&task_info, 0, sizeof(task_info_t));

	id_array_size = 512;
	ret_val = etm_get_task_id_by_state(state,task_id_array,&id_array_size);
	if(ret_val ==0){
		if(id_array_size>512) 
			id_array_size=512;
		for(index=0;index<id_array_size;index++)
		{
			task_id = task_id_array[index];
//			ret_val=etm_get_task_info(task_id,&task_info);
                        int result = 0;
                        get_task_info_by_id(task_id, &task_info, &result);
			if(result != 0){
				if (task_info.file_name) {
        				free(task_info.file_name);
        			}    
        
       			 	if (task_info.file_path) {
        				free(task_info.file_path); 
        			}
				if (task_info.url) {
					free(task_info.url);
				} 
				return result;
			}else{
			//display_task_info(&task_info);
       			MCEMT_DBG("task_info.file_size is %llu\n", task_info.file_size);
				xl_task_to_xml(&task_info, buf);
				if (task_info.file_name) {
        				free(task_info.file_name);
        			}    
        
        			if (task_info.file_path) {
        				free(task_info.file_path); 
        			} 
				if (task_info.url) {
					free(task_info.url);
				}	
			}
		}
	}

	MCEMT_DBG("xl_get_list_task finish\n");

	return ret_val;

        return 0;
}
#endif


static cgi_protocol_t* find_pro_handler(const char * pro_opt)
{
	MCEMT_DBG("find_pro_handler start\n");
	int i;
	if(pro_opt == NULL)
		return NULL;
	i = 0;
        if(xl_flag == 1){
        	while(1){
                        MCEMT_DBG("while\n")
        		if(pro_xl_list[i].name == NULL)
        			return NULL;
        		if(strcmp(pro_xl_list[i].name, pro_opt) == 0){
        			return &pro_xl_list[i];
        		}			
        		i++;
        	}
        }else if(xl_flag == 0){
               while(1){
                        MCEMT_DBG("while\n")
        		if(pro_tr_list[i].name == NULL)
        			return NULL;
        		if(strcmp(pro_tr_list[i].name, pro_opt) == 0){
        			return &pro_tr_list[i];
        		}			
        		i++;
        	}
        }else{
                exit(0);
        }
                        
      	MCEMT_DBG("find_pro_handler finish\n");
	return NULL;
}



void default_cgi_handler(void *data)
{
	MCEMT_DBG("default_cgi_handler start\n");
	cgi_rec_t *rec;
	cgi_t *cgi = (cgi_t *)data;
	char *pro_opt = NULL, *pro_class = NULL;
	sys_outbuf_t *buf = NULL;
	int len, pro_errno=0;
	char str[128];
	cgi_protocol_t *cur_protocol;
        char *pro_function;
        int function_flag;

	rec = cgi->cgirec;

	if ((buf = sys_outbuf_new(&buf)) == NULL) {
		return;
	} else {
		buf->set_blksize(buf, 2048);
	}

	if ((pro_class = rec->get_val(rec, "fname")) == NULL ||
		pro_class[0] == '\0') {
		MCEMT_DBG("pro_class is NULL\n");
		pro_errno = PRO_BASE_ARG_ERR;
	}

	if ((pro_opt = rec->get_val(rec, "opt")) == NULL ||
		pro_opt[0] == '\0') {
		MCEMT_DBG("pro_opt is NULL\n");
		pro_errno = PRO_BASE_ARG_ERR;
	}
	xml_add_header(buf);

	xml_add_elem(XML_ELEM_START, pro_class, NULL, buf);

	xml_add_elem(XML_ELEM_START, pro_opt, NULL, buf);
        
	if (((pro_function = rec->get_val(rec, "function")) == NULL) &&
		((pro_function = rec->get_val(rec, "fun")) == NULL)) {
		MCEMT_DBG("pro_function is NULL\n");
                pro_errno = PRO_BASE_ARG_ERR;
	}else{       
                if(strcmp(pro_function, "set") == 0)
			function_flag = 1;
		else if(strcmp(pro_function, "get") == 0)
			function_flag = 0;
		else{
			pro_errno = PRO_BASE_ARG_ERR;
		}
	}
        MCEMT_DBG("pro_errno is %d\n", pro_errno);
	if(!pro_errno){
		cur_protocol = find_pro_handler(pro_opt);
		if(cur_protocol == NULL){
                        MCEMT_DBG("cur_protocol == NULL");
			pro_errno = PRO_BASE_ARG_ERR;
		}else{	
                        
                        if(function_flag == 1)
                                PRO_FLAG_SET_MODE(cur_protocol->flag);
                        else
                                PRO_FLAG_GET_MODE(cur_protocol->flag);

                        PRO_FLAG_XML_MODE(cur_protocol->flag); 
      			pro_errno = cur_protocol->handler(cgi, cur_protocol, buf);
		}
	}
	sprintf(str, "%d", pro_errno);
	xml_add_elem(XML_LABEL, "errno", str, buf);
	xml_add_elem(XML_ELEM_END, pro_opt, NULL, buf);
	xml_add_elem(XML_ELEM_END, pro_class, NULL, buf);
	xml_add_end(buf);

	/* Initialize http header */
	len = buf->get_bufsize(buf);
	cgi->cgi_init_header(cgi, len, CGI_HEADER_TYPE_XML);

	/* Output the buffer      */
	buf->print(buf, cgi->sockfd);
	
	/* Destroy buffer object  */
	buf->clear(buf);
	buf->del(buf);
	MCEMT_DBG("default_cgi_handler finish\n");
	return;
}



static int pro_xl_system_handler(cgi_t *p, cgi_protocol_t *d, sys_outbuf_t *buf)
{	
	MCEMT_DBG("pro_xl_system_handler start\n");
        char bind_key[32];
        int ret;
        char *action;
        cgi_rec_t *rec;
        int bind_ok, net_ok, license_ok, disk_ok;
	rec = p->cgirec;

        memset(&bind_key, '\0', sizeof(bind_key));
        action= rec->get_val(rec, "action");
        bind_ok = 1;
        if(!action){
                if(unbind_flag){
                        while(bind_ok == 1)
                                ret = xl_getsysinfo(bind_key, &bind_ok, &net_ok, &license_ok, &disk_ok);
                        unbind_flag = 0;
                }
                
                ret = xl_getsysinfo(bind_key, &bind_ok, &net_ok, &license_ok, &disk_ok);                        
                if(ret == 0){
                        xml_add_elem(XML_LABEL, "bindkey", bind_key, buf);
                        if(bind_ok)
                                xml_add_elem(XML_LABEL, "bind_ok", "1", buf);
                        else
                                xml_add_elem(XML_LABEL, "bind_ok", "0", buf);
                        if(net_ok)
                                xml_add_elem(XML_LABEL, "net_ok", "1", buf);
                        else
                                xml_add_elem(XML_LABEL, "net_ok", "0", buf);
                        if(license_ok)
                                xml_add_elem(XML_LABEL, "license_ok", "1", buf);
                        else
                                xml_add_elem(XML_LABEL, "license_ok", "0", buf);
                        if(disk_ok)
                                xml_add_elem(XML_LABEL, "disk_ok", "1", buf);
                        else
                                xml_add_elem(XML_LABEL, "disk_ok", "0", buf);
        	        return 0;
               }else
                        return -1;
        }else{                      
                if(!strcmp(action, "unbind")){
                        unbind_flag = 1;
                        if(!xl_unbind())
                                return 0;
                        else 
                                return -1;
                } 
        }    
        return -1;
}


static int pro_xl_maxtasks_handler(cgi_t *p, cgi_protocol_t *d, sys_outbuf_t *buf)
{
        MCEMT_DBG("pro_dld_base_handler\n");
        cgi_rec_t *rec;
        char* count;
        int icount;
        FILE *fp, *fpp;
        char buff[12];
        char  *n;
        rec = p->cgirec;

        char touch[32];
        sprintf(touch, "touch %s", XL_INFO_TASK_PATH);

        if(access(XL_INFO_TASK_PATH, F_OK)){
                fpp = popen(touch, "r");
                pclose(fpp);
        }
                
        if((d->flag&PROTOCOL_FUCTION) == 0){
                fp = fopen(XL_INFO_TASK_PATH, "r");
                if(!fp)
                        return -1;
                memset(buff, '\0', 12);
                n = fgets(buff, 12, fp);
                if(n)
                        xml_add_elem(XML_LABEL, "num", buff, buf);
                else 
                        xml_add_elem(XML_LABEL, "num", "3", buf);
                fclose(fp);
                return 0;
        }else{
                if ((count = rec->get_val(rec, PRO_DLD_XL_NUM)) != NULL) {                           
                		icount = atoi(count);
                		if((icount < 1) || (icount > 3))
                			return PRO_BASE_ARG_ERR;
                                if(!xl_set_maxtask(icount)){
                                        fp = fopen(XL_INFO_TASK_PATH, "w");
                                        if(!fp)
                                                return -1;
                                        fprintf(fp, "%d", icount);
                                        fclose(fp);
                                        FILE *fpsyn;
	                                fpsyn = popen("/etc/init.d/etcsync", "w");
	                                if(fpsyn != NULL)
		                                pclose(fpsyn); 
                                        return 0;
                                }else
                                        return 1024;
                }
        }
        
        return 0;  
}  



static int pro_xl_speed_handler(cgi_t *p, cgi_protocol_t *d, sys_outbuf_t *buf)
{
        MCEMT_DBG("start pro_dld_xl_handler\n");
        int uspeed;
        int dspeed;
        FILE *fp, *fpp;
        char upb[12];
        char dob[12];
        cgi_rec_t *rec;
	rec = p->cgirec;
        char *n;
	/*set msg*/
	char *name = NULL;           
	int icount;
        int i = 3;
        char touch[32];
        sprintf(touch, "touch %s", XL_INFO_SPEED_PATH);

        if(access(XL_INFO_SPEED_PATH, F_OK)){
                fpp = popen(touch, "r");
                pclose(fpp);
        }
                
        if((d->flag&PROTOCOL_FUCTION) == 0){
                fp = fopen(XL_INFO_SPEED_PATH, "r");
                if(!fp)
                        return -1;
        
                memset(upb, '\0', 12);
                memset(dob, '\0', 12);
                n = fgets(dob, 12, fp);
                if(n)
                        xml_add_elem(XML_LABEL, "downrate", dob, buf);
                else
                        xml_add_elem(XML_LABEL, "downrate", "0", buf);
                fgets(upb, 12, fp);
                if(n)                        
                        xml_add_elem(XML_LABEL, "uprate", upb, buf);
                else
                        xml_add_elem(XML_LABEL, "uprate", "0", buf);
                fclose(fp);
                return 0;
        }else{
                fp = fopen(XL_INFO_SPEED_PATH, "w");
                if(!fp)
                return -1;
        
        	if ((name = rec->get_val(rec, PRO_DLD_XL_UPRATE)) != NULL) {
                        MCEMT_DBG("the name is %s\n", name);
        		icount = atoi(name);
                        uspeed = icount;                        
        	}
        	if ((name = rec->get_val(rec, PRO_DLD_XL_DOWNRATE)) != NULL) {
        		icount = atoi(name);
                        dspeed = icount;
        	}

                if(!dspeed) dspeed = -1;
                if(!uspeed) uspeed = -1;

                while(i>0){       
                        if(!xl_set_limit_speed(dspeed, uspeed)){
                               if(dspeed==-1) dspeed = 0;
                               if(uspeed==-1) uspeed = 0;
                               MCEMT_DBG("dspeed is %d\n", dspeed);
                               fprintf(fp, "%d", dspeed);
                               fputs("\n", fp);
                               fprintf(fp, "%d", uspeed);
                               fputs("\n", fp);
                               fclose(fp);
                                FILE *fpsyn;
	                        fpsyn = popen("/etc/init.d/etcsync", "w");
	                        if(fpsyn != NULL)
		                        pclose(fpsyn); 
                	       return 0;
                        }else
                               i--;
                }   
                return -1;
        }
        
        return 0;
}



static int pro_change_to_xl(cgi_t *p, cgi_protocol_t *d, sys_outbuf_t *buf)
{
        MCEMT_DBG("pro_change_to_xl start\n");
        FILE *fd;
        char fl[10];
        fd = fopen(DLD_ETC_PATH, "r+");
        fgets(fl, 10, fd);
        xl_flag = atoi(fl);
        if(xl_flag == 0){
                pthread_cancel(tid);
                if(pthread_create(&tid, NULL, xl_monitor, NULL) != 0){
                        xml_add_elem(XML_LABEL, "xl", "1", buf); 
                        fputs("0", fd);
                        fclose(fd);
                }else{
                        xml_add_elem(XML_LABEL, "xl", "-1", buf);
                        fclose(fd);
                }
        }
        MCEMT_DBG("pro_change_to_xl start\n");
        return 0;
}



static int pro_change_to_tr(cgi_t *p, cgi_protocol_t *d, sys_outbuf_t *buf)
{
        MCEMT_DBG("pro_change_to_tr start\n");
        FILE *fd;
        char fl[10];
        fd = fopen(DLD_ETC_PATH, "r+");
        fgets(fl, 10, fd);
        xl_flag = atoi(fl);
        if(xl_flag == 1){
                pthread_cancel(tid);
                if(pthread_create(&tid, NULL, tr_monitor, NULL) != 0){
                        xml_add_elem(XML_LABEL, "tr", "1", buf);
                        fputs("0", fd);
                        fclose(fd);
                }else{
                        xml_add_elem(XML_LABEL, "tr", "-1", buf);
                        fclose(fd);
                }
        }

        MCEMT_DBG("pro_change_to_tr finish\n");
        return 0;
                
}



void *xl_monitor(void *arg)
{
	int sockf = -1;       
	MCEMT_DBG("xl_monitor start\n");
        FILE *fp, *fpl;
        char buff[64];
        char xls[256];  
        char *license;
      	//vs_cfg_init();  
       // MCEMT_DBG("get_license enter\n");
        
        /*
        license = get_license(license);
        MCEMT_DBG("license is %s\n", license);  
        if (license == NULL) {
                MCEMT_DBG("GET LICENSE FAILURE!\n");
                exit(-1);
        }
        
        char *g_key = NULL;
        g_key = strdup(license);
                            
        if (license != NULL){
                free(license);
        }	
        */

        fpl = fopen(XL_LICENSE_PATH, "r");

        if(fpl){
                fgets(buff, 64, fpl);
                license = strstr(buff, "license=");
                license+=8;
                fclose(fpl);        
                sprintf(xls, "/usr/local/etc/thunder/thunder.sh 144 %s", license);
                fp = popen(xls, "r");
                
        }else
                fp = popen("/usr/local/etc/thunder/thunder.sh 144 120831000100000010001443i3abvz6hxluf9091a8", "r");
	
	if(fp==NULL){
                printf("xl start failure\n");
                exit(-1);
        }
 
        while(1){
                sockf = xl_opensock("127.0.0.1");
                if(sockf > 0){
                        close(sockf);
			sockf = -1;
			break;
		}
                sleep(1);
        }

        pclose(fp);  
	while(1){
		sleep(10);
		printf("xl monitor\n");
	}
}

