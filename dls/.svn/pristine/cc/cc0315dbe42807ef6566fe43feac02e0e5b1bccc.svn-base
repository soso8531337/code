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

#include "etm_json_interface.h"
#include "xl.h"
#include "xml.h"
#include "lthread.h"

#include <ctype.h>


#define DLD_ETC_PATH "/etc/dld.cfg"

extern pthread_t tid;
extern int xl_flag;

static int pro_change_to_tr(cgi_t *p, cgi_protocol_t *d, sys_outbuf_t *buf);
static int pro_change_to_xl(cgi_t *p, cgi_protocol_t *d, sys_outbuf_t *buf);


static int pro_dld_xl_handler(cgi_t *p, cgi_protocol_t *d, sys_outbuf_t *buf);
static int pro_xl_base_handler(cgi_t *p, cgi_protocol_t *d, sys_outbuf_t *buf);

static int pro_xl_sinfo_handler(cgi_t *p, cgi_protocol_t *d, sys_outbuf_t *buf);
static int pro_xl_arg_handler(cgi_t *p, cgi_protocol_t *d, sys_outbuf_t *buf);
static int pro_xl_record_handler(cgi_t *p, cgi_protocol_t *d, sys_outbuf_t *buf);
static int pro_xl_tasks_handler(cgi_t *p, cgi_protocol_t *d, sys_outbuf_t *buf);
static int pro_xl_url_handler(cgi_t *p, cgi_protocol_t *d, sys_outbuf_t *buf);
static int pro_xl_system_handler(cgi_t *p, cgi_protocol_t *d, sys_outbuf_t *buf);


static cgi_protocol_t pro_xl_list[] =
{
        {PRO_XL_SELECT, PROTOCOL_CLASS_SYS|PROTOCOL_LOGIN, pro_change_to_tr},
	{PRO_XL_SIMPLEINFO, PROTOCOL_CLASS_SYS|PROTOCOL_LOGIN, pro_xl_sinfo_handler},
	{PRO_XL_ARG, PROTOCOL_CLASS_SYS|PROTOCOL_LOGIN, pro_xl_arg_handler},
	{PRO_XL_RECORD, PROTOCOL_CLASS_SYS|PROTOCOL_LOGIN, pro_xl_record_handler},
	{PRO_XL_TASKS, PROTOCOL_CLASS_SYS|PROTOCOL_LOGIN, pro_xl_tasks_handler},
	{PRO_XL_URL, PROTOCOL_CLASS_SYS|PROTOCOL_LOGIN, pro_xl_url_handler},
	{PRO_XL_SYSTEM, PROTOCOL_CLASS_SYS|PROTOCOL_LOGIN, pro_xl_system_handler},
	{PRO_DLD_BASE, PROTOCOL_CLASS_SYS|PROTOCOL_LOGIN, pro_xl_base_handler},
	{PRO_DLD_XL, PROTOCOL_CLASS_SYS|PROTOCOL_LOGIN, pro_dld_xl_handler},
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



int xl_stop_etm(void)
{
	MCEMT_DBG("xl_stop_etm start\n");
	int ret_val =0;
	printf("\n================================\netm_uninit...byebye!\n\n");
	// ∑¥≥ı ºªØ
	 etm_uninit();

	MCEMT_DBG("xl_etm_stop finish\n");
	return ret_val;
}



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
}

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
	return;
}

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
}

