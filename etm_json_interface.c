/******************************************************************************
 *  * Copyright (C) 2012
 *  * 
 *  * File: etm_json_interface.c
 *  *
 *  * Date: 2012-03-26
 *  *
 *  * Author: Hulong, <hulong@birdsongsoft.com>
 *  *
 *  * Descriptor:
 *  *   
 *  * Note:
 *  *  XunLei etm library json interface
 *  *
 *  * Version: 0.1
 *  *
 *  * Modified:
 *  *
 *******************************************************************************/
/*-----------------------------------------------------------------------------
  System header files 
------------------------------------------------------------------------------*/
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <string.h> 
#include <memory.h>
#include <malloc.h>
#include <dirent.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>




/*-----------------------------------------------------------------------------
  Local header files
------------------------------------------------------------------------------*/
#include "json.h"
#include "common.h"
#include "etm_json_interface.h"
#include "et_task_manager.h"

/*-----------------------------------------------------------------------------
  const
------------------------------------------------------------------------------*/
#define LOCAL_PATH "/data/HardDisk1/Volume1/.vst/./tddownload/"
#define ETM_SYSTEM_PATH "/data/HardDisk1/Volume1/.vst/./etm_system/"
#define DEFAULT_DOWNLOAD_PATH "/data/HardDisk1/Volume1/.vst/./tddownload/"
 
/*-----------------------------------------------------------------------------
  struct
------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
  global
------------------------------------------------------------------------------*/

int mc_rt_failed = FALSE;
static Bool g_state_changed_lock = FALSE;

static unsigned int g_state_changed_tasks[32];
const static  char* g_task_state_name[6] = 
{ 
        "WAITING", 
        "RUNNING", 
        "PAUSED", 
        "SUCCESS", 
        "FAILED", 
        "DELETED"
};


/*-----------------------------------------------------------------------------
  local functions
------------------------------------------------------------------------------*/

static char vs_cfg_buf[VSINFO_READ_MAX];

static int flash_read(const char *mtddev, int start, char *buf, int buflen)
{
        int fd = -1;
 
        if((fd = open(mtddev, O_RDONLY)) == -1){
                return -1;
        }
        /* Write the mtdblock device  */
        if(lseek(fd, start, SEEK_SET) == -1){
                close(fd);
                return -1;
        }
        /* Read data from mtdblock    */
        if(read_n(fd, buf, buflen - 1) != (buflen - 1)){
                close(fd);
                return -1;
        }else{
                buf[buflen - 1] = '\0'; /* prevent over-read */
        }
#if 0
        str_dbgdump(buf, buflen);
#endif
        close(fd);
 
        return 0;
}
static char *vs_cfg_read_vsinfo(void)
{
        memset(vs_cfg_buf, 0, sizeof(vs_cfg_buf));
 
        if(!flash_read(VS_CFG_MTD, 0, vs_cfg_buf, sizeof(vs_cfg_buf)))
                return NULL;
 
        return vs_cfg_buf;
}

int vs_cfg_init(void)
{       
        /* Read information */
        vs_cfg_read_vsinfo();
        
        return 1;
}

static char *get_license(char *license)
{
        char vs_cfg_buf[VSINFO_READ_MAX];
 
        memset(vs_cfg_buf, 0, sizeof(vs_cfg_buf));
        if(flash_read(VS_CFG_MTD, 0, vs_cfg_buf, sizeof(vs_cfg_buf)))
                return NULL;
        vsinfo_t *vsinfo = (vsinfo_t *)vs_cfg_buf;
        vscominfo_t *cinfo = &vsinfo->cominfo;
 
        license = strdup(cinfo->xunlei.str);
 
        return license;
}

int add_state_changed_task(unsigned int  task_id)
{
	int i=0;
	
	while(g_state_changed_lock)
	{
		usleep(10);
		if(i++>=5)
		{
			printf("\n*******add_state_changed_task crash!!!**********\n");
			return -1;
		}
	}
	
	g_state_changed_lock=TRUE;
	
	i=0;
	while((i<32)&&(g_state_changed_tasks[i]!=0)&&(g_state_changed_tasks[i]!=task_id))
		i++;
	
	if(i==32)
	{
		g_state_changed_lock=FALSE;
		printf("\n*******add_state_changed_task TOO_MANY_RUNNING_TASKS!!!**********\n");
		return -1;
	}
	g_state_changed_tasks[i] = task_id;
	
	g_state_changed_lock=FALSE;
	return 0;
}


int32 task_change_state_call_back(u32 task_id, ETM_TASK_INFO * p_task_info)
{
	if (task_id == 0)
	{
		printf("\n\n+++++++++++++++++++++++++++++++++++\n");
		printf("	task auto stared!");
		printf("+++++++++++++++++++++++++++++++++++\n\n");
		return 0;
	}
	
	add_state_changed_task(task_id);
	g_state_changed_lock = TRUE;
	printf("\n\n+++++++++++++++++++++++++++++++++++\n");
	printf("	task:%u change state to :%s !\n",task_id,g_task_state_name[p_task_info->_state]);
	printf("+++++++++++++++++++++++++++++++++++\n\n");
	return 0;
}



int mc_start_etm(void)
{
      	int ret_val =0;
	
        ret_val = etm_init(ETM_SYSTEM_PATH,strlen(ETM_SYSTEM_PATH));
	if(ret_val != 0)
	{
		MCEMT_DBG("\netm_init Error:%d\n",ret_val);
		return ret_val;
	}
       
	ret_val=etm_set_default_encoding_mode(EEM_ENCODING_UTF8);
        if(ret_val!=0) {
                MCEMT_DBG("etm_set_default_encoding_mode Error:%d\n",ret_val);
                goto ErrHandler;
        }
	 
	ret_val=etm_set_download_path(DEFAULT_DOWNLOAD_PATH,strlen(DEFAULT_DOWNLOAD_PATH));
        if(ret_val!=0) {
                MCEMT_DBG("etm_set_download_path Error:%d\n",ret_val);
//                goto ErrHandler;
        }
	ret_val=etm_set_task_state_changed_callback(task_change_state_call_back);
       	if(ret_val!=0) {
                MCEMT_DBG("etm_set_task_state_changed_callback Error:%d\n",ret_val);
                goto ErrHandler;
        }

	ret_val=etm_set_vod_cache_path(VOD_CACHE_PATH,strlen(VOD_CACHE_PATH));
        if(ret_val!=0) {
                MCEMT_DBG("etm_set_vod_cache_path Error:%d\n",ret_val);
                goto ErrHandler;
        }

	ret_val=etm_set_vod_cache_size(VOD_CACHE_SIZE);
        if(ret_val!=0) {
                MCEMT_DBG("etm_set_vod_cache_size Error:%d\n",ret_val);
                goto ErrHandler;
        }
	
	ret_val=etm_set_vod_buffer_size(VOD_BUFFER_SIZE);
        if(ret_val!=0) {
                MCEMT_DBG("etm_set_vod_buffer_size Error:%d\n",ret_val);
                goto ErrHandler;
        }

	ret_val=etm_set_task_auto_start(TRUE);
        if(ret_val!=0) {
                MCEMT_DBG("\n etm_set_task_auto_start Error:%d\n",ret_val);
                goto ErrHandler;
        }

        char *license = NULL;
        
	vs_cfg_init();  
        MCEMT_DBG("get_license enter\n");
        license = get_license(license);
        MCEMT_DBG("license is %s\n", license);  
        if (license == NULL) {
                MCEMT_DBG("GET LICENSE FAILURE!\n");
                goto ErrHandler;
        }
        
        char *g_key = NULL;
        g_key = strdup(license);
                
        if (license != NULL){
                free(license);
        }		
	ret_val=etm_set_license(g_key, strlen(g_key));
        MCEMT_DBG("the ret_val is %d\n", ret_val);
        if (g_key != NULL) {
                free(g_key);
        }
        if(ret_val != 0) {
                MCEMT_DBG("etm_set_license Error:%d\n",ret_val);
                if(ret_val!=1925)
                {
                        goto ErrHandler;
                }
        }
        return 0 ;

	
ErrHandler:
        MCEMT_DBG("etm_uninit...byebye!\n");
        
   
	etm_uninit();
 
        return ret_val;
	
}


