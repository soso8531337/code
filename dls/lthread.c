/******************************************************************************
 * Copyright (C) 2007 by IOVST
 * 
 * File: lthread.c
 *
 * Date: 2008-02-27
 *
 * Author: wang songnian
 *
 * Descriptor: the general thread pool
 *   
 * Version: 0.1
 *
 * Modified:
 *
 ******************************************************************************/

/*----------------------------------------------------------------------------
  System header files 
------------------------------------------------------------------------------*/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>

#include <sys/time.h>
/*-----------------------------------------------------------------------------
  Local header files
------------------------------------------------------------------------------*/
#include "list.h"
#include "lthread.h"

/*-----------------------------------------------------------------------------
  Data structures
------------------------------------------------------------------------------*/
static int total ;
/*-----------------------------------------------------------------------------
  Global variables and constants
------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
  Functions Declare
------------------------------------------------------------------------------*/
static void th_pool_delete(th_pool_t *p);
static int th_pool_alloc(th_pool_t *p, th_handler_t h, void *d,
			 int overtime, th_handler_t qh, void *qd);
static int th_pool_kill(th_pool_t *p, int no);
static void *th_pool_monitor(void *arg);
static lthread_t *th_create_thread(th_pool_t *p);
static void th_kill_thread(lthread_t *th);
static void *th_handler(void *arg);
static void th_update_time(th_pool_t *p, lthread_t *th);
static void th_recycle(th_pool_t *p, lthread_t *th);

/* Task record  */
static lthread_t *th_task_alloc(void);
static void th_task_free(lthread_t *th);


/*-----------------------------------------------------------------------------
  Functions
------------------------------------------------------------------------------*/
/* Purpose   : Create a thread pool object
 * Parameter : **p: the pointer to stor thread pool object
 *             defnum: the default thread number
 *             maxnum: the max thread number
 *             idle_time: the max idle time
 * Return    : NULL: Error
 *             the thread pool object
 */
th_pool_t *th_pool_new(th_pool_t **p, int defnum, int maxnum, int idle_time)
{
	th_pool_t *tmp;
	int i;

	total = 0;
	if ((defnum <= 0) || (maxnum <= 0) || 
	    (defnum > maxnum) || (idle_time <= 0)) {
	    return NULL;
	}

	if ((tmp = calloc(1, sizeof(th_pool_t))) == NULL)
		return NULL;
	else
		*p = tmp;
	
	tmp->defnum = defnum;
	tmp->maxnum = maxnum;
	tmp->idle_time = idle_time;
	pthread_mutex_init(&tmp->lock, NULL);
	tmp->terminate = 0;
	INIT_LIST_HEAD(&tmp->list);

	/* Create the thread pool    */
	for (i = 0; i < defnum; i++) {
		if (th_create_thread(tmp) == NULL) {
			goto TH_ERR;
		}
	}
	
	/* Set the ways              */
	tmp->delete = th_pool_delete;
	tmp->alloc  = th_pool_alloc;
	tmp->kill   = th_pool_kill;

	/* Create the monitor thread */
	if (pthread_create(&tmp->tid, NULL, th_pool_monitor, tmp) != 0) {
		goto TH_ERR;
	}  
	return tmp;
		
TH_ERR:
	th_pool_delete(tmp);
	return NULL;
}

/* Purpose  : Destory the thread pool
 * Prameter : *p: the thread pool object
 * Return   : void
 */
static void th_pool_delete(th_pool_t *p)
{
	struct list_head *pos, *tmp;
	lthread_t *rec;
	
	/* Kill the monitor thread */
	if (p->tid != 0) {
		p->terminate = 1;
		if (pthread_cancel(p->tid) == 0) {
			pthread_join(p->tid, NULL);
		}
	}

	pthread_mutex_lock(&p->lock);
	
	/* Free the thread pool    */
	list_for_each_safe(pos, tmp, &p->list) {
		rec = list_entry(pos, lthread_t, node);
		if (rec->stat & (LTHREAD_STOP | LTHREAD_RUN)) {
			th_kill_thread(rec);
		}
		list_del(&rec->node);
		th_task_free(rec);
	}
	pthread_mutex_unlock(&p->lock);

	free(p);
}