static int srv_ck_url(char *url, char *strtype)
{
	MCEMT_DBG("srv_ck_url start\n");
	int ret = FAILURE;
	char  *s;
	char prefix[32];

	strcpy(strtype, "Unknow");

	if (url == NULL)
		return ret;
	else {
        	memset(prefix, 0, sizeof(prefix));
        	strncpy(prefix, url, 16);
        	str_to_lower(prefix);
	}
	MCEMT_DBG("URL:%s\n", url);
	MCEMT_DBG("PREFIX:%s\n", prefix);

	if (*(url+1) == ':') { /* Local BT seed file  */
		s = url + strlen(url) - 8;
		if ((strcmp(s, ".torrent") == 0) || (strcmp(s, ".TORRENT") == 0)) {
			ret = DLD_BT_TYPE;
		}
		strcpy(strtype, "BT");
	} else if (strncmp(prefix, EMULE_PREFIX, EMULE_PREFIX_LEN) == 0) {
		ret = DLD_ML_TYPE;
		strcpy(strtype, "ED");
	} else if (strncmp(prefix, FTP_PREFIX, FTP_PREFIX_LEN) == 0) {
		ret = DLD_FTP_TYPE;
		strcpy(strtype, "FTP");
	} else if ((strncmp(prefix, HTTP_PREFIX, HTTP_PREFIX_LEN) == 0)
		|| (strncmp(prefix, HTTPS_PREFIX, HTTPS_PREFIX_LEN) == 0)) {
		ret = DLD_HTTP_TYPE;
		strcpy(strtype, "HTTP");
	} else if (strncmp(prefix, THUNDER_PREFIX, THUNDER_PREFIX_LEN) == 0) {
		ret = DLD_THUNDER_TYPE;
		strcpy(strtype, "THUNDER");
	} else if (strncmp(prefix, CID_PREFIX, CID_PREFIX_LEN) == 0) {
		ret = DLD_CID_TYPE;
		strcpy(strtype, "CID");
	} else if (strncmp(prefix, FLASHGET_PREFIX, FLASHGET_PREFIX_LEN) == 0) {
		ret = DLD_FLASHGET_TYPE;
		strcpy(strtype, "FLASHGET");
        } else if (strncmp(prefix, QQ_PREFIX, QQ_PREFIX_LEN) == 0) {
		ret = DLD_QQ_TYPE;
		strcpy(strtype, "QQ");
        } else if (strncmp(prefix, RAYFILE_PREFIX, RAYFILE_PREFIX_LEN) == 0) {
		ret = DLD_RAYFILE_TYPE;
		strcpy(strtype, "FAYFILE");
        }
 	MCEMT_DBG("srv_ck_url finish\n");
	return ret;
}

/*-----------------------------------------------------------------------------
  Download link parse operations
------------------------------------------------------------------------------*/

static char base64_decode_map[256] = {	
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 62, 255, 255, 255, 63, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 255, 255,
	255, 0, 255, 255, 255, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
	15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 255, 255, 255, 255, 255, 255, 26, 27, 28,
	29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
	49, 50, 51, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255};

static int thunder_decode(const char *sstr, char **dstr)
{
	MCEMT_DBG("thunder_decode start\n");
	int i = 0, j = 0, src_len = 0;
	char src[4096], dst[4096];
	char *tmp = NULL;
	
	memset(src, 0, 4096);
	memset(dst, 0, 4096);
	
	strcpy(src, &sstr[THUNDER_PREFIX_LEN]);
	//printf("src: %s\n", src);

	src_len = strlen(src);
	for (; i < src_len; i += 4) {
		dst[j++] = base64_decode_map[(int)src[i]] << 2 | base64_decode_map[(int)src[i + 1]] >> 4;
		dst[j++] = base64_decode_map[(int)src[i + 1]] << 4 | base64_decode_map[(int)src[i + 2]] >> 2;
		dst[j++] = base64_decode_map[(int)src[i + 2]] << 6 | base64_decode_map[(int)src[i + 3]];
	}
	//printf("dest: %s\n", dst);
	/* AAXXXXZZ */
    	tmp = strrchr(dst, 'Z');
    	if (tmp != NULL) {
    		*(tmp-1) = '\0';
    	} else {
		return FAILURE;
	}
	
	*dstr=strdup(dst+2);
	MCEMT_DBG("thunder_decode finish\n");
	return SUCCESS;
}