/* Purpose: Create the directory for etm lib
 *
 * In     : void
 *
 * Return : SUCCESS  1
 *          FAILURE  0
 */
static int mc_create_directory(void)
{
        struct stat stat_buf;

        /* Check if the system directory is exist */
        memset(&stat_buf,0,sizeof(stat_buf));
        if ((lstat(ETM_SYSTEM_PATH, &stat_buf)==0)&&
             (S_ISDIR(stat_buf.st_mode)))
        {
                MCEMT_DBG("Directory :%s is found!\n", 
                          ETM_SYSTEM_PATH);  
        } else {   
                MCEMT_DBG("Directory :%s is not found,need be created!\n",
                          ETM_SYSTEM_PATH);  
                /* Create the  directory */
                mkdir(ETM_SYSTEM_PATH,0700);
        }

        /* Chek if the download directory is exist */
        memset(&stat_buf,0,sizeof(stat_buf));
        if((lstat(DEFAULT_DOWNLOAD_PATH, &stat_buf)==0)&&
            (S_ISDIR(stat_buf.st_mode)))
        {
                MCEMT_DBG("Directory :%s is found!\n",
                        DEFAULT_DOWNLOAD_PATH); 
        } else {   
                MCEMT_DBG("Directory :%s is not found,need be created!\n",
                        DEFAULT_DOWNLOAD_PATH);  
                /* Create the  directory */
                mkdir(DEFAULT_DOWNLOAD_PATH,0700);
        }

        /* Chek if the vod cache directory is exist */
        memset(&stat_buf,0,sizeof(stat_buf));
        if((lstat(VOD_CACHE_PATH, &stat_buf)==0)&&(S_ISDIR(stat_buf.st_mode)))
        {
                MCEMT_DBG("Directory :%s is found!\n",VOD_CACHE_PATH);  
        } else  {   
                MCEMT_DBG("Directory :%s is not found,need be created!\n",
                         VOD_CACHE_PATH);  
                /* Create the  directory */
                mkdir(VOD_CACHE_PATH,0700);
        }
        
        return 1;
}

/* Purpose: callback function for remote control message 
 *
 * In     : *ver: the version
 *
 * Return : SUCCESS  1
 *          FAILURE  0
 */
