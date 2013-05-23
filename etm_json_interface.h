#ifndef _ETM_JSON_INTERFACE_H
#define _ETM_JSON_INTERFACE_H

/******************************************************************************
 *  * Copyright (C) 2012
 *  * 
 *  * File: etm_json_interface.h
 *  *
 *  * Date: 2012-03-26
 *  *
 *  * Author: Hulong, <hulong@birdsongsoft.com>
 *  *
 *  * Descriptor:
 *  *   
 *  * Note:
 *  *  XunLei etm library json header
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
#include <errno.h>
#include <sys/types.h>
#include <string.h>
#include <memory.h>
#include <malloc.h>
#include <dirent.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include "et_task_manager.h"
/*-----------------------------------------------------------------------------
  const
------------------------------------------------------------------------------*/
#define VOD_CACHE_PATH "etm_vod/"
#define MC_JSON_BUFFER_LEN      (2*1024)   //4KB
#define MC_JSON_BT_BUFFER_LEN   (4*1024)  //8KB
#define MC_BTFLAG_TRUE          (1)
#define MC_BTFLAG_FALSE         (0)
#define VOD_CACHE_SIZE (3*1024*1024)    // 3 GB
#define VOD_BUFFER_SIZE (2*1024) 
#define DRM_FILE_FULL_PATH "./480p.xlmv"

#define BOARD_FLASH_BASE_ADDR                   0x30000000
#define BOARD_FLASH_PARAM_ADDR                  (BOARD_FLASH_BASE_ADDR + 0x00020000)
/* bootloader         */

#ifdef _BOARD_DLDUIS2000
#define VS_CFG_MTD "/dev/mtd2"
#else
#define VS_CFG_MTD "/dev/mtd1"
#endif
/* bootloader            */
#define VSBOOTINFO_MAX            		(1024)
#define VSBOOTINFO_IP_LEN         		(16)
#define VSBOOTINFO_MASK_LEN    		(16)
#define VSBOOTINFO_GATEWAY_LEN    	(16)
#define VSBOOTINFO_SERVER_LEN     	(16)
#define VSBOOTINFO_RESV           		(VSBOOTINFO_MAX-VSBOOTINFO_IP_LEN-\
                                           			VSBOOTINFO_MASK_LEN-VSBOOTINFO_GATEWAY_LEN-\
                                           			VSBOOTINFO_SERVER_LEN)
/* License               		*/
#define VSLICENSE_MAX_LEN        		 (128)
/* Common information    	*/
#define VSCOMINFO_MAX             	(4096)
#define VSCOMINFO_VENDOR          	(16)
#define VSCOMINFO_PRODUCT		(32)
#define VSCOMINFO_VER             	(8)
#define VSCOMINFO_SERIAL          	(32)
#define VSCOMINFO_MAC0            	(24)
#define VSCOMINFO_MAC1            	(24)

#define VSINFO_READ_MAX           	((sizeof(vsinfo_t) + 4095)/4096)*4096
/* Socket */
#define SERVER_PORT               	(4800)
#define SERVER_IP                 		"127.0.0.1"

/* To support large file check for XunLei, Add by Liu Yong  */
#define VS_KIBIBYTE_SIZE          1024LL
#define VS_MEBIBYTE_SIZE          1048576LL
#define VS_GIBIBYTE_SIZE          1073741824LL
#define VS_TEBIBYTE_SIZE          1099511627776LL
/*------------------------------------------------------
-----------------------------------------------------------------------------*/

typedef enum t_task_state
{
	TS_TASK_WAITING=0, 
	TS_TASK_RUNNING, 
	TS_TASK_PAUSED,
	TS_TASK_SUCCESS, 
	TS_TASK_FAILED, 
	TS_TASK_DELETED 
} TASK_STATE;

typedef struct task_info {
	int task_id;
	int task_state;
	int task_type;
	char *file_name;
	char *file_path;
	char *url;
	uint64 file_size;
	uint64 download_data_size;
	int start_time;
	int finished_time;
	int failed_code;
	int dl_speed;
	int ul_speed;
	int downloading_pipe_num;
	int connecting_pipe_num;
	int is_green_channel_ready;
	int green_channel_state;
	int green_channel_failed_code;
	uint64 created_file_size;
} task_info_t;

typedef struct system_info {
	int result;
	int is_net_ok;
	int is_license_ok;
	char *license;
	int is_bind_ok;
	char *bind_acktive_key;
	char *peerid;
	char *userid;
	char *username;
	int  is_multi_disk;
} system_info_t;

typedef struct task_bt_sub {
	int subid;
	char *subname;
	char *sub_file_path;
	int subfilesize;
} task_bt_sub_t;

typedef struct task_status {
	int task_id;
	int group_id;
	int task_state;
	int task_type;
	int sub_id;
} task_status_t;

/* 1K Bytes for bootloader         */
struct _vsbootinfo_t {
        char ip[VSBOOTINFO_IP_LEN];
        char mask[VSBOOTINFO_MASK_LEN];
        char gateway[VSBOOTINFO_GATEWAY_LEN];
        char server[VSBOOTINFO_SERVER_LEN];
        char resv[VSBOOTINFO_RESV];
} __attribute__ ((packed));

typedef struct _vsbootinfo_t vsbootinfo_t;
/* Product license                 */
struct _vslicense_t {
        int len;                     /* license length */
        char str[VSLICENSE_MAX_LEN]; /* license string */
} __attribute__ ((packed));

typedef struct _vslicense_t vslicense_t;
/* 4K Bytes for common information */
struct _vscominfo_t {
        char vendor[VSCOMINFO_VENDOR];
        char product[VSCOMINFO_PRODUCT];
        char ver[VSCOMINFO_VER];
        char serial[VSCOMINFO_SERIAL]; /* Product serial number */
        char mac0[VSCOMINFO_MAC0];
        char mac1[VSCOMINFO_MAC1];
        vslicense_t xunlei;
        char resv[1];
} __attribute__ ((packed));

typedef struct _vscominfo_t vscominfo_t;
/* Partition 1 information         */
struct _vsinfo_t {
	unsigned int ver;
        vsbootinfo_t bootinfo;
        vscominfo_t cominfo;
} __attribute__ ((packed));

typedef struct _vsinfo_t vsinfo_t;
/*-----------------------------------------------------------------------------
  global
------------------------------------------------------------------------------*/
extern int mc_rt_failed;

/*-----------------------------------------------------------------------------
  Macro
------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
  local functions
------------------------------------------------------------------------------*/
extern int mc_etm_init();
extern int mc_create_task_url(char *url, char *file_path, char *file_name, u32 *task_id, int *result);
extern int mc_create_task_bt(char *seed_file_path, char *file_path, int *selected_id, u32 *task_id, int *result);
extern int mc_create_task_tcid(char *tcid, char *file_path, char *file_name, u32 *task_id, int *result);
extern int mc_crate_task_emule(char *url, char *file_path, char *file_name, u32 *task_id, int *result);
extern int set_auto_start_green_channel(int is_auto_start);
extern int mcetm_use_green_channel(int task_id ,int *result);
extern int mcetm_start_task(int task_id, int *result);
extern int mcetm_stop_task(int task_id, int *result);
extern int mcetm_delete_task(int task_id, int is_delete_file, int *result);
extern int get_task_info_by_id(int task_id, struct task_info *t, int *result);
extern int get_system_info();
extern int mcetm_unbind();
extern int mcetm_get_all_task_id_and_group_id(struct task_status t[], int success_flag, int *length, int *result);
extern int mcetm_get_bt_seed_info(char *seed_file_path, char *encoding_mode, int *length, int *result);
#endif
