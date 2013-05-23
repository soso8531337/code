#ifndef _XL_H
#define _XL_H                  1
#include "cgi.h"
//#include "common.h"
#include "tr.h"

/*
#define LOCAL_PATH "/data/HardDisk1/Volume1/.vst/./tddownload/"
#define ETM_SYSTEM_PATH "/data/HardDisk1/Volume1/.vst/./etm_system/"
#define DEFAULT_DOWNLOAD_PATH "/data/HardDisk1/Volume1/.vst/./tddownload/"

#define LICENSE "10051800010000000000001foh2j15w25ih9k3pc30"  //Super license
//#define LICENSE "090402000100000180000077cgq7x8a51wjr5etwtd"
//#define LICENSE "0904020001000001900000716i8375l7n6fwezpym1"
//#define LICENSE "09040200010000042000007ykh2pf2k3474ws6500b"
*/
#define XL_ERRNO_SETTING_LICENSE	(102406)

/* Link parse                */
#define EMULE_PREFIX            "ed2k://"
#define EMULE_PREFIX_LEN        (7)
#define FTP_PREFIX              "ftp://"
#define FTP_PREFIX_LEN          (6)
#define HTTP_PREFIX             "http://"
#define HTTP_PREFIX_LEN         (7)
#define HTTPS_PREFIX            "https://"
#define HTTPS_PREFIX_LEN        (8)
#define CID_PREFIX              "cid://"
#define CID_PREFIX_LEN          (6)
#define THUNDER_PREFIX          "thunder://"
#define THUNDER_PREFIX_LEN      (10)
#define FLASHGET_PREFIX         "flashget://"
#define FLASHGET_PREFIX_LEN     (11)
#define QQ_PREFIX               "qqdl://"
#define QQ_PREFIX_LEN           (7)
#define RAYFILE_PREFIX          "fs2you://"
#define RAYFILE_PREFIX_LEN      (9)

#define DLD_ML_TYPE        (0x00000001)
#define DLD_P2P_TYPE       (DLD_ML_TYPE)
#define DLD_BT_TYPE        (0x00000002)
#define DLD_FTP_TYPE       (0x00000004)
#define DLD_HTTP_TYPE      (0x00000008)
#define DLD_CID_TYPE       (0x00000010)
#define DLD_XLML_TYPE      (0x00000020)
#define DLD_FLASHGET_TYPE  (0x00000040)
#define DLD_QQ_TYPE        (0x00000080)
#define DLD_RAYFILE_TYPE   (0x00000100)
#define DLD_THUNDER_TYPE   (0x00000200)
#define DLD_NO_NORMAL_TYPE (DLD_FLASHGET_TYPE | DLD_QQ_TYPE | DLD_RAYFILE_TYPE | DLD_THUNDER_TYPE)
#define DLD_XL_TYPE        (DLD_HTTP_TYPE | DLD_FTP_TYPE | DLD_CID_TYPE | DLD_BT_TYPE | DLD_XLML_TYPE | DLD_NO_NORMAL_TYPE)
#define DLD_ALL_TYPE       (DLD_ML_TYPE | DLD_XL_TYPE)


#define PRO_BASE_ARG_ERR			20100000
#define PRO_DLD_TASK_START_ERR			20102180
#define PRO_DLD_TASK_STOP_ERR			20102181
#define PRO_DLD_TASK_DEL_ERR			20102182
#define PRO_DLD_TASK_RESUME_ERR			20102183
#define PRO_DLD_TASK_CLEAR_ERR			20102184
#define PRO_DLD_TASK_PRIHIGH_ERR			20102185
#define PRO_DLD_TASK_PRILOW_ERR			20102186
#define PRO_DLD_TASK_URL_ERR			20102200
#define PRO_DLD_TASK_URL_TYPE_ERR			20102201
#define PRO_DLD_TASK_URL_NAME_ERR			20102202
/* protocol each function    */


/*bit 5 means if need login 0->no need login 1->need login*/
#define PROTOCOL_LOGIN	1<<4


#define PRO_SERV_ACTION_START                   1
#define PRO_SERV_ACTION_STOP                    0


#define   PRO_DLD_BASE				"base"
#define   PRO_DLD_BASE_INTERVAL		        "interval"
#define   PRO_DLD_BASE_LOWSPEED		        "lowspeed"
#define   PRO_DLD_BASE_LOWTIME		        "lowtime"

#define   PRO_BASE_NOTROOT                      20100005      
#define   PRO_DLD_BASE_INTERVAL_ERR		20102010
#define   PROTOCOL_FUCTION	                1<<5
#define   PRO_BASE_IDENTITY_ERR			20100002
#define   PRO_DLD_BASE_DSPEED_ERR		20102013
#define   PRO_DLD_BASE_LSPEEDTIMEOUT_ERR        20102014


#define       PRO_DLD_XL		        "xl"
#define       PRO_DLD_XL_STATUS			"action"
#define       PRO_DLD_XL_NUM			"num"
#define       PRO_DLD_XL_CONNECT		"connect"
#define       PRO_DLD_XL_UPRATE			"uprate"
#define       PRO_DLD_XL_DOWNRATE		"downrate"

#define   PRO_TR_CHOOSE                         "choose"
#define   PRO_XL_SELECT                         "select"
#define   PRO_XL_SIMPLEINFO			"simpleinfo"
#define   PRO_XL_ARG				"arg"
#define   PRO_XL_RECORD			        "list"
#define   PRO_XL_RECORD_LIST		        "list"
#define   PRO_XL_RECORD_ID		        "id"
#define   PRO_XL_TASKS				"tasks"
#define   PRO_XL_TASKS_ID			"id"
#define   PRO_XL_TASKS_ACTION	                "action"
#define   PRO_XL_URL				"url"
#define   PRO_XL_URL_PATH			"path"
#define   PRO_XL_URL_FLAG			"flag"
#define   PRO_XL_URL_NAME	                "name"
#define   PRO_XL_URL_INDEX		        "index"
#define   PRO_XL_SYSTEM				"system"

void default_cgi_handler(void *data);
void *xl_monitor(void *arg);

#endif

