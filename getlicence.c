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

#include "common.h"

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

char *get_license(char *license)
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