#define FLASHGET_FLAG            "[FLASHGET]"
#define FLASHGET_FLAG_LEN        (10)
static int flashget_decode(const char *sstr, char **dstr)
{
	MCEMT_DBG("flashget_decode start\n");
        int i = 0, j = 0, src_len = 0;
        char src[4096], dst[4096];
        char *tmp = NULL;
 
        memset(src, 0, 4096);
        memset(dst, 0, 4096);
 
        strcpy(src, &sstr[FLASHGET_PREFIX_LEN]);
//        printf("flashget src: %s\n", src);
 
        src_len = strlen(src);
        for (; i < src_len; i += 4) {
                dst[j++] = (char)(base64_decode_map[(int)src[i]] << 2 | base64_decode_map[(int)src[i + 1]] >> 4);
                dst[j++] = (char)(base64_decode_map[(int)src[i + 1]] << 4 | base64_decode_map[(int)src[i + 2]] >> 2);
                dst[j++] = (char)(base64_decode_map[(int)src[i + 2]] << 6 | base64_decode_map[(int)src[i + 3]]);
        }
//        printf("dest: %s\n", dst);
 
        /* [FLASHGET]XXXX[FLASHGET] */
        tmp = strstr(dst+FLASHGET_FLAG_LEN, "[FLASHGET]");
        if (tmp != NULL) {
                *tmp = '\0';
        } else {
                return FAILURE;
        }
 
        *dstr = strdup(dst+FLASHGET_FLAG_LEN);
 	MCEMT_DBG("flashget_decode finish\n");
        return SUCCESS;
}

static int qq_decode(const char *sstr, char **dstr)
{
	MCEMT_DBG("qq_decode start\n");
        int i = 0, j = 0, src_len = 0;
        char src[4096], dst[4096];
 
        memset(src, 0, 4096);
        memset(dst, 0, 4096);
 
        strcpy(src, &sstr[QQ_PREFIX_LEN]);
//        printf("qq src: %s\n", src);
 
        src_len = strlen(src);
        for (; i < src_len; i += 4) {
                dst[j++] = (char)(base64_decode_map[(int)src[i]] << 2 | base64_decode_map[(int)src[i + 1]] >> 4);
                dst[j++] = (char)(base64_decode_map[(int)src[i + 1]] << 4 | base64_decode_map[(int)src[i + 2]] >> 2);
                dst[j++] = (char)(base64_decode_map[(int)src[i + 2]] << 6 | base64_decode_map[(int)src[i + 3]]);
        }
 
//        printf("qq dest: %s\n", dst);
        *dstr=strdup(dst);
 	MCEMT_DBG("qq_decode finish\n");
        return SUCCESS;
}

static int rayfile_decode(const char *sstr, char **dstr)
{
	MCEMT_DBG("rayfile_decode start\n");
        int i = 0, j = 0, src_len = 0;
        char src[4096], dst[4096];
        char *tail = NULL;
 
        memset(src, 0, 4096);
        memset(dst, 0, 4096);
 
        strcpy(src, &sstr[RAYFILE_PREFIX_LEN]);
//        printf("rayfile src: %s\n", src);
 
        src_len = strlen(src);
        for (; i < src_len; i += 4) {
                dst[j++] = (char)(base64_decode_map[(int)src[i]] << 2 | base64_decode_map[(int)src[i + 1]] >> 4);
                dst[j++] = (char)(base64_decode_map[(int)src[i + 1]] << 4 | base64_decode_map[(int)src[i + 2]] >> 2);
                dst[j++] = (char)(base64_decode_map[(int)src[i + 2]] << 6 | base64_decode_map[(int)src[i + 3]]);
        }
//        printf("rayfile dest: %s\n", dst);
 
        /* |...., Add the "http://", delete "|..." */
        tail = strrchr(dst, '|');
        if (tail != NULL) {
                *tail = '\0';
        }
        memset(src, 0, 4096);
        strcpy(src, "http://");
        strcat(src, dst);
        *dstr=strdup(src);
 	MCEMT_DBG("rayfile_decode finish\n");
        return SUCCESS;
}


char *parse_link(const char *url)
{
	MCEMT_DBG("parse_link start\n");
        char *tmpurl = NULL;
        char prefix[32];
        int ret = 0;
 
        memset(prefix, 0, sizeof(prefix));
        strncpy(prefix, url, 16);
        str_to_lower(prefix);
 
        if (strncmp(prefix, THUNDER_PREFIX, THUNDER_PREFIX_LEN) == 0) {
                ret = thunder_decode(url, &tmpurl);
        } else if (strncmp(prefix, FLASHGET_PREFIX, FLASHGET_PREFIX_LEN) == 0) {
                ret = flashget_decode(url, &tmpurl);
        } else if (strncmp(prefix, QQ_PREFIX, QQ_PREFIX_LEN) == 0) {
                ret = qq_decode(url, &tmpurl);
        } else if (strncmp(prefix, RAYFILE_PREFIX, RAYFILE_PREFIX_LEN) == 0) {
                ret = rayfile_decode(url, &tmpurl);
        } else {
                tmpurl = strdup(url);
                ret = SUCCESS;
        }
		
 
        if (ret == SUCCESS) {
                return tmpurl;
        } else {
                if (tmpurl)
                        free(tmpurl);
        }
	MCEMT_DBG("parse_link finish\n");
        return NULL;
}