/* Purpose  : Allocate a thread from the thread pool
 * Prameter : *p: the thread pool object
 *            *h: the thread handler
 *            *d: the thread handler data
 *            overtime: the overtime for the thread
 *                      >0: the valid ovetime, unit: second
 *                      -1: ignore the overtime
 *            *qh: the thread handler for quit the thread
 *            *qd: the data for qh.
 * Return   : 0: Error
 *            1: the thread unique ID in the thread pool
 */
static int th_pool_alloc(th_pool_t *p, th_handler_t h, void *d,
			 int overtime, th_handler_t qh, void *qd)
{
	struct list_head *pos;
	lthread_t *rec = NULL;
	int ret = 0, found = 0;

	if (h == NULL) {
		return 0;
	}
	
	pthread_mutex_lock(&p->lock);

	/* Find the free task(STOP)  */
	list_for_each(pos, &p->list) {
		rec = list_entry(pos, lthread_t, node);
		if (rec->stat & LTHREAD_STOP) {
			found = 1;
			break;
		}
	}

	if (!found) {
		rec = th_create_thread(p);
		if (rec == NULL) {
			goto TH_ERR;
		}
	}


	/* Fill the thread record data */
	rec->stat     = LTHREAD_RUN;
	rec->handler  = h;
	rec->data     = d;
	rec->overtime = overtime;
	rec->qhandler = qh;
	rec->qdata    = qd;

	/* Active the thread           */
	ret = rec->no;
	pthread_mutex_unlock(&rec->wtlock);

TH_ERR:
	pthread_mutex_unlock(&p->lock);

	
	return ret;
}

/* Purpose  : Kill the thread
 * Prameter : *p: the thread pool object
 *            no: the thread ID
 * Return   : 0: Failure
 *            1: Success
 */
static int th_pool_kill(th_pool_t *p, int no)
{
	struct list_head *pos;
	lthread_t *rec = NULL;
	
	pthread_mutex_lock(&p->lock);

	/* Find the thread */
	list_for_each(pos, &p->list) {
		rec = list_entry(pos, lthread_t, node);
		if (rec->no == no) {
			th_kill_thread(rec);
			break;
		}
	}

	pthread_mutex_unlock(&p->lock);
	return 1;
}

/* Purpose  : The monitor thread handler
 * Prameter : *arg: the thread handler argument
 * Return   : NULL
 */
static void *th_pool_monitor(void *arg)
{
        MCEMT_DBG("pid is %ld\n", getpid());  
	struct list_head *pos, *tmp;
	lthread_t *rec = NULL;
	th_pool_t *p = (th_pool_t *)arg;
#ifdef MEM_LEAK
	int count = 0;
#endif
        sigset_t mask;
        /* Block thread signals           */
        sigfillset(&mask);
        pthread_sigmask(SIG_BLOCK, &mask, NULL);

	while (1) {	
		/* Update the thread record in the pool  */
		pthread_mutex_lock(&p->lock);
#ifdef MEM_LEAK
		count ++;
		if(!((60+count)%20)){
			printf("thread total:%d\n", total);
  			dbg_mem_stat();
  			dbg_heap_dump("");
		}
#endif
		list_for_each_safe(pos, tmp, &p->list) {
			rec = list_entry(pos, lthread_t, node);
			if (rec->stat & LTHREAD_RUN) {
				th_update_time(p, rec);
			} else {
				th_recycle(p, rec);
			}
		}

		pthread_mutex_unlock(&p->lock);

		if (p->terminate) {
			break;
		} else
			sleep(1);
	}

	pthread_exit(NULL);
}

/* Purpose    : Create a thread
 * Parameters : *p: the thread pool object
 * Return     : NULL: Error
 *              the thread object
 */
static lthread_t *th_create_thread(th_pool_t *p)
{
	lthread_t *tmp = NULL;
/*
	if (p->curnum >= p->maxnum) {
		return NULL;
	}
*/
	/* Create task record       */
	if ((tmp = th_task_alloc()) == NULL) {
		return NULL;
	}
	tmp->stat = LTHREAD_STOP;
	tmp->overtime = -1;
	pthread_mutex_lock(&tmp->wtlock);
	if (pthread_create(&tmp->tid, NULL, th_handler, tmp) != 0) {              
		goto TH_ERR;
	}else      
                  
           

	/* Set the current task No. */
	{
		int maxno = 0;
		struct list_head *pos;
		lthread_t *rec = NULL;
	
		list_for_each(pos, &p->list) {
			rec = list_entry(pos, lthread_t, node);
			if (rec->no > maxno)
				maxno = rec->no;
		}

		tmp->no = maxno+1;
	}
	
	/* Add the task record into pool thread */
	list_add(&tmp->node, &p->list);
	
	p->curnum++;

//	DBG("Create thread %d, tid: %ld", tmp->no, tmp->tid);

	return tmp;

TH_ERR:	
	if (tmp)
		th_task_free(tmp);
	return NULL;
}

