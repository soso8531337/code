#ifndef XL_UTIL_H
#define XL_UTIL_H

typedef struct limit_info{
        int s;
        int task_num;
        int task_co;
        int uspeed;
        int dspeed;
}limit_info;

extern int xl_unbind();
extern int xl_safe_quit();
extern int xl_getsysinfo(char *bind_key, int *bind_ok, int *net_ok, int *license_ok, int *disk_ok);
extern int xl_set_maxtask(int num);
extern int xl_set_limit_speed(int dspeed, int uspeed);


#endif