static int xl_commit(char *url, char *path, int uid, int *fileindex, int num)
{

	MCEMT_DBG("xl_commit start!\n");
	char file_path[ETM_MAX_FILE_PATH_LEN],file_name[ETM_MAX_FILE_NAME_LEN];
	u32 task_id = 0;
	int type = 0;
	int ret = 0;
        int result = 0;
	char strtype[32];
	char newurl[2048];
	memset(file_path, 0, sizeof(file_path));
	memset(file_name, 0, sizeof(file_name));	
	type = srv_ck_url(url, strtype);
        MCEMT_DBG("the type is %d\n", type);
	if(type == 0){
		return PRO_DLD_TASK_URL_TYPE_ERR;
	}
	
        if (type & (DLD_NO_NORMAL_TYPE|DLD_HTTP_TYPE|DLD_FTP_TYPE)) {
                char *tmpurl = NULL;
		if(url == NULL) {
			return FAILURE;
		}
        	if ((tmpurl = parse_link(url)) == NULL) {
			return FAILURE;
        	} else {
			strcpy(newurl, tmpurl);
			free(tmpurl);
        	}	
	}
	strcpy(file_path, path);

        ret=etm_set_task_auto_start(TRUE);
	if(ret!=0)
	{
		MCEMT_DBG("\n etm_set_task_auto_start Error:%d\n",ret);
	}
	if(type == DLD_BT_TYPE){
		int length = 0, result = 0; 
		mcetm_get_bt_seed_info(url, "utf8", &length, &result) ;
			MCEMT_DBG("fileindex: %d\n", num);
			MCEMT_DBG("the length is %d\n", length);
			if(fileindex == NULL){
                        	int array[length];
				memset(array, 0, sizeof(array));
				int i = 0;
                       		for(i=0; i<length; i++){
                                	array[i] = i;
                        }
			array[length] = -1;
                        mc_create_task_bt(url, file_path, array, &task_id, &result);
                        ret = result;
                        
                	}else{        
                        	mc_create_task_bt(url, file_path, fileindex, &task_id, &result);
                        	ret = result;
                	}
        	              
	}else if(type == DLD_ML_TYPE){

                mc_crate_task_emule(url, file_path, file_name, &task_id, &result);
                ret = result;

	}else if(type == DLD_CID_TYPE){
		MCEMT_DBG("enter mc_create_task_tcid \n");
                mc_create_task_tcid(url, file_path, file_name, &task_id, &result);
                ret = result;
	}else{
              
                MCEMT_DBG("enter mc_create_task_url \n");
		mc_create_task_url(newurl, file_path, file_name, &task_id, &result);
                ret = result;
	}


	MCEMT_DBG("xl_commit finish\n");
	return ret;
}



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

static int pro_xl_sinfo_handler(cgi_t *p, cgi_protocol_t *d, sys_outbuf_t *buf)
{
        MCEMT_DBG(" pro_xl_sinfo_handler start\n");
	unsigned int  task_id_array[512];
	int id_array_size = 512,index=0,task_id=0;
	int ret_val = 0;
        task_info_t t;
        task_status_t st[512];
    
        
	id_array_size = 512;
     	memset(task_id_array, 0, sizeof(task_id_array));
	mcetm_get_all_task_id_and_group_id(st, FALSE, &id_array_size, &ret_val);
        int i = 0, j = 0;
        for(i=0;i<id_array_size-1;i++){
                if(st[i].task_state==TS_TASK_RUNNING){
                        task_id_array[j]=st[i].task_id;
			j++;
			MCEMT_DBG("the task_id is %d\n", st[i].task_id);
                } 
        }     

	if (j == 0) {
		return ret_val;
	}
	MCEMT_DBG("the j is %d\n", j);
        MCEMT_DBG("the id_array_size is %d\n", id_array_size);
		
	if(ret_val ==0){
		if(j>512) 
			j=512;
		for(index=0;index<j;index++)
		{
			task_id = task_id_array[index];

      			MCEMT_DBG("the index is %d task_id is  %d\n", index, task_id);
                        get_task_info_by_id(task_id,&t,&ret_val);
			if(ret_val != 0){
				if (t.file_name) {
        				free(t.file_name);
        			}    
        
        			if (t.file_path) {
        				free(t.file_path); 
        			}
				if (t.url) {
					free(t.url);
				} 
				return ret_val;
			}else{
                                MCEMT_DBG("enter xl_task_to_simple_xml\n");
                                xl_task_to_simple_xml(&t, buf);
				if (t.file_name) {
        				free(t.file_name);
        			}    
        
        			if (t.file_path) {
        				free(t.file_path); 
        			}
				if (t.url) {
					free(t.url);
				} 
			}
		}
	}

        MCEMT_DBG(" pro_xl_sinfo_handler finish\n");
	return ret_val;	
}