/* Purpose    : Kill a thread
 * Parameters : *th: the thread pool object
 * Return     : void
 */
static void th_kill_thread(lthread_t *th)
{
	th->terminate = 1;
	pthread_mutex_unlock(&th->wtlock);
	if (pthread_cancel(th->tid) == 0) {
		pthread_join(th->tid, NULL);
	}
	total--;
	if (th->qhandler)
		th->qhandler(th->qdata);

	th->stat = LTHREAD_EXIT;
	/* Clear thread record */
	th->terminate = 0;
	th->overtime = -1;
	th->tid = 0;
	th->handler = NULL;
	th->data = NULL;
	th->qhandler = NULL;
	th->qdata = NULL;
	
	//pthread_mutex_trylock();
}

/* Purpose    : The thread pool general thread handler
 * Parameters : *arg: the thread handler argument
 * Return     : NULL
 */
static void *th_handler(void *arg)
{       
        MCEMT_DBG("getpid is %ld\n", getpid());  
	lthread_t *th = (lthread_t *)arg;
	
	total++;
	printf("th_handler  on:%x\n", (int)pthread_self());
	while (1){
#if 0
        	/* Change the nice value           */
        	if (nice(-20) == -1) {
                	fprintf(stderr, "Fail: nice(-20)\n");
        	}
#endif
		pthread_mutex_lock(&th->wtlock);

		if (th->terminate) {
			break;
		}
		printf("th_handler  run:%d\n", (int)getpid());
		//sleep(1);
		th->stat = LTHREAD_RUN;
		if (th->handler) {
			th->handler(th->data);
		}

		th->stat = LTHREAD_STOP;
		/* Clear thread record */
		th->overtime = 0;
		th->handler = NULL;
		th->data = NULL;
		th->qhandler = NULL;
		th->qdata = NULL;
	}
	printf("th_handler off:%x\n", (int)pthread_self());

	pthread_exit(NULL);
}

/* Purpose    : Update the running time for thread
 * Parameters : *th: the thread handler
 * Return     : void
 */
static void th_update_time(th_pool_t *p, lthread_t *th)
{
	if (th->overtime == -1) {
		return;
	} else if (th->overtime == 0) {
		th_kill_thread(th);
	} else {
		if (th->overtime > 0)
			th->overtime--;
	}
}

/* Purpose    : Recycle the thread
 * Parameters : *th: the thread handler
 * Return     : void
 */
static void th_recycle(th_pool_t *p, lthread_t *th)
{
	if (th->stat & LTHREAD_EXIT) {
		if (p->curnum <= p->defnum) {
			th->stat = LTHREAD_STOP;
			th->overtime = 0;
			if (pthread_create(&th->tid, NULL, 
					   th_handler, th) != 0) {
				th->stat = LTHREAD_EXIT;
			}
			         
	
		} else {
			list_del(&th->node);
			th_task_free(th);
			p->curnum--;
		}
	} else if (th->stat & LTHREAD_STOP) {
		if (th->overtime >= p->idle_time) {
			if (p->curnum > p->defnum) {
				th_kill_thread(th);
				list_del(&th->node);
				th_task_free(th);
				p->curnum--;
			}
		} else {
			th->overtime++;
		}
	}
}

/* Purpose   : Allocate a new thread task record
 * Parameter : void
 * Return    : NULL: error
 *             the thread task pointer
 */
static lthread_t *th_task_alloc(void)
{
	lthread_t *tmp;
	
	if ((tmp = calloc(1, sizeof(lthread_t))) == NULL) {
		return NULL;
	}
	
	INIT_LIST_HEAD(&tmp->node);
	pthread_mutex_init(&tmp->wtlock, NULL);
	
	return tmp;
}

/* Purpose   : Free the thread task
 * Parameter : *th: the thread task pointer
 * Return    : void
 */
static void th_task_free(lthread_t *th)
{
	free(th);
}

