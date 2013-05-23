#ifndef _TR_UTIL_H_
#define _TR_UTIL_H_
#include <stdbool.h>
#include "common.h"

typedef struct status_info{
        int uploadspeed;
        int downloadspeed;
}status_info_t;

typedef struct task_info{
	int task_id;
	int task_state;
        int failed_code;
	char *file_name;
	long long  file_size;
        long long  left_data_size;
	long long  download_data_size;
        long long  uploaded_data_size;
	int start_time;
	int finished_time;
	int dl_speed;
	int ul_speed;
        int utime;
        int ltime;
        int status;
        bool isStalled;
        bool isFinished;
        float uploadRatio;
        float percentDone;
       	int downloading_pipe_num;
	int connecting_pipe_num;
} task_info_t;

extern int tr_send_post_getlimit(int s, int *num, int *down, int *up);
extern int tr_send_post_setdownlimit(int s, int down);
extern int tr_send_post_setpath(int s, char *path);
extern int tr_send_post_setlimit(int s, int num, int con, int down, int up);
void *tr_send_post_remove(void *id);
extern int tr_send_post_start(int s, int id);
extern int tr_send_post_stop(int s, int id);
extern int tr_send_post_get_info(int s, task_info_t **t, int *tl);
extern int tr_send_post_get_info_re(int s, task_info_t **t, int *tl);
extern int tr_send_post_get_info_num(int s, task_info_t **t, int *tl, int *task);
extern int tr_send_post_add(int s, const char *file_path);
extern int tr_send_post_get_session(int s);
extern int tr_send_post_get_session_status(int s, status_info_t *t);




#endif