static int pro_xl_arg_handler(cgi_t *p, cgi_protocol_t *d, sys_outbuf_t *buf)
{

        MCEMT_DBG(" pro_xl_arg_handler start\n");

//	cgi_rec_t *rec;
	int num;
//	unsigned long long lnum;
	char str[128];
//	rec = p->cgirec;

//	num = dld->up_rate(dld);
	num = etm_get_current_upload_speed();
	sprintf(str, "%d", num);
	xml_add_elem(XML_LABEL, "up_rate", str, buf);

//	num = dld->down_rate(dld);
	num = etm_get_current_download_speed();
	sprintf(str, "%d", num);
	xml_add_elem(XML_LABEL, "down_rate", str, buf);
/*
	num = dld->num(dld);
	sprintf(str, "%d", num);
	xml_add_elem(XML_LABEL, "num", str, buf);


	lnum = dld->dsize(dld);
	sprintf(str, "%llu", lnum);
	xml_add_elem(XML_LABEL, "dsize", str, buf);
	
	lnum = dld->csize(dld);
	sprintf(str, "%llu", lnum);
	xml_add_elem(XML_LABEL, "csize", str, buf);

	lnum = dld->asize(dld);
	sprintf(str, "%llu", lnum);
	xml_add_elem(XML_LABEL, "asize", str, buf);

*/
        MCEMT_DBG(" pro_xl_arg_handler finish\n");
	return 0;
}

