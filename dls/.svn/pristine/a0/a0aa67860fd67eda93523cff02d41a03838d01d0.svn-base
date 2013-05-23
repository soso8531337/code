#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "httpd.h"
#include "xl.h"

#define DLD_ETC_PATH "/etc/dld.cfg"

int xl_flag;
char sses[256] = "B9Ps4WwhrbZgcUl5NTigWblCCbSywqdzg0slULKGCIW1rrEU";
int sockfd;
pthread_t tid;




int main()
{
        MCEMT_DBG("getpid is %ld\n", getpid());  
	httpd_t *ht;  
        FILE *fd;
        char fl[10];
        if(access(DLD_ETC_PATH, F_OK)){
                fd = fopen(DLD_ETC_PATH, "w+");
                fputs("1", fd);
                fclose(fd);
        }
       
        fd = fopen(DLD_ETC_PATH, "r+");
        fgets(fl, 10, fd);
        xl_flag = atoi(fl);
        fclose(fd);

      	MCEMT_DBG("main start!\n");
        if(xl_flag == 1){
                        MCEMT_DBG("main pthread_create start!\n");
	        if(pthread_create(&tid, NULL, xl_monitor, NULL) != 0)
	                return -1;
                        
        }else if(xl_flag == 0){
                        MCEMT_DBG("main pthread_create start!\n");
                if(pthread_create(&tid, NULL, tr_monitor, NULL) != 0)
                        return -1;                                                      
	}else{
	        exit(0);
        }
      
	/* Allocate httpd object  */
	if ((ht = httpd_new(&ht)) == NULL){
		MCEMT_DBG("main httpd_new finish!\n");
		return 1;
	}

	/* Initialize httpd server        */
	if (!ht->init(ht, prgcgi_main_handler)) {
		MCEMT_DBG("main ht->init finish!\n");
		return 1;
	}

	/* Waiting client to request      */
	ht->wait(ht);

	/* Finish httpd server            */
	ht->finish(ht);
	
	/* Destroy the httpd object       */
	ht->delete(ht);

	if (pthread_cancel(tid) == 0) {
		pthread_join(tid, NULL);
	}
	
	MCEMT_DBG("main finish!\n");
	return 0;
}
