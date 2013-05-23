#ifndef _LTHREAD_H
#define _LTHREAD_H                  1

/******************************************************************************
 * Copyrigh (C) 2008 by IOVST
 *
 * File: lthread.h
 *
 * Date: 2008-02-27
 *
 * Author: Liu Yong, <liuyong@iovst.com>
 *
 * Version: 0.1
 *
 * Descriptor:
 *   The general thread pool
 *
 * Modified:
 *
 ******************************************************************************/

/*-----------------------------------------------------------------------------
  System header files 
------------------------------------------------------------------------------*/
#include <pthread.h>

/*-----------------------------------------------------------------------------
  Local header files
------------------------------------------------------------------------------*/
#ifdef MEM_LEAK
#include "memleak.h"
#endif
#include "list.h"
#include "common.h"

/*-----------------------------------------------------------------------------
  Data structures
------------------------------------------------------------------------------*/
typedef void *(*th_handler_t)(void *);

typedef struct _lthread_t {
	struct list_head node;     /* Thread record node             */
/*
 * Data
 */
        int no;                     /* Thread unique No. in the pool  */
#define LTHREAD_CLR             (0U)
#define LTHREAD_STOP            (1U)
#define LTHREAD_RUN             (2U)
#define LTHREAD_EXIT            (4U)
	volatile unsigned int stat; /*
	                             * STOP <----------> RUN 
				     *  |                 |
				     *  +-----------------+
                                     *          |
                                     *         EXIT 
				     */
	pthread_mutex_t wtlock;    /* waiting lock                    */
	volatile int terminate;             /* Terminate flag                  */
	
	int overtime;              /* unused time for the thread      */
	pthread_t tid;             /* pthread ID                      */

	th_handler_t handler;      /* thread handler function         */
	void *data;                /* thread handler function argument*/

	th_handler_t qhandler;     /* quit handler function           */
        void *qdata;               /* quit handler function argument  */

} lthread_t, *lthread_tp;


typedef struct _th_pool_t {
/*
 * Data
 */
	struct list_head list; /* thread pool list                   */
	int defnum;            /* the min. number of thread          */
	int maxnum;            /* the max. number of thread          */
	int curnum;            /* the current number of thread       */	
	pthread_mutex_t lock;  /* Protect:
			        *  list, defnum, maxnum, curnum
			        */
	int idle_time;         /* the idle time of free thread       */
	
	pthread_t tid;         /* the monitor thread                 */
	int terminate;         /* the waiting lock of monitor thread */

/* Basic Ways   */
	void (*delete)(struct _th_pool_t *);

/* Special Ways */
 	int (*alloc)(struct _th_pool_t *, th_handler_t h, void *d,
		     int overtime, th_handler_t qh, void *qd);
	int (*kill)(struct _th_pool_t *, int no);

} th_pool_t, *th_pool_tp;

/*-----------------------------------------------------------------------------
  Consts
------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
  Function declaration
------------------------------------------------------------------------------*/
extern th_pool_t *th_pool_new(th_pool_t **p, int defnum,
			      int maxnum, int idle_time);

/*-----------------------------------------------------------------------------
  Macros
------------------------------------------------------------------------------*/

#endif /* _LTHREAD_H */