static int pro_xl_record_handler(cgi_t *p, cgi_protocol_t *d, sys_outbuf_t *buf)
{
	
        MCEMT_DBG(" pro_xl_record_handler start !\n");
	cgi_rec_t *rec;
	rec = p->cgirec;
	char *name;
	int z;
	int ret = 0;

	z = 1;
	do {
		name = rec->get_val_no(rec, PRO_XL_RECORD_LIST, z);
		if(name == NULL)
			break;
		if(strcmp(name,"run") == 0){
			ret = xl_get_list_task(ETS_TASK_RUNNING, buf);
			ret = xl_get_list_task(ETS_TASK_WAITING, buf);
			ret = xl_get_list_task(ETS_TASK_PAUSED, buf);
		}else if(strcmp(name,"finish") == 0){
			ret = xl_get_list_task(ETS_TASK_SUCCESS, buf);
		}else if(strcmp(name,"trash") == 0){
		}else if(strcmp(name,"id") == 0){
			int i,j=0;
			char  *s;
			i = 1;
			do {
				j = 0;
				s = rec->get_val_no(rec, PRO_XL_RECORD_ID, i);
				if (s != NULL) {
					task_info_t task_info;                             
                                        int result = 0;
                                        get_task_info_by_id(atoi(s), &task_info, &result);
					if(result == 0){
                                        MCEMT_DBG("the task_info.file_size is %llu !\n", task_info.file_size);
                                        MCEMT_DBG("##########################################################"
                                                  "###########################################################"
                                                  "############################################################"
                                                  "#############################################################\n");
                                        
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
					} else {
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
				i++;
			} while (s);
		}
		z++;
	}while(name);
	
        MCEMT_DBG("pro_xl_record_handler finish\n");
	return 0;
}

static int pro_xl_tasks_handler(cgi_t *p, cgi_protocol_t *d, sys_outbuf_t *buf)
{
        MCEMT_DBG("pro_xl_tasks_handler start\n");

	cgi_rec_t *rec;
	int i;
	char *s, *action;
	rec = p->cgirec;
	int result = 0;
	if ((action= rec->get_val(rec, PRO_XL_TASKS_ACTION)) == NULL ||
		action[0] == '\0') {
		return PRO_BASE_ARG_ERR;
	}

	i = 1;
	if(!strcmp(action, "run")){
		do{ 
			s = rec->get_val_no(rec, PRO_XL_TASKS_ID, i);
			if (s != NULL) {
                                mcetm_start_task(atoi(s), &result);
				i++;
			}
		}while(s);                        
	}else if(!strcmp(action, "stop")){
               do{ 
                	s = rec->get_val_no(rec, PRO_XL_TASKS_ID, i);
			if(s != NULL) {
                        	mcetm_stop_task(atoi(s), &result);
                                i++;
                        }
                }while(s);
		MCEMT_DBG("##########################\n");
		MCEMT_DBG("the i is %d\n", i);
		MCEMT_DBG("##########################\n");
	}else if(!strcmp(action, "delete")){
                do{
                	s = rec->get_val_no(rec, PRO_XL_TASKS_ID, i);
			if (s != NULL) {
                        	mcetm_delete_task(atoi(s), FALSE, &result);
                               	i++;
                        }
                  }while(s);
        }else if(!strcmp(action, "deleteall")){
                do{
                        s = rec->get_val_no(rec, PRO_XL_TASKS_ID, i);
			if (s != NULL) {
                        	mcetm_delete_task(atoi(s), TRUE, &result);
                               	i++;
                        }
                  }while(s);       
	}else if(!strcmp(action, "green")){
		s = 0;
        	mcetm_use_green_channel(atoi(s), &result);
	} 

        MCEMT_DBG("pro_xl_tasks_handler finish\n");
        

	return 0;
}

static int url_valid(const char *url)
{
	MCEMT_DBG("url_valid start\n");
	if (strstr(url, "//"))
		return SUCCESS;
	else if (strstr(url, "/data/")) {
		return SUCCESS;
	} else if (strstr(url, ".torrent")) {
		return SUCCESS;
	} else if (strstr(url, ".TORRENT")) {
		return SUCCESS;
	}else
		return FAILURE;
}

static int pro_xl_url_handler(cgi_t *p, cgi_protocol_t *d, sys_outbuf_t *buf)
{
	MCEMT_DBG("pro_xl_url_handler start!\n");
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
	if (cgi->flag & CGI_BUF_OVERFLOW) {
		urllist = cgi->cgi_read_fvar(cgi, PRO_XL_URL);
		if ((urllist == NULL) || (urllist[0] == '\0')) {
			return PRO_DLD_TASK_URL_ERR;
		}
		overflow = 1;
	} else
		urllist = rec->get_val(rec, PRO_XL_URL);

	if (urllist == NULL) {
		MCEMT_DBG(" urlist is NULL\n");
	}
	/* path            */
	path = rec->get_val(rec, PRO_XL_URL_PATH);
	char *tmp = NULL;
	tmp = path + 24;	
	sprintf(path, "C:/%s", tmp);
	MCEMT_DBG("the path is :%s\n", path );
	
	if ((path == NULL) || (path[0] == '\0'))
		return PRO_DLD_TASK_URL_ERR;

	for(i=1,j=0;j<255;i++,j++){
		sindex = rec->get_val_no(rec, PRO_XL_URL_INDEX, i);
		if (sindex != NULL) {
			iindex[j] = atoi(sindex);
		}else
			break;
	}

	if(j != 0){
		fileindex = calloc(j+1,sizeof(int));
		memcpy(fileindex, iindex, j*sizeof(int));
		fileindex[j] = -1;
	}
	MCEMT_DBG("the j is %d", j);
	/* Get url list    */
	if (urllist == 0) {
		MCEMT_DBG("urllist is NULL\n"); 
	}	
	url = urllist;
	while (*url != '\0') {
	MCEMT_DBG("the num is %d\n", num);	
		if(num == 200)
			break;
		end = strchr(url, '\r');
		if (end)
			*end = '\0';
		if (url_valid(url)) {
			char *torrent;
			char torrenturl[256];
			strcpy(torrenturl, url);
			if((torrent = strrchr(url, '.')) != NULL){
				if ((strcmp(torrent, ".torrent") == 0) || (strcmp(torrent, ".TORRENT") == 0)) {
					sprintf(torrenturl, "C:/.vst/.torrent/%s", url);
				}
			}
			MCEMT_DBG("enter xl_commit!\n");
			xl_commit(torrenturl, path, uid, fileindex, j);
			num++;
		}
		if (end == NULL)
			break;
		else {
			url = end + 1;
			/* Skip '\r','\n',' ', '\t' */
			while (*url != '\0') {
				if ((*url == '\r') || (*url == '\n') ||
				    (*url == ' ') || (*url == '\t'))
					url++;
				else /* Other code, break */
					break;
			}
		}
	}

	/* Free the list   */

	MCEMT_DBG("pro_xl_url_handler finish \n");
	if (overflow && urllist)
		free(urllist);
	if (fileindex)
		free(fileindex);

	return 0;
}

static int pro_xl_system_handler(cgi_t *p, cgi_protocol_t *d, sys_outbuf_t *buf)
{	
	MCEMT_DBG("pro_xl_system_handler start\n");
	system_info_t t;
	
	int retry_time = 0;	
	while (retry_time < 20 ) {
                usleep(1000*1000);
                get_system_info(&t);
                MCEMT_DBG("\n get_system_info times is %d\n", retry_time);                      
                if (t.is_bind_ok == 1) {
                        printf("####################################\n");
                        printf("YOUR BIND USRNAME is %s\n",t.username);
                        printf("####################################\n");
        		xml_add_elem(XML_LABEL, "username", t.username, buf);
	                if (t.license) {
                                free(t.license);
                        }
                        if (t.bind_acktive_key) {
                                free(t.bind_acktive_key);
                        }
                        if (t.peerid) {
                                free(t.peerid);
                        }
                        if (t.userid) {
                                free(t.userid);
                        }
                        if (t.username) {
                                free(t.username);
                        }
 
                        break;
                } else  if (strlen(t.bind_acktive_key) == 0) {
                        MCEMT_DBG("\nget the remote control key is failed\n");
                        retry_time++;
                        if (t.license) {
                                free(t.license);
                        }
                        if (t.bind_acktive_key) {
                                free(t.bind_acktive_key);
                        }       
                        if (t.peerid) {
                                free(t.peerid);
                        }       
                        if (t.userid) {
                                free(t.userid);
                        }
                        if (t.username) {
                                free(t.username);
                        }                       
                
                } else {
                        MCEMT_DBG("the remote control key is %s\n", t.bind_acktive_key);
                        xml_add_elem(XML_LABEL, "bindkey", t.bind_acktive_key, buf);
                        if (t.license) {
                                free(t.license);
                        }
                        if (t.bind_acktive_key) {
                                free(t.bind_acktive_key);
                        }
                        if (t.peerid) {
                                free(t.peerid);
			}
			if (t.userid) {
                                free(t.userid);
                        }
                        if (t.username) {
                                free(t.username);
                        }  
			break;   
 		}
	
        }                                      
	MCEMT_DBG("pro_xl_system_handler finish\n");
	return 0;	
}

static int pro_xl_base_handler(cgi_t *p, cgi_protocol_t *d, sys_outbuf_t *buf)
{
        MCEMT_DBG("pro_dld_base_handler\n");
        cgi_rec_t *rec;
	int interval ;
	char str[128];
	
        if((d->flag&PROTOCOL_FUCTION) == 0){
		/*get msg*/
		interval = 10 ;
		sprintf(str, "%d", interval);
		xml_add_elem(XML_LABEL, "interval", str, buf);

		interval = etm_get_download_limit_speed();
		sprintf(str, "%d", interval);
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
                        etm_set_download_limit_speed(idspeed);
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
        return 0;
        
        
}  


static int pro_dld_xl_handler(cgi_t *p, cgi_protocol_t *d, sys_outbuf_t *buf)
{

        MCEMT_DBG("#############################\n");
        MCEMT_DBG("#############################\n");
        MCEMT_DBG("start pro_dld_xl_handler\n");
        MCEMT_DBG("#############################\n");
        MCEMT_DBG("#############################\n");
	int status,num;
        u32 dspeed;
        u32 uspeed;
        int ret_val;
	char str[128];
   
        cgi_rec_t *rec;
	rec = p->cgirec;
        MCEMT_DBG()
	if((d->flag&PROTOCOL_FUCTION) == 0){
                MCEMT_DBG("#############################\n");
                MCEMT_DBG("#############################\n");
                MCEMT_DBG("start pro_dld_xl_handler get\n");
                MCEMT_DBG("#############################\n");
                MCEMT_DBG("#############################\n");
		/*get msg*/
		xml_add_elem(XML_LABEL, "support", "1", buf);
                status = 1;
		sprintf(str, "%d", status);
		xml_add_elem(XML_LABEL, "status", str, buf);
		if(status){
			num = etm_get_max_tasks();                       
			sprintf(str, "%d", num);
			xml_add_elem(XML_LABEL, "num", str, buf);

			num = etm_get_max_task_connection();
			sprintf(str, "%d", num);
			xml_add_elem(XML_LABEL, "connect", str, buf);

			uspeed = etm_get_upload_limit_speed();
			sprintf(str, "%d", num);
			xml_add_elem(XML_LABEL, "uprate", str, buf);

			dspeed = etm_get_download_limit_speed();
			sprintf(str, "%d", num);
			xml_add_elem(XML_LABEL, "downrate", str, buf);
		}

	}else{
	        MCEMT_DBG("#############################\n");
                MCEMT_DBG("#############################\n");
                MCEMT_DBG("start pro_dld_xl_set\n");
                MCEMT_DBG("#############################\n");
                MCEMT_DBG("#############################\n");
		/*set msg*/
		char *name = NULL;           
		int icount;
		if ((name = rec->get_val(rec, PRO_DLD_XL_STATUS)) != NULL) {
			icount = atoi(name);
			/*1->start 2->stop*/
			if(icount == PRO_SERV_ACTION_START){
			         ret_val =  mc_etm_init();
		                if(ret_val == 0)
			                MCEMT_DBG("etm init failure\n");			            		                  
                        }else if(icount == PRO_SERV_ACTION_STOP){
                               xl_stop_etm();
			}else
				return PRO_BASE_ARG_ERR;                
		}
                
		if ((name = rec->get_val(rec, PRO_DLD_XL_NUM)) != NULL) {
			icount = atoi(name);
			if((icount < 1) || (icount > 10))
				return PRO_BASE_ARG_ERR;
                        etm_set_max_tasks(icount);
		}

		if ((name = rec->get_val(rec, PRO_DLD_XL_CONNECT)) != NULL) {
			int port_len,j;
                        port_len = strlen(name);
                        for(j=0;j<port_len;j++){
                                if(!isdigit(name[j]))
                                        return PRO_BASE_ARG_ERR;
                        }
						
			icount = atoi(name);
			if((icount < 1) || (icount > 100))
				return PRO_BASE_ARG_ERR;
                        etm_set_max_task_connection(icount);
                        
		}

		if ((name = rec->get_val(rec, PRO_DLD_XL_UPRATE)) != NULL) {
			icount = atoi(name);
			if((icount < 1) || (icount > 2048))
				return PRO_BASE_ARG_ERR;
                        etm_set_upload_limit_speed(icount);
		}

		if ((name = rec->get_val(rec, PRO_DLD_XL_DOWNRATE)) != NULL) {
			icount = atoi(name);
			if((icount < 1) || (icount > 2048))
				return PRO_BASE_ARG_ERR;
                        etm_set_download_limit_speed(icount);
		}
           
		{
			FILE *fp;
			fp = popen("/etc/init.d/etcsync", "w");
			if(fp != NULL)
				pclose(fp);
		}

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
        MCEMT_DBG("getpid is %ld\n", getpid()); 
	MCEMT_DBG("xl_monitor start\n");
	int ret_val =0;
	while(1){
                printf("here\n");
		ret_val = mc_etm_init();
		if(ret_val == 1){
			printf("ret_val == 1\n");
			break;
		}
	
		sleep(60);
	}

	while(1){
		sleep(10);
		printf("xl monitor\n");
	}	

	xl_stop_etm();

	MCEMT_DBG("xl_monitor finish\n");
	return NULL;
}