static int mc_remote_control_msg(u32 msg_id, char *msg, u32 msg_len)
{
        MCEMT_DBG("remote_control_msg:msg_id:%d, msg:%s\n", msg_id, msg);
        if(msg_id == 2) 
                mc_rt_failed = TRUE;
        
        return 0;
}
int mc_etm_init()
{
        ETM_RC_PATH path;
        int ret_val = 0;
        int retry_time = 0;
	u32 license_result = 0;
	system_info_t t;
        if (!mc_create_directory()) {
                MCEMT_DBG("Fail to create_directory()\n");
                return 0;
        }

        /* Start ETM lib       */
        ret_val = mc_start_etm();
        if(ret_val != 0)
        {
                MCEMT_DBG("\nstart etm Error:%d\n",ret_val);
                return 0;
        }
        MCEMT_DBG("\nstart etm SUCCESS!\n");
        /* Connect the remote server for remoting control */
        memcpy(path._data_path, LOCAL_PATH, ETM_MAX_FILE_PATH_LEN );
	MCEMT_DBG("\nmemcpy is finished!\n");
        do
        {
                usleep(1000 * 1000);
                ret_val = etm_remote_control_start( &path, 1, mc_remote_control_msg);
                MCEMT_DBG("\etm_remote_control_start ret:%d \n",ret_val);
                retry_time++;
        } while(ret_val != 0 && retry_time < 3);

	if (ret_val != 0) {
		MCEMT_DBG("Fail to etm_remote_control_start\n"); 
		return 1;
	}
	MCEMT_DBG("\nConnect remote server SUCCESS!\n");
	/* Get the remote control key */
	retry_time = 0;
	while (retry_time < 20 ) {
		usleep(1000*1000);
		get_system_info(&t);
		MCEMT_DBG("\n get_system_info times is %d\n", retry_time);			
		if (t.is_bind_ok == 1) {
			MCEMT_DBG("####################################\n");
		 	MCEMT_DBG("YOUR BIND USRNAME is %s\n",t.username);
			MCEMT_DBG("####################################\n");
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
		} else 	if (strlen(t.bind_acktive_key) == 0) {
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
			printf("the remote control key is %s\n", t.bind_acktive_key);
			
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
	
	/* Get the license result */
	retry_time = 0;
	while (retry_time < 10) {
		MCEMT_DBG("\n get_license times is %d\n", retry_time);
        	ret_val = etm_get_license_result(&license_result);
        	if(ret_val != 0) {
                	MCEMT_DBG("\netm_get_license_result Error:%d,license_result=%u \n",
			ret_val, license_result);
        	} else {
                	MCEMT_DBG("\netm_get_license_result ok:license_result=%u \n",license_result);
			break;
        	}
		usleep(1000 * 1000);
		retry_time++;
	}

	if (retry_time > 30) {
		return 0;
	} else {
		return 1;
	}
}
static int mc_json_interface(const char *p_input, int btflag, char **p_output)
{
	
	int ret_val = 0;
	int ret_try = 1;
	u32 buffer_len;
 
   	MCEMT_DBG("input:%s, len=%d\n", p_input, strlen(p_input));
	if (btflag == MC_BTFLAG_FALSE)
                buffer_len = MC_JSON_BUFFER_LEN;
        else if(btflag == MC_BTFLAG_TRUE)
                buffer_len = MC_JSON_BT_BUFFER_LEN;
        else 
                buffer_len = 128;
        while (ret_try < 10) {
                MCEMT_DBG("#####################\n");
                buffer_len *= ret_try;
                MCEMT_DBG("start calloc\n");
                *p_output= (char*)calloc(buffer_len, sizeof(char));
                if (*p_output == NULL) {
                        MCEMT_DBG("calloc failure\n");
                        continue;
                }
                MCEMT_DBG("####################\n");
                ret_val = etm_mc_json_call(p_input, *p_output, &buffer_len);
		MCEMT_DBG("the ret_val is %d\n",ret_val);
		if (ret_val == 0) {
                        break;
                } else if (ret_val == 105476) {
                        free(*p_output);
                        *p_output = NULL;
                        ret_try++;
			MCEMT_DBG("the ret_try is %d\n", ret_try);
                        continue;
                }else{
		        break;
                }
        }
	if (ret_val == 0)
		return 1; 
	else 
		return 0;	

	
}

/* Purpose: Create the url task
 *
 * In     : *file_path:
 *          *file_name: 
 *
 * Return : 0: FAILURE
 *          1: SUCCESS
 */
int mc_create_task_url(char *url, char *file_path, char *file_name, u32 *task_id, int *result)
{
        char *p_create_task_string = NULL;
        char *p_output = NULL;
        struct json_object *args = NULL;
        struct json_object *obj1 = NULL, *obj2 = NULL;
        int ret_val = 0;

	
        if (url == NULL || file_path == NULL ) {
                MCEMT_DBG("url is NULL\n");
                return 0;
        }
        if(file_name == NULL) {
		p_create_task_string = (char*)calloc(strlen(url) + strlen(file_path) + 128,sizeof(char));
       		if (p_create_task_string  == NULL) { 
                MCEMT_DBG("Fail to calloc p_create_task_string\n");
                return 0;
        }
                sprintf(p_create_task_string,"{\"23\":[0,\"%s\",\"\",\"%s\",\"\",\"\",[],\"\",0,\"\",0,0,\"\"]}",
                        file_path, url);
        } else {
		p_create_task_string = (char*)calloc(strlen(url) + strlen(file_path) + strlen(file_name) + 128,sizeof(char));
        	if (p_create_task_string  == NULL){ 
        	MCEMT_DBG("Fail to calloc p_create_task_string\n");
                return 0;
        }
                sprintf(p_create_task_string,"{\"23\":[0,\"%s\",\"%s\",\"%s\",\"\",\"\",[],\"\",0,\"\",0,0,\"\"]}",
                        file_path, file_name,url);
        }
	
        if (!mc_json_interface(p_create_task_string, MC_BTFLAG_FALSE, &p_output)) {
	         MCEMT_DBG("Fail to mc_json_interface\n");
                 goto MC_CREATE_ERR;
        }        

	args = json_tokener_parse(p_output);
	if(is_error(args)){
                MCEMT_DBG("json_tokener_parse failure!\n");
                goto MC_CREATE_ERR;
        }
	
	if(!json_object_is_type(args, json_type_array)){
                 MCEMT_DBG("json_object_is_type failure!\n");
                 goto MC_CREATE_ERR;

	}
	obj1 = json_object_array_get_idx(args, 0);

	if(json_object_is_type(obj1, json_type_int)){
		*result = json_object_get_int(obj1);
		MCEMT_DBG("the result is %d!\n", *result);
	}else{
		MCEMT_DBG("json_object_is_type failure!\n");
        }
	obj2 = json_object_array_get_idx(args, 1);
        if (obj2 == NULL) {
		 MCEMT_DBG("json_object_array_get_idx failure!\n");
		 goto MC_CREATE_ERR;		
      	}
	
	if (json_object_is_type(obj2, json_type_int)) {
		*task_id = json_object_get_int(obj2);
                MCEMT_DBG("the task_id is %d\n", *task_id);
        } else { 
		MCEMT_DBG("json_object_get_int is failure\n");
		goto MC_CREATE_ERR;
	}
	
	if(result == 0) 
                ret_val = 1;
MC_CREATE_ERR:
        if (args) {
                json_object_put(args);
        }
        if (p_create_task_string) {
                free(p_create_task_string);
        }
        if (p_output) {
                free(p_output);
        }
        return ret_val;

}

int mcetm_get_bt_seed_info(char *seed_file_path, char *encoding_mode, int *length, int *result)
{
	char *p_get_bt_seed_info  = NULL;
	char *p_output = NULL;
	struct json_object *args = NULL, *obj1 = NULL, *obj2 = NULL;
//	char *title_name = NULL;
	int total_size = 0;
	int ret_val = 0;
	if (seed_file_path == NULL || encoding_mode  == NULL) {
		MCEMT_DBG("url == NULL or file_path == NULL or file_name is NULL\n");
                return 0;
        }

	p_get_bt_seed_info = (char*)calloc(strlen(seed_file_path)+strlen(encoding_mode) +48 , sizeof(char));
	if (p_get_bt_seed_info == NULL) {  
		MCEMT_DBG("Fail to calloc p_get_bt_seed_info\n");
       		return 0;
        } 
	
	sprintf(p_get_bt_seed_info, "{\"22\":[\"%s\",\"%s\"]}", seed_file_path, encoding_mode);  
	
	if (!mc_json_interface(p_get_bt_seed_info, MC_BTFLAG_FALSE, &p_output)) {
                 MCEMT_DBG("Fail to mc_json_interface\n");
                 goto MC_CREATE_ERR;
        }        

        args = json_tokener_parse(p_output);
        if(is_error(args)){
                MCEMT_DBG("json_tokener_parse failure!\n");
                goto MC_CREATE_ERR;
        }
        
        if(!json_object_is_type(args, json_type_array)){
                 MCEMT_DBG("json_object_is_type failure!\n");
                 goto MC_CREATE_ERR;
        }	

	 obj1 = json_object_array_get_idx(args, 0);

        if(json_object_is_type(obj1, json_type_int)){
                *result = json_object_get_int(obj1);
                MCEMT_DBG("the result is %d!\n", *result);
        } else {
                MCEMT_DBG("json_object_is_type failure!\n");
        }
        obj2 = json_object_array_get_idx(args, 1);
        if (obj2 == NULL) {
                 MCEMT_DBG("json_object_array_get_idx failure!\n");
                 goto MC_CREATE_ERR;            
        }
	
	/*
	if (json_object_is_type(obj2, json_type_string)) {
                title_name = (void*)calloc(256,sizeof(char));
                strcpy(title_name, json_object_get_string(obj2));
	}
	*/
	obj2 = json_object_array_get_idx(args, 2);
	if (obj2 == NULL) {
                 MCEMT_DBG("json_object_array_get_idx failure!\n");
                 goto MC_CREATE_ERR;
        }

	if (json_object_is_type(obj2, json_type_string)) {
 		total_size = json_object_get_int(obj2);
	}
	
	obj2 = json_object_array_get_idx(args, 3);
	if(!json_object_is_type(obj2, json_type_array)){
        	 MCEMT_DBG("json_object_is_type failure!\n");
                 goto MC_CREATE_ERR;
        }       
	
	
	*length = json_object_array_length(obj2);
	/*

	int i = 0;
	for (i=0; i < *length; i++) {
		obj3 = json_object_array_get_idx(obj2, i);
			t[i].subid = json_object_get_int(json_object_array_get_idx(obj3, 0));
			t[i].subname = calloc(512, sizeof(char));
			if (t[i].subname != NULL) {	
			strcpy(t[i].subname, json_object_get_string(json_object_array_get_idx(obj3, 1)));	
			}
			t[i].sub_file_path = calloc(512, sizeof(char));
			if (t[i].sub_file_path != NULL) {
			strcpy(t[i].sub_file_path, json_object_get_string(json_object_array_get_idx(obj3, 2)));
			} 
			t[i].subfilesize = json_object_get_int(json_object_array_get_idx(obj3, 3));
	}
*/
	if (*result == 0)
		ret_val = 1;


MC_CREATE_ERR: 
	if (args) {
                json_object_put(args);
        }
        if (p_get_bt_seed_info) {
                free(p_get_bt_seed_info);
        }
        if (p_output) {
                free(p_output);
        }

        return ret_val;
	
}


/* Purpose: Create the bt task
 *
 * In     : *file_path:
 *          *file_name: 
             selected_id[]:
            
 *
 * Return : 0: FAILURE
 *          1: SUCCESS
 */

int mc_create_task_bt(char *seed_file_path, char *file_path, int *selected_id,u32 *task_id, int *result)
{
        MCEMT_DBG("mc_create_task_bt start\n");
        int i = 0, num = 0;
        char *p_create_task_string = NULL;
        char *p_output = NULL;
        char tstr[16];
        struct json_object *args = NULL;
	struct json_object *obj1 = NULL, *obj2 = NULL;
        int ret_val = 0;

    

        if (seed_file_path == NULL || file_path == NULL || selected_id == NULL) {
                MCEMT_DBG("url == NULL or file_path == NULL or file_name is NULL\n");
                return 0;
        }
        
       	p_create_task_string = (char*)calloc(strlen(file_path) + strlen(seed_file_path) + 128,sizeof(char));
        if (p_create_task_string == NULL) {
                MCEMT_DBG("Fail to calloc p_create_task_string\n");
                return 0;
        }
	
        sprintf(p_create_task_string,"{\"23\":[1,\"%s\",\"\",\"\",\"\",\"%s\",[", 
        file_path, seed_file_path);
        i = 0;
        while (i < 512 && selected_id[i] != -1) {
                num++;
                i++;
        }
        i = 0;
        while (i < num - 1) {
                memset(tstr, 0, sizeof(tstr));
                sprintf(tstr, "%d,", selected_id[i]);
                strcat(p_create_task_string, tstr);
                i++;
        }
        memset(tstr, 0, sizeof(tstr));
        sprintf(tstr, "%d", selected_id[i]);
        strcat(p_create_task_string, tstr); 
        
        strcat(p_create_task_string, "], \"\",0,\"\",0,0,\"\"]}");
        MCEMT_DBG("Buf: %s\n", p_create_task_string);


	if (!mc_json_interface(p_create_task_string, -1, &p_output)) {

	        MCEMT_DBG("Fail to mc_json_interface\n");
                goto MC_CREATE_ERR;
        }        
                MCEMT_DBG("#########################\n")
		args = json_tokener_parse(p_output);
        if(is_error(args)){
                MCEMT_DBG("json_tokener_parse failure!\n");
                goto MC_CREATE_ERR;
        }
        if (!json_object_is_type(args, json_type_array)){
                MCEMT_DBG("json_object_is_type failure!\n");
                goto MC_CREATE_ERR;
        }

        obj1 = json_object_array_get_idx(args, 0);
        if (obj1 == NULL) {
                MCEMT_DBG("json_object_array_get_idx failed!\n");
                goto MC_CREATE_ERR;
        }
                        
        if (json_object_is_type(obj1, json_type_int)){
                *result = json_object_get_int(obj1);
        }else{
                MCEMT_DBG("json_object_is_type failure!\n");
                goto MC_CREATE_ERR;
        }

	if (result == 0)
        	obj2 = json_object_array_get_idx(args, 1);
        if (obj2 == NULL) {
               MCEMT_DBG("json_object_array_get_idx failed!\n");
               goto MC_CREATE_ERR;
        }

        if(json_object_is_type(obj2, json_type_int)){
                *task_id = json_object_get_int(obj2);
        }
        if(*result == 0)
                ret_val = 1;
       
	
        

MC_CREATE_ERR:
        if (args) {
                json_object_put(args);
        }
        if (p_create_task_string) {
                free(p_create_task_string);
        }
        if (p_output) {
                free(p_output);
        }
        MCEMT_DBG("mc_create_task_bt finish\n");								
        return ret_val;

}		

/* Purpose: Create the  tcid task
 *
 * In     : *tcid:    
            *file_path:
 *          *file_name: 
 *
 * Return : 0: FAILURE
 *          1: SUCCESS
 */

int mc_create_task_tcid(char *tcid, char *file_path, char *file_name, u32 *task_id, int *result)		
{
        
        char *p_create_task_string = NULL;
        char *p_output = NULL;
        struct json_object *args = NULL;
        int ret_val = 0;
        struct json_object *obj1 = NULL, *obj2 = NULL;

	tcid += 6;
        if (tcid == NULL || file_path == NULL) {
                MCEMT_DBG("tcid or file_path is NULL\n");
                return 0;
        }
        if (file_name == NULL) {
		p_create_task_string = (char*)calloc(strlen(file_path) + strlen(tcid) + 128,sizeof(char));
		if (p_create_task_string == NULL) {
               		MCEMT_DBG("Fail to calloc p_create_task_string\n");
                	return 0;
       		}
		sprintf(p_create_task_string, "{\"23\":[2,\"%s\",\"\",\"\",\"\",\"\",[],\"%s\",0,\"\",0,0,\"\"]}",file_path, tcid);
	} else {  
			p_create_task_string = (char*)calloc(strlen(file_path) + strlen(tcid) + strlen(file_name) + 128,sizeof(char));
			if (p_create_task_string == NULL) {
               			MCEMT_DBG("Fail to calloc p_create_task_string\n");
                		return 0;
       		 	}
                sprintf(p_create_task_string, "{\"23\":[2,\"%s\",\"%s\",\"\",\"\",\"\",[],\"%s\",0,\"\",0,0,\"\"]}",file_path, file_name, tcid);        
       	}
      
	if (!mc_json_interface(p_create_task_string, MC_BTFLAG_FALSE, &p_output)) {

                MCEMT_DBG("Fail to mc_json_interface\n");
                goto MC_CREATE_ERR;
        }        
              args = json_tokener_parse(p_output);
              if(is_error(args)){
                      MCEMT_DBG("json_tokener_parse failure!\n");
                      goto MC_CREATE_ERR;
        }
           
        if (!json_object_is_type(args, json_type_array)){
                MCEMT_DBG("json_object_is_type failure!\n");
		goto MC_CREATE_ERR;
        }

        obj1 = json_object_array_get_idx(args, 0);
	if (obj1 == NULL) {
		MCEMT_DBG("json_object_array_get_idx failed!\n");
		goto MC_CREATE_ERR;
	}
			
        if (json_object_is_type(obj1, json_type_int)){
                *result = json_object_get_int(obj1);
        }else{
                MCEMT_DBG("json_object_is_type failure!\n");
		goto MC_CREATE_ERR;
        }
        obj2 = json_object_array_get_idx(args, 1);
	if (obj2 == NULL) {
		MCEMT_DBG("json_object_array_get_idx failed!\n");
                goto MC_CREATE_ERR;
	}

        if(json_object_is_type(obj2, json_type_int)){
                *task_id = json_object_get_int(obj2);
        }
        if(*result == 0) {
                ret_val = 1;
        }


        

MC_CREATE_ERR:
	if (args) {
              	json_object_put(args);
	}
        if (p_create_task_string) {
              free(p_create_task_string);
        }
        if (p_output) {
              free(p_output);
        }

        return ret_val;
}

/* Purpose: Create the emule task
 *
 * In     : *url:
            *file_path:
 *          *file_name: 
 *
 * Return : 0: FAILURE
 *          1: SUCCESS
 */

int mc_crate_task_emule(char *url, char *file_path, char *file_name, u32 *task_id, int *result)
{
        
        char *p_create_task_string = NULL;
        char *p_output = NULL;
        struct json_object *args = NULL;
        struct json_object *obj1 = NULL,*obj2 = NULL;
        int ret_val = 0;
        

        if (url == NULL || file_path == NULL) {
                MCEMT_DBG("url or file_path is NULL\n");
                return 0;
        }
        if(file_name == NULL) {
               		 p_create_task_string = (char*)calloc(strlen(file_path) + strlen(url) + 128,sizeof(char));
       			 if (p_create_task_string  == NULL) {
               			 MCEMT_DBG("Fail to calloc p_create_task_string\n");
               			 return 0;
       			}
			sprintf(p_create_task_string,"{\"23\":[4,\"%s\",\"\",\"%s\",\"\",\"\",[],\"\",0,\"\",0,0,\"\"]}",
                        file_path, url);
        } else {
             		p_create_task_string = (char*)calloc(strlen(file_path) + strlen(url) + strlen(file_name) + 128,sizeof(char));
       			if (p_create_task_string  == NULL) {
                		MCEMT_DBG("Fail to calloc p_create_task_string\n");
                		return 0;
        }
			sprintf(p_create_task_string,"{\"23\":[4,\"%s\",\"%s\",\"%s\",\"\",\"\",[],\"\",0,\"\",0,0,\"\"]}",
                        file_path, file_name,url);
        }

	if (!mc_json_interface(p_create_task_string, MC_BTFLAG_FALSE, &p_output)) {

	              MCEMT_DBG("Fail to mc_json_interface\n");
                      goto MC_CREATE_ERR;
        }
        args = json_tokener_parse(p_output);
        if(is_error(args)){
                MCEMT_DBG("json_tokener_parse failure!\n");
                goto MC_CREATE_ERR;
        }
        if(!json_object_is_type(args, json_type_array)){
                MCEMT_DBG("json_object_is_type failure!\n");
		goto MC_CREATE_ERR;
        }

        obj1 = json_object_array_get_idx(args, 0);
        if(json_object_is_type(obj1, json_type_int)){
                *result = json_object_get_int(obj1);
        }else{
                MCEMT_DBG("json_object_is_type failure!\n");


        }
      	
      	if(json_object_array_length(args) == 2) {
        	obj2 = json_object_array_get_idx(args, 1);
        	if(json_object_is_type(obj2, json_type_int)) {
                	*task_id = json_object_get_int(obj2);
       		} else {
        		 MCEMT_DBG("the task's don't be created!\n");
		}
        } 
        
        if(*result == 0) {
                ret_val = 1;
        }

            
MC_CREATE_ERR:
	if (args) {
            	json_object_put(args);
    	}
    	if (p_create_task_string) {
            	free(p_create_task_string);
    	}
    	if (p_output) {
            	free(p_output);
    	}

    	return ret_val;
      
}
      
/* Purpose: make a task use greenchannel
*
* In	 : taskid
* 
* Return : 0: FAILURE
	   1: SUCCESS
*/
int mcetm_use_green_channel(int task_id, int *result)
{
	char *p_mc_use_green_channel = NULL;
	char *p_output = NULL;
        struct json_object *args = NULL;
        struct json_object *obj = NULL;
        int ret_val = 0;

	p_mc_use_green_channel = (char*)calloc(48,sizeof(char));
	if (p_mc_use_green_channel == NULL) {
		MCEMT_DBG("Fail to calloc p_mc_use_green_channel\n");
                return 0;
	}                
	
	sprintf(p_mc_use_green_channel, "{\"10\":[%d]}",task_id);
	
	if (!mc_json_interface(p_mc_use_green_channel, MC_BTFLAG_FALSE, &p_output)) {

              MCEMT_DBG("Fail to mc_json_interface\n");
              goto MC_ERR;
        }
        args = json_tokener_parse(p_output);
        if(is_error(args)){
                MCEMT_DBG("json_tokener_parse failure!\n");
                goto MC_ERR;
        }
        if(!json_object_is_type(args, json_type_array)){
                MCEMT_DBG("json_object_is_type failure!\n");
        }
	obj = json_object_array_get_idx(args, 0);
        if(json_object_is_type(obj, json_type_int)){
                *result = json_object_get_int(obj);
                }
        if(*result == 0) {
                 ret_val = 1;
        }

MC_ERR:
	if (args) {
        json_object_put(args);
	}
        if (p_mc_use_green_channel) {
            free(p_mc_use_green_channel);
	}
        if (p_output) {
            free(p_output);
        }
 
        return ret_val;
}


 /* Purpose: set auto start green channel
 *
 * In     : is_auto_start : 0 or 1
 *           
 *
 * Return : 0: FAILURE
 *          1: SUCCESS
 */
                
//设置是否自动启动高速通道
int  set_auto_start_green_channel(int is_auto_start)
{
        char *p_set_auto_start_green_channel = NULL;
        char *p_output = NULL;
        struct json_object *args = NULL;
        struct json_object *obj = NULL;
        int ret_val = 0;
        int result = 0;        
        p_set_auto_start_green_channel = (char*)calloc(48,sizeof(char));

        if (p_set_auto_start_green_channel== NULL) {
                MCEMT_DBG("Fail to calloc p_set_auto_start_green_channel\n");
                return 0;
        }
               
        if (is_auto_start) {
                sprintf(p_set_auto_start_green_channel,"{\"13\":[1]}");
        } else {
                sprintf(p_set_auto_start_green_channel,"{\"13\":[0]}");
        }
     


	if (!mc_json_interface(p_set_auto_start_green_channel, MC_BTFLAG_FALSE, &p_output)) {

              MCEMT_DBG("Fail to mc_json_interface\n");
              goto MC_ERR;
        }        
        args = json_tokener_parse(p_output);
        if(is_error(args)){
                MCEMT_DBG("json_tokener_parse failure!\n");
                goto MC_ERR;
        }
        if(!json_object_is_type(args, json_type_array)){
                MCEMT_DBG("json_object_is_type failure!\n");
        }

        obj = json_object_array_get_idx(args, 0);
        if(json_object_is_type(obj, json_type_int)){
                result = json_object_get_int(obj);
                }
        if(result == 0) {
                 ret_val = 1;
        }        

MC_ERR:
	if (args) {
        	json_object_put(args);
    	}
   	if (p_set_auto_start_green_channel) {
            free(p_set_auto_start_green_channel);
    	}
    	if (p_output) {
            free(p_output);
   	}

    return ret_val;
             
}

/* get all task id and group id success or failed
 *
 * In     : success_flag: 1(success)or 0 (failed)
 *           
 *
 * Return : 0: FAILURE
 *          1: SUCCESS
 */

int mcetm_get_all_task_id_and_group_id(struct task_status *t, int success_flag,  int *length, int *result)
{
        char *p_mc_get_all_task_id_and_group_id = NULL;
        char *p_output = NULL;
        struct json_object *args = NULL, *obj1 = NULL, *obj2 = NULL;
  
        int ret_val = 0;      	
        p_mc_get_all_task_id_and_group_id = (char*)calloc(48,sizeof(char));
        if (p_mc_get_all_task_id_and_group_id == NULL) {
                MCEMT_DBG("Fail to calloc p_mc_get_all_task_id_and_group_id\n");
                return 0;
        }

        if (success_flag)
                sprintf(p_mc_get_all_task_id_and_group_id,"{\"15\":[%d]}",1);
        else 
                sprintf(p_mc_get_all_task_id_and_group_id,"{\"15\":[%d]}",0);
     
        if (!mc_json_interface(p_mc_get_all_task_id_and_group_id, MC_BTFLAG_FALSE, &p_output)) {

	      MCEMT_DBG("Fail to mc_json_interface\n");
              goto MC_ERR;
        }        
        args = json_tokener_parse(p_output);
        

        if (is_error(args)){
        	MCEMT_DBG("json_tokener_parse failure!\n");
            	goto MC_ERR;
        }
        
        if (!json_object_is_type(args, json_type_array)){
                MCEMT_DBG("json_object_is_type failure!\n");
                goto MC_ERR;
        }
        

        

        *length = json_object_array_length(args);
        obj1 = json_object_array_get_idx(args, 0);
       
        if (json_object_is_type(obj1, json_type_int)){
        	*result = json_object_get_int(obj1);
	}
	if ( result == 0) {
		ret_val = 1;
	}
	
	int i = 0;	
        for(i = 0; i < *length-1; i++) {
	        obj2 = json_object_array_get_idx(args, i+1);
		MCEMT_DBG("json_object_array_get_idx(args, i+1);]\n");
		if (obj2 != NULL) {
			t[i].task_id = json_object_get_int(json_object_array_get_idx(obj2,0));
			t[i].group_id = json_object_get_int(json_object_array_get_idx(obj2,1));	
                        t[i].task_state = json_object_get_int(json_object_array_get_idx(obj2,2));
			t[i].task_type = json_object_get_int(json_object_array_get_idx(obj2,3));
			t[i].sub_id = json_object_get_int(json_object_array_get_idx(obj2,4));
	   	 } else 
			goto MC_ERR;                  
        }

        
MC_ERR:
	MCEMT_DBG("MC_ERR is here!\n");
        if (args) {
                json_object_put(args);
        }
        if (p_mc_get_all_task_id_and_group_id) {
                free(p_mc_get_all_task_id_and_group_id);
        }
        if (p_output) {
                free(p_output);
        }
	MCEMT_DBG("the ret_val is %d\n", ret_val); 	
        return ret_val;
}
       



/* Purpose: start the task
 *
 * In     : task_id:
 *          
 *
 * Return : 0: FAILURE
 *          1: SUCCESS
 */

int mcetm_start_task(int task_id, int *result)
{
        char *p_mc_start_task = NULL;
        char *p_output = NULL;
        struct json_object *args = NULL;
        struct json_object *obj = NULL;
        int ret_val = 0;
        p_mc_start_task = (char*)calloc(48,sizeof(char));

        if (p_mc_start_task== NULL) {
               MCEMT_DBG("Fail to calloc p_mc_start_task\n");
               return 0;
        }
      
                
        sprintf(p_mc_start_task,"{\"2\":[%d]}",task_id);

        if (!mc_json_interface(p_mc_start_task, MC_BTFLAG_FALSE, &p_output)) {
	        MCEMT_DBG("Fail to mc_json_interface\n");
                goto MC_ERR;
        }        
        args = json_tokener_parse(p_output);

        if (is_error(args)){
                MCEMT_DBG("json_tokener_parse failure!\n");
                goto MC_ERR;
        }
        if(!json_object_is_type(args, json_type_array)){
                MCEMT_DBG("json_object_is_type failure!\n");
        }

        obj = json_object_array_get_idx(args, 0);
        if(json_object_is_type(obj, json_type_int)){
                *result = json_object_get_int(obj);
                
                }
        if(*result == 0) {
                 ret_val = 1;
        }        
MC_ERR:
        if (args) {
        json_object_put(args);
        }
        if (p_mc_start_task) {
                free(p_mc_start_task);
        }
        if (p_output) {
                free(p_output);
        }

        return ret_val;
}

/* Purpose: stop the task
 *
 * In     : task_id:
 *          
 *
 * Return : 0: FAILURE
 *          1: SUCCESS
 */

int mcetm_stop_task(int task_id, int *result)
{
	char *p_mc_stop_task = NULL;
        char *p_output = NULL;
        struct json_object *args = NULL;
        struct json_object *obj = NULL;
        int ret_val = 0;
        p_mc_stop_task = (char*)calloc(48,sizeof(char));
        if (p_mc_stop_task == NULL) {
                MCEMT_DBG("Fail to calloc p_mc_stop_task\n");
                return 0;
        }
        
   	sprintf(p_mc_stop_task, "{\"3\":[%d]}", task_id);
	MCEMT_DBG("p_mc_stop_task:%s, len=%d\n", p_mc_stop_task, strlen(p_mc_stop_task));
	if (!mc_json_interface(p_mc_stop_task, MC_BTFLAG_FALSE, &p_output)) {

	        MCEMT_DBG("Fail to mc_json_interface\n");
                goto MC_ERR;
        }

	if (p_output != NULL) {
        	args = json_tokener_parse(p_output);

        	if (is_error(args)){
                	MCEMT_DBG("json_tokener_parse failure!\n");
                	goto MC_ERR;
        	}
        	if(!json_object_is_type(args, json_type_array)){
                	MCEMT_DBG("json_object_is_type failure!\n");
     			goto MC_ERR;
		}
        	obj = json_object_array_get_idx(args, 0);
        	if(json_object_is_type(obj, json_type_int)){
                	*result = json_object_get_int(obj);
        	}
		MCEMT_DBG("the result is %d\n", *result);
        	if(*result == 0) {
                	ret_val = 1;
        	}
	}
 
MC_ERR:
        if (args) {
                json_object_put(args);
        }
	
        if (p_mc_stop_task) {
                free(p_mc_stop_task);
        }
        if (p_output) {
                free(p_output);
        }

        return ret_val;


}

/* Purpose: delete the task
 *
 * In     : task_id:
 *          
 *
 * Return : 0: FAILURE
 *          1: SUCCESS
 */

int mcetm_delete_task(int task_id, int is_delete_file, int *result)
{
        char *p_mc_delete_task = NULL;
        char *p_output = NULL;
        struct json_object *args = NULL;
        struct json_object *obj = NULL;
        int ret_val = 0;
        
        
        p_mc_delete_task = (char*)calloc(48,sizeof(char));

        if (p_mc_delete_task == NULL) {
                MCEMT_DBG("Fail to calloc p_mc_delete_task\n");        
                return 0;
        }
        sprintf(p_mc_delete_task,"{\"4\":[[%d,%d]]}",task_id, is_delete_file);

        if (!mc_json_interface(p_mc_delete_task, MC_BTFLAG_FALSE, &p_output)) {
		MCEMT_DBG("Fail to mc_json_interface\n");
                goto MC_ERR;
        }
        args = json_tokener_parse(p_output);

        if (is_error(args)){
                MCEMT_DBG("json_tokener_parse failure!\n");
                goto MC_ERR;
        }
          if(!json_object_is_type(args, json_type_array)){
                MCEMT_DBG("json_object_is_type failure!\n");
        }

        obj = json_object_array_get_idx(args, 0);
        if(json_object_is_type(obj, json_type_int)){
                *result = json_object_get_int(obj);
                
                }
        if(*result == 0) {
                 ret_val = 1;
        }      
        MCEMT_DBG("mcetm_delete_task end\n");        
MC_ERR:
        if (args) {
                json_object_put(args);
        }
        if (p_mc_delete_task) {
                free(p_mc_delete_task);
        }
        if (p_output) {
                free(p_output);
        }

        return ret_val;
    
}

/* Purpose: get the task infomation
 *
 * In     : task_id:
 *          
 *
 * Return : 0: FAILURE
 *          1: SUCCESS
 */

int get_task_info_by_id(int task_id, struct task_info *t, int *result)
{
        char *p_get_task_info_by_id = NULL;
        char *p_output = NULL;
        struct json_object *args = NULL, *obj = NULL, *obj1 = NULL;
        int ret_val = 0;

	t->file_name = NULL;
	t->file_path = NULL;
	MCEMT_DBG("get_task_info_by_id start\n");
        p_get_task_info_by_id = (char*)calloc(48,sizeof(char));

        if (p_get_task_info_by_id == NULL) {
               MCEMT_DBG("Fail to calloc p_get_task_info_by_id\n"); 
               return 0;
        }
        sprintf(p_get_task_info_by_id, "{\"5\":[%d]}",task_id);
	
        if (!mc_json_interface(p_get_task_info_by_id, MC_BTFLAG_FALSE, &p_output)) {
		MCEMT_DBG("Fail to mc_json_interface\n");
                goto MC_ERR;
        } 
	if (p_output == NULL) {
		MCEMT_DBG("p_output is NULL\n");   
		goto MC_ERR; 
	}
        args = json_tokener_parse(p_output);
        if (is_error(args)){
                MCEMT_DBG("json_tokener_parse failure!\n");
                goto MC_ERR;
        }

        if (!json_object_is_type(args, json_type_array)){
                MCEMT_DBG("json_object_is_type failure!\n");
        }

        obj = json_object_array_get_idx(args, 0);
        if (json_object_is_type(obj, json_type_int)){
                *result = json_object_get_int(obj);
        }
	if (*result != 0) {
		 MCEMT_DBG("get information is failure!\n");
		 goto MC_ERR;
	} else 
		ret_val = 1;
        t->task_state =  json_object_get_int(json_object_array_get_idx(args, 1));
        t->task_type = json_object_get_int(json_object_array_get_idx(args, 2));


	obj1 = json_object_array_get_idx(args, 3);
	if (obj1 != NULL) {	
		if (json_object_is_type(obj1, json_type_string)) {
			t->file_name = (char*)calloc(512,sizeof(char));	
			strcpy(t->file_name, json_object_get_string(obj1));
			MCEMT_DBG("t->file_name is %s\n", t->file_name);
		} else {
			MCEMT_DBG("json_object_is_type failure!\n");
			goto MC_ERR;
		}	
	} else {
		ret_val = 0;
		goto MC_ERR;
	}
	
	obj1 = json_object_array_get_idx(args, 4);
	if (obj1 != NULL) {	
        	if (json_object_is_type(obj1, json_type_string)){
       			t->file_path = (char*)calloc(512,sizeof(char));  
       			strcpy(t->file_path, json_object_get_string(obj1));
		} else {
			MCEMT_DBG("json_object_is_type failure!\n");
			goto MC_ERR;
		}
	} else {
		ret_val = 0;
		goto MC_ERR;
	} 
	obj1 = json_object_array_get_idx(args, 23);
        if (obj1 != NULL) {
                if (json_object_is_type(obj1, json_type_string)){
                        t->url = (char*)calloc(1024,sizeof(char));
                        strcpy(t->url, json_object_get_string(obj1));
                } else {
                        MCEMT_DBG("json_object_is_type failure!\n");
                        goto MC_ERR;
                }
        } else {
                ret_val = 0;
                goto MC_ERR;
        }	

	t->task_id = task_id;
	t->file_size = json_object_get_double(json_object_array_get_idx(args, 5));
        MCEMT_DBG("#####################\n");
        MCEMT_DBG("%llu\n", t->file_size);
        MCEMT_DBG("#####################\n");
        t->download_data_size = json_object_get_double(json_object_array_get_idx(args, 6));
        t->start_time = json_object_get_int(json_object_array_get_idx(args, 7));
        t->finished_time = json_object_get_int(json_object_array_get_idx(args, 8));
        t->failed_code = json_object_get_int(json_object_array_get_idx(args, 9));
        t->dl_speed = json_object_get_int(json_object_array_get_idx(args, 10));
        t->ul_speed = json_object_get_int(json_object_array_get_idx(args, 11));
        t->downloading_pipe_num = json_object_get_int(json_object_array_get_idx(args, 12));
        t->connecting_pipe_num = json_object_get_int(json_object_array_get_idx(args, 13));
        t->is_green_channel_ready = json_object_get_int(json_object_array_get_idx(args, 14));
        t->green_channel_state = json_object_get_int(json_object_array_get_idx(args, 15));
        t->green_channel_failed_code = json_object_get_int(json_object_array_get_idx(args, 16));
        t->created_file_size = json_object_get_double(json_object_array_get_idx(args, 22));
MC_ERR:
        if(args){
                json_object_put(args);
        }
        if (p_get_task_info_by_id) {
                free(p_get_task_info_by_id);
        }
        if (p_output) {
                free(p_output);
        }
	MCEMT_DBG("get_task_info_by_id finish\n");
        return ret_val;
}


/* Purpose: get the system infomation^M
 *^M
 * In     : :^M
 *          ^M
 *^M
 * Return : 0: FAILURE^M
 *          1: SUCCESS^M
 */
int get_system_info(struct system_info *t)
{
	char *p_get_system_info = NULL;
        char *p_output = NULL;
        struct json_object *args = NULL, *obj1 = NULL;
        int ret_val = 0;
        int result = 0;
       	MCEMT_DBG("get_system_info start\n"); 
        p_get_system_info = "{\"6\":[]}";
        if (!mc_json_interface(p_get_system_info, MC_BTFLAG_FALSE, &p_output)) {

                MCEMT_DBG("Fail to mc_json_interface\n");
                goto MC_ERR;
        } 

	if (p_output == NULL) {
		MCEMT_DBG("p_output is NULL\n");
                goto MC_ERR; 
	}
      
	
	args = json_tokener_parse(p_output);
	if (is_error(args)){
                MCEMT_DBG("json_tokener_parse failure!\n");
                goto MC_ERR;
        }
        if (!json_object_is_type(args, json_type_array)){
                MCEMT_DBG("json_object_is_type failure!\n");
        }
	
	obj1 = json_object_array_get_idx(args, 0);
        if (json_object_is_type(obj1, json_type_int)){
                result = json_object_get_int(obj1);
	}
	


	t->is_net_ok  = json_object_get_int(json_object_array_get_idx(args, 1));
	t->is_license_ok = json_object_get_int(json_object_array_get_idx(args, 2));
	obj1 = json_object_array_get_idx(args, 3);
        if (json_object_is_type(obj1, json_type_string)){
                t->license = (void*)calloc(256,sizeof(char));
                strcpy(t->license, json_object_get_string(obj1));
        } else {
                MCEMT_DBG("json_object_is_type failure!\n");
                goto MC_ERR;
        }
	


	obj1 = json_object_array_get_idx(args, 5);
        if (json_object_is_type(obj1, json_type_string)){
                t->bind_acktive_key = (char*)calloc(256,sizeof(char));
                strcpy(t->bind_acktive_key, json_object_get_string(obj1));
        } else {
                MCEMT_DBG("json_object_is_type failure!\n");
                goto MC_ERR;
                }

	obj1 = json_object_array_get_idx(args, 6);
        if (json_object_is_type(obj1, json_type_string)){
                t->peerid = (char*)calloc(256,sizeof(char));
                strcpy(t->peerid, json_object_get_string(obj1));
        } else {
                MCEMT_DBG("json_object_is_type failure!\n");
                goto MC_ERR;
                }
		
	
        obj1 = json_object_array_get_idx(args, 7);
        if (json_object_is_type(obj1, json_type_string)){
                t->userid = (char*)calloc(256,sizeof(char));
                strcpy(t->userid, json_object_get_string(obj1));
        } else {
                MCEMT_DBG("json_object_is_type failure!\n");
                goto MC_ERR;
        }


	
        obj1 = json_object_array_get_idx(args, 8);
        if (json_object_is_type(obj1, json_type_string)){
                t->username = (char*)calloc(256,sizeof(char));
                strcpy(t->username, json_object_get_string(obj1));
        } else {
                MCEMT_DBG("json_object_is_type failure!\n");
                goto MC_ERR;
                }
	t->is_bind_ok = json_object_get_int(json_object_array_get_idx(args, 4));
	t->is_multi_disk = json_object_get_int(json_object_array_get_idx(args, 9)); 
	if (result == 0) {
        	ret_val = 1;
        } 

MC_ERR:

       	if (args) {
                json_object_put(args);
        }
        if (p_output) {
                free(p_output);
        }
	MCEMT_DBG("get_system_info finish\n");	
        return ret_val;
}

int mcetm_unbind()
{
        char *p_unbind = NULL;
        char *p_output = NULL;
        struct json_object *args = NULL;
        struct json_object *obj = NULL;
        int ret_val = 0;
        int result = 0 ;
       
      	p_unbind = "{\"9\":[]}"; 
 
        if (!mc_json_interface(p_unbind, MC_BTFLAG_FALSE, &p_output)) {
                MCEMT_DBG("Fail to mc_json_interface\n");
                goto MC_ERR;
        }
        args = json_tokener_parse(p_output);
        if (is_error(args)){
                MCEMT_DBG("json_tokener_parse failure!\n");
                goto MC_ERR;
        }
          if(!json_object_is_type(args, json_type_array)){
                MCEMT_DBG("json_object_is_type failure!\n");
        }
	
	 args = json_tokener_parse(p_output);

        if (is_error(args)){
                MCEMT_DBG("json_tokener_parse failure!\n");
                goto MC_ERR;
        }
          if(!json_object_is_type(args, json_type_array)){
                MCEMT_DBG("json_object_is_type failure!\n");
        }

        obj = json_object_array_get_idx(args, 0);
        if(json_object_is_type(obj, json_type_int)){
                result = json_object_get_int(obj);
                
                }
        if(result == 0) {
                 ret_val = 1;
		MCEMT_DBG("########################################\n");
		MCEMT_DBG("UNBIND SUCCESS\n");
		MCEMT_DBG("########################################\n");
        }        
MC_ERR:
        if (args) {
                json_object_put(args);
        }
        return ret_val;
    
}
