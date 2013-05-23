/******************************************************************************
 * 
 * File: cgi.c
 *
 * Date: 2007-06-14
 *
 * Author: Wang Songnian
 *
 * Descriptor:
 *   
 * Note:
 *  Create a CGI table
 *
 * Version: 0.1
 *
 * Modified:
 *
 ******************************************************************************/
/*-----------------------------------------------------------------------------
  System header files
------------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>

/*-----------------------------------------------------------------------------
  Local header files
------------------------------------------------------------------------------*/
#include "cgi.h"
#include "ht_def.h"
/*-----------------------------------------------------------------------------
  Data structures
------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
  Global variables and constants
------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
  Functions Declare
------------------------------------------------------------------------------*/
/*
 * CGI environment variables
 */
 static cgi_env_t *cgienv_new(cgi_env_t **env);
cgi_env_t *cgi_env_new(cgi_env_t **env);
static void cgienv_delete(cgi_env_t *env);
static int cgienv_add_val(cgi_env_t *env, const char *n, const char *v);
static char *cgienv_get_val(cgi_env_t *env, const char *n);
static void cgienv_destroy(cgi_env_t *env);

/*
 * CGI variables
 */
cgi_rec_t *cgi_rec_new(cgi_rec_t **rec);
static void cgirec_delete(cgi_rec_t *rec);
static int cgirec_add_val(cgi_rec_t *rec, const char *n, const char *v);
static char *cgirec_get_val(cgi_rec_t *rec, const char *n);
static char *cgirec_get_val_no(cgi_rec_t *rec, const char *n, int no);
static void cgirec_destroy(cgi_rec_t *rec);
static cgi_rec_t *cgirec_new(cgi_rec_t **rc);

/*
 * CGI session
 */
cgi_sess_t *cgisess_new(cgi_sess_t **sess, int expire);
static void cgisess_delete(cgi_sess_t *sess);
static int cgisess_chk_sid(cgi_sess_t *sess, const char *id);
static int cgisess_add_val(cgi_sess_t *sess, const char *n, const char *v);
static int cgisess_alter_val(cgi_sess_t *sess, const char *n, const char *v);
static int cgisess_del_val(cgi_sess_t *sess, const char *n);
static char *cgisess_get_val(cgi_sess_t *sess, const char *n);
static void cgisess_gen_sid(cgi_sess_tab_t *stab, cgi_sess_t *sess);
static cgi_sess_tab_t *cgisess_tab_new(cgi_sess_tab_t **stab);

/*
 * CGI session table
 */
cgi_sess_tab_t *cgi_sess_tab_new(cgi_sess_tab_t *stab);
static void cgisess_tab_delete(cgi_sess_tab_t *st);
static cgi_sess_t *cgisess_tab_create(cgi_sess_tab_t *st, char *ssid);
static void cgisess_tab_free(cgi_sess_tab_t *st, cgi_sess_t *s);
static cgi_sess_t *cgisess_tab_find(cgi_sess_tab_t *st, const char *sid);
static int cgisess_tab_set_expire(cgi_sess_tab_t *st, int expire);
static void cgisess_tab_chk_expire(cgi_sess_tab_t *st, int sec);

/*
 * CGI functions
 */
cgi_t *cgi_new(cgi_t **cgi);
static void cgi_delete(cgi_t *cgi);
static void cgi_set_sess(cgi_t *cgi, cgi_sess_t *s);
static int cgi_init(cgi_t *cgi);
static void cgi_end(cgi_t *cgi);
static int cgi_form(cgi_t *cgi, const char *dname, int *per);
//static char *cgi_form_write_file(cgi_t *cgi, int dfd, char *start, 
//				 const char *bound,
//				 unsigned long long *alen, int *per);
static char *cgi_form_read_fvar(cgi_t *cgi, const char *vname);
static int cgi_init_header(cgi_t *cgi, unsigned long long len, int type);
static int cgi_redirect(cgi_t *cgi, const char *url);
static int cgi_sess_start(cgi_t *cgi);
static int cgi_sess_destroy(cgi_t *cgi);
static int cgi_sess_output(cgi_t *cgi);
static int cgi_printf(cgi_t *cgi, const char *fmt, ...);
static int cgi_vprintf(cgi_t *cgi, const char *fmt, va_list ap);
static int cgi_output(cgi_t *cgi, const char *buf, int len);
static int cgi_flush(cgi_t *cgi);
static void cgi_clear(cgi_t *cgi);

static void cgi_init_parse_env(cgi_t *cgi);
static void cgi_init_parse_rec(cgi_t *cgi);

/*
 * CGI table functions
 */
static void cgi_tab_delete(cgi_tab_t *ctab);
static cgi_t *cgi_tab_alloc(cgi_tab_t *ctab);
static void cgi_tab_free(cgi_tab_t *ctab, cgi_t *cgi);

/*
 * Other
 */
static int cgi_add_val(struct list_head *head, const char *n, const char *v);
static char *cgi_get_val(struct list_head *head, const char *n);
static void cgi_destroy(struct list_head *head);

/*-----------------------------------------------------------------------------
  Functions
------------------------------------------------------------------------------*/
/* Purpose: Create cgi environment variables list object
 * In     : **env: the pointer to store the object
 * Return : NULL: error
 *          the object pointer
 */
static cgi_env_t *cgienv_new(cgi_env_t **env)
{
	cgi_env_t *tmp;

/* Allocate memory space  */
	if ((tmp = calloc(1, sizeof(cgi_env_t))) == NULL) {
		return NULL;
	} else {
		*env = tmp;
		INIT_LIST_HEAD(&tmp->head);
	}

/* Initialize basic ways */
	tmp->delete = cgienv_delete;
	tmp->add_val = cgienv_add_val;
	tmp->get_val = cgienv_get_val;
	tmp->destroy = cgienv_destroy;

	return tmp;
}

/* Purpose: Destroy cgi environment variables list object
 * In     : *env: the object pointer
 * Return : void
 */
static void cgienv_delete(cgi_env_t *env)
{
        if (env == NULL) {
		return;
        }
	env->destroy(env);
	free(env);
}

/* Purpose: Add variables record into environment variables list
 * In     : *env: the object pointer
 *          *n: the variable name
 *          *v: the variable value
 * Return : 0: error
 *          1: ok
 */
static int cgienv_add_val(cgi_env_t *env, const char *n, const char *v)
{
	if (cgi_add_val(&env->head, n, v)) {
		env->num++;
		return 1;
        } else {
		return 0;
}
}

/* Purpose: Get the value of a environment variable
 * In     : *env: the object pointer
 *          *n: the environment variable name
 * Return : NULL: error
 *          the value of the environment variable
 */
static char *cgienv_get_val(cgi_env_t *env, const char *n)
{
	return cgi_get_val(&env->head, n);
}

/* Purpose: Delete all environment variables
 * In     : *env: the object pointer
 * Return : void
 */
static void cgienv_destroy(cgi_env_t *env)
{
	/* Free the head list  */
	cgi_destroy(&env->head);
}

/* Purpose: Create cgi record variables list object
 * In     : **rc: the pointer to store the object
 * Return : NULL: error
 *          the object pointer
 */
static cgi_rec_t *cgirec_new(cgi_rec_t **rc)
{
	cgi_rec_t *tmp;

	tmp = (cgi_rec_t *)calloc(1, sizeof(cgi_rec_t));
	if (tmp == NULL) {
		return NULL;
	} else {
		*rc = tmp;
		INIT_LIST_HEAD(&tmp->head);
	}
	
/* Intialize ways */
	tmp->delete = cgirec_delete;
	tmp->add_val = cgirec_add_val;
	tmp->get_val = cgirec_get_val;
	tmp->get_val_no = cgirec_get_val_no;
	tmp->destroy = cgirec_destroy;

	return tmp;
}

/* Purpsoe: Delete cgi record list object
 * In     : *rc: the cgi record pointer
 * Return : NULL: error
 *          the object pointer
 */
static void cgirec_delete(cgi_rec_t *rc)
{
	if (rc) {
		rc->destroy(rc);
		free(rc);
	}
}

/* Purpose: Add a cgi record into list
 * In     : *rc:the cgi record pointer
 *        : *n: the cgi variable name
 *          *v: the cgi variable value
 * Return : 0: error
 *          1: ok
 */
static int cgirec_add_val(cgi_rec_t *rc, const char *n, const char *v)
{
	if (cgi_add_val(&rc->head, n, v)) {
		rc->num++;
		return 1;
        } else {
		return 0;
}
}

/* Purpose: Get a variable value of variable name
 * In     : *rc: the record list object
 *          *n: the cgi variable name
 * Return : NULL: error
 *          the address of the variable value
 */
static char *cgirec_get_val(cgi_rec_t *rc, const char *n)
{
	return cgi_get_val(&rc->head, n);
}

/* Purpose: Get a variable value of the no-th's variable name
 * In     : *rc: the record list object
 *          *n: the cgi variable name
 *          no: the unique ID, from 1 .. n
 * Return : NULL: error
 *          the address of the variable value
 */
static char *cgirec_get_val_no(cgi_rec_t *rc, const char *n, int no)
{
	cgi_val_t *rec;
	struct list_head *pos;
	int i = 0;

        if ((n == NULL) || (no <= 0)) {
		return NULL;
        }

	list_for_each(pos, &rc->head) {
		rec = list_entry(pos, cgi_val_t, node);
		if (strcmp(rec->name, n) == 0) {
			i++;
			if (i == no) {
				return rec->val;
			}
		}
	}

	return NULL;
}

/* Purpose: Delete all variable records
 * In     : *rc: the record list object
 *          *n: the cgi variable name
 * Return : void
 */
static void cgirec_destroy(cgi_rec_t *rc)
{
	return cgi_destroy(&rc->head);
}

/* Purpose: Create cgi session variables list object
 * In     : **sess: the pointer to store the object
 *          expire: the expire time
 * Return : NULL: error
 *          the object pointer
 */
cgi_sess_t *cgisess_new(cgi_sess_t **sess, int expire)
{
	cgi_sess_t *tmp;

        if (expire < 0) {
		return NULL;
        }

/* Allocate memory space   */
	tmp = (cgi_sess_t *)calloc(1, sizeof(cgi_sess_t));
	if (tmp == NULL) {
		return NULL;
	} else {
		*sess = tmp;
		INIT_LIST_HEAD(&tmp->node);
		INIT_LIST_HEAD(&tmp->head);
	}

	/* Set expire time */
	tmp->expire = expire;
	
/* Initialize ways         */
	tmp->delete = cgisess_delete;
	tmp->chk_sid = cgisess_chk_sid;
	tmp->add_val = cgisess_add_val;
	tmp->alter_val = cgisess_alter_val;
	tmp->del_val = cgisess_del_val;
	tmp->get_val = cgisess_get_val;

	return tmp;
}

/* Purpose: Delete the cgi session variable list object
 * In     : *sess: the session object pointer
 * Return : void
 */
static void cgisess_delete(cgi_sess_t *sess)
{
	if (sess) {
		cgi_val_t *rec, *tmp;

		//sessionfile_del_item(sess->sessid);

		if (!list_empty(&sess->head)) {
			list_for_each_entry_safe(rec, tmp, &sess->head, node) {
				list_del(&rec->node);
				if (rec->name != NULL)
					free(rec->name);
				if (rec->val != NULL)
					free(rec->val);
				free(rec);
			}
		}
		free(sess);
	}
}

/* Purpose: Check session ID
 * In     : *sess: the session object pointer
 *          *id: the session ID
 * Return : 0: different
 *          1: same
 */
static int cgisess_chk_sid(cgi_sess_t *sess, const char *id)
{
	if (strcmp(sess->sessid, id) == 0)
		return 1;
	else
		return 0;
}

/* Purpose: Register a session variable
 * In     : *sess: the session object pointer
 *          *n: the session variable name
 *          *v: the value of session variable name
 * Return : 0: error
 *          1: ok
 */
static int cgisess_add_val(cgi_sess_t *sess, const char *n, const char *v)
{
	char *name = NULL;
	char *val = NULL;

        if ((n == NULL) || (v == NULL)) {
		return 0;
        }

	/* Allocate space for name    */
        if ((name = strdup(n)) == NULL) {
		goto ADD_ERR;
        }

	/* Allocate space for value  */
        if ((val = strdup(v)) == NULL) {
		goto ADD_ERR;
        }
	if (!cgi_add_val(&sess->head, name, val)) {
		goto ADD_ERR;
	}
	
	return 1;

ADD_ERR:
	if (name != NULL)
		free(name);
	if (val != NULL)
		free(val);
	return 0;
}

/* Purpose: Alter the value of the session variable,
 *          Only alter the shorter than old string.
 * In     : *sess: the session object pointer
 *          *n: the session variable name
 *          *v: the session variable value
 * Return : 0: error
 *          1: ok
 */
static int cgisess_alter_val(cgi_sess_t *sess, const char *n, const char *v)
{
	struct list_head *pos;
	cgi_val_t *rec;

        if ((n == NULL) || (v == NULL)) {
                return 0;
        }

	/* Find the session vairable          */
	list_for_each(pos, &sess->head) {
		rec = list_entry(pos, cgi_val_t, node);
		if (strcmp(rec->name, n) == 0) {
			char *val;

                        if ((val = strdup(v)) == NULL) {
				return 0;
                        }
			if (rec->val != NULL)
				free(rec->val);
			rec->val = val;
			return 1;
		}
	}

	return 0;
}

/* Purpose: Unregister a session variable
 * In     : *sess: the session object pointer
 *          *n: the session variable name
 * Return : 0: error
 *          1: ok
 */
static int cgisess_del_val(cgi_sess_t *sess, const char *n)
{
	cgi_val_t *rec;

        if ((sess == NULL) || (n == NULL)) {
                return 0;
        }

	/* Find the session variable        */
	if (!list_empty(&sess->head)) {
		list_for_each_entry(rec, &sess->head, node) {
			if (strcmp(rec->name, n) == 0) { /* Found  */
				list_del(&rec->node);
				if (rec->name != NULL)
					free(rec->name);
				if (rec->val != NULL)
					free(rec->val);
				free(rec);
				return SUCCESS;
			}
		}
	}
	
	return FAILURE;
}

/* Purpose: Get the value of the session variable
 * In     : *sess: the session object pointer
 *          *n: the session variable name
 * Return : NULL: error
 *          the value of session variable name
 */
static char *cgisess_get_val(cgi_sess_t *sess, const char *n)
{
	return cgi_get_val(&sess->head, n);
}

/* Purpose: Generate a session ID
 * In     : *stab: the session table object
 *          *sess: the current session object
 * Return : void
 */
static void cgisess_gen_sid(cgi_sess_tab_t *stab, cgi_sess_t *sess)
{
	char *table = "123456789abcdefghijlmnopqrstuvxzwyABCDEFGHIJLMOPQRSTUVXZYW";
	unsigned int len = strlen(table);
	char sbuf[SESS_ID_LEN+1];
	int flag;
	register int i;

        if ((stab == NULL) || (sess == NULL)) {
		return;
        }
	do {
		struct list_head *pos;
		cgi_sess_t *se;

		flag = 0;
		/* Generate a session ID  */
		for (i = 0; i < SESS_ID_LEN; i++) {
			sbuf[i] = table[rand()%len];
		}
		sbuf[SESS_ID_LEN] = '\0';

		/* Check if the session ID is unique  */
		list_for_each(pos, &stab->head) {
			se = list_entry(pos, cgi_sess_t, node);
			if (strcmp(sbuf, se->sessid) == 0)
				flag = 1;
		}
	} while (flag);

	/* Set the new generated session ID */
	strcpy(sess->sessid, sbuf);
	printf("sessid:%s\n", sbuf);
}

/* Purpose: Create cgi session table(list) object
 * In     : **stab: the pointer to store the object
 * Return : NULL: error
 *          the object pointer
 */
static cgi_sess_tab_t *cgisess_tab_new(cgi_sess_tab_t **stab)
{
	cgi_sess_tab_t *tmp;

/* Allocate session table object */
	tmp = (cgi_sess_tab_t *)calloc(1, sizeof(cgi_sess_tab_t));
	if (tmp == NULL) {         
		return NULL;
	} else {
		*stab = tmp;
	}
	INIT_LIST_HEAD(&tmp->head);
	pthread_mutex_init(&tmp->lock, NULL);

/* Initialize ways               */
	tmp->delete     = cgisess_tab_delete;
	tmp->new        = cgisess_tab_create;
	tmp->cfree      = cgisess_tab_free;
	tmp->find       = cgisess_tab_find;
	tmp->set_expire = cgisess_tab_set_expire;
	tmp->chk_expire = cgisess_tab_chk_expire;

	return tmp;
}

/* Purpose: Destroy cgi session table object
 * In     : *st: the session table object
 * Return : void
 */
static void cgisess_tab_delete(cgi_sess_tab_t *st)
{
	cgi_sess_t *rec, *tmp;

	pthread_mutex_lock(&st->lock);

	if (!list_empty(&st->head)) {
		list_for_each_entry_safe(rec, tmp, &st->head, node) {
			list_del(&rec->node);
			rec->delete(rec);
		}
	}
	
	pthread_mutex_unlock(&st->lock);
	free(st);
}

/* Purpose: Create a new session
 * In     : *st: the session table object
 *          *ssid: session ID
 * Return : NULL: error
 *          the valid new session pointer
 */
static cgi_sess_t *cgisess_tab_create(cgi_sess_tab_t *st, char *ssid)
{
	cgi_sess_t *sess;
	
	if ((sess = cgisess_new(&sess, st->expire)) == NULL) {
		return NULL;
	}

	/* Set a unique ID for current session  */
	if (ssid == NULL) {
		cgisess_gen_sid(st, sess);
	} else {
		strcpy(sess->sessid, ssid);
	}
	/* Add the session into session list    */
	pthread_mutex_lock(&st->lock);

	list_add(&sess->node, &st->head);

	pthread_mutex_unlock(&st->lock);

	return sess;
}

/* Purpose: Free a session
 * In     : *st: the session table object
 *          *s: the session object
 * Return : 0: error
 *          1: ok
 */
static void cgisess_tab_free(cgi_sess_tab_t *st, cgi_sess_t *s)
{
	cgi_sess_t *rec;

        if ((st == NULL) || (s == NULL)) {
		return;
        }
	
	pthread_mutex_lock(&st->lock);

	if (!list_empty(&st->head)) {
		list_for_each_entry(rec, &st->head, node) {
			if (rec == s) {
				list_del(&rec->node);
				rec->delete(rec);
				break;
			}
		}
	}

	pthread_mutex_unlock(&st->lock);
}

/* Purpose: Find the session according to session ID
 * In     : *st: the session table object
 *          *sid: the session ID
 * Return : NULL: error
 *          other: the valid session pointer
 */
static cgi_sess_t *cgisess_tab_find(cgi_sess_tab_t *st, const char *sid)
{
	struct list_head *pos;
	cgi_sess_t *rec;
	
        if ((st == NULL) || (sid == NULL)) {
                return NULL;
        }
	pthread_mutex_lock(&st->lock);

	list_for_each(pos, &st->head) {
		rec = list_entry(pos, cgi_sess_t, node);
		if (rec->chk_sid(rec, sid)) {
			pthread_mutex_unlock(&st->lock);
			return rec;
		}
	}

	pthread_mutex_unlock(&st->lock);
        
	return NULL;
}

/* Purpose: Set the expiring time
 * In     : *st: the session table object
 *          expire: the expire time
 * Return : 1: SUCCESS
 *          0: FAILURE
 */
static int cgisess_tab_set_expire(cgi_sess_tab_t *st, int expire)
{
	cgi_sess_t *rec;
	
        if ((st == NULL) || (expire <= 0)) {
                return 0;
        }
	pthread_mutex_lock(&st->lock);

	st->expire = expire;
	if (!list_empty(&st->head)) {
		list_for_each_entry(rec, &st->head, node) {
			if (rec->expire > expire)
				rec->expire = expire;
		}
	}
	pthread_mutex_unlock(&st->lock);
        return 1;
}

/* Purpose: Check the expiring session
 * In     : *st: the session table object
 *          sec: passed the number of seconds
 * Return : void
 */
static void cgisess_tab_chk_expire(cgi_sess_tab_t *st, int sec)
{
	cgi_sess_t *rec, *tmp;

        if ((st == NULL) || (sec <= 0)) {
		return;
        }
	pthread_mutex_lock(&st->lock);

	if (!list_empty(&st->head)) {
		list_for_each_entry_safe(rec, tmp, &st->head, node) {
			if (rec->expire <= sec) {
				list_del(&rec->node);
				rec->delete(rec);
			} else
				rec->expire -= sec;
		}
	}

	pthread_mutex_unlock(&st->lock);
}

/* Purpose: Create cgi record object
 * In     : **cgi: the pointer to store the CGI record object
 * Return : NULL: error
 *          the cgi pointer
 */
cgi_t *cgi_new(cgi_t **cgi)
{
	cgi_t *tmp;

/* Allocate cgi object memory space*/
        if ((tmp = (cgi_t *)calloc(1, sizeof(cgi_t))) == NULL) {
		return NULL;
        } else {
		*cgi = tmp;
		INIT_LIST_HEAD(&tmp->node);
	}
	tmp->sockfd = -1;
	
/* Initialize the data             */
	/* Create a cgienv object  */
	if ((tmp->cgienv = cgienv_new(&tmp->cgienv)) == NULL) {
		goto CGI_ERR;
	}

	/* Create a cgirec object */
	if ((tmp->cgirec = cgirec_new(&tmp->cgirec)) == NULL) {
		goto CGI_ERR;
	}

/* Initialize ways               */
	tmp->delete = cgi_delete;
	tmp->set_sess = cgi_set_sess;

	tmp->cgi_init = cgi_init;
	tmp->cgi_end = cgi_end;
	tmp->cgi_parse_rec = cgi_init_parse_rec;
	tmp->cgi_parse_env = cgi_init_parse_env;
	tmp->cgi_form = cgi_form;
	tmp->cgi_read_fvar = cgi_form_read_fvar;
	tmp->cgi_init_header = cgi_init_header;
	tmp->cgi_redirect = cgi_redirect;
	
	tmp->sess_start = cgi_sess_start;
	tmp->sess_destroy = cgi_sess_destroy;
	tmp->sess_output = cgi_sess_output;

	tmp->printf = cgi_printf;
	tmp->vprintf = cgi_vprintf;
	tmp->output = cgi_output;
	tmp->flush = cgi_flush;

	return tmp;

CGI_ERR:
	if (tmp) {
		cgi_delete(tmp);
	}
	return NULL;
}

/* Purpose: delete cgi object
 * In     : *cgi: cgi object pointer
 * Return : void
 */
static void cgi_delete(cgi_t *cgi)
{
	if (cgi != NULL) {
                if ((cgi->flag & CGI_USED) && (cgi->sockfd != -1)) {
			close(cgi->sockfd);
                }

		if (cgi->cgirec != NULL)
			cgi->cgirec->delete(cgi->cgirec);
		if (cgi->cgienv != NULL)
			cgi->cgienv->delete(cgi->cgienv);
		
		free(cgi);
	}
}

/* Purpose: set session cgi object
 * In     : *cgi: cgi object pointer
 * Return : void
 */
static void cgi_set_sess(cgi_t *cgi, cgi_sess_t *s)
{
	cgi->sess = s;
}

/* Purpose: initialize the current cgi to start new cgi
 *          1. initialize cgi environment variable list
 *          2. initialize cgi record variable list
 * In     : *cgi: cgi object
 * Return : 0: error
 *          1: ok
 */
static int cgi_init(cgi_t *cgi)
{
	/* 1. Parse cgi environment string        */
	cgi_init_parse_env(cgi);

	/* 2. Parse cgi record string             */
	if (cgi->flag & (CGI_PUTFILE | CGI_BUF_OVERFLOW))
		return 1;
	
	cgi_init_parse_rec(cgi);

	return 1;
}

/* Purpose: parse environment string
 * Note   : the environment string format:
 *              name=value name=value
 * In     : cgi object
 * Return : void
 */
static void cgi_init_parse_env(cgi_t *cgi)
{
	cgi_env_t *env;
	char *vname, *vval = NULL, *end;
	
        if (cgi->envlen == 0) {
		return;
        }
	env = cgi->cgienv;
	end = vname = cgi->envbuf;
	
	while (*end != '\0') {
		vval = strchr(vname, '=');
		if (vval == NULL)
			break;
		else
			*vval++ = '\0';

		end = strchr(vval, '\t');
		if (end == NULL)
			break;
		else
			*end++ = '\0';

		/* Add the vname and vval into cgi_env_t */
		env->add_val(env, vname, vval);

		/* Next variable  */
		vname = end;
	}
	cgi->envlen = 0;

	/* If isn't cgi content type, return */
	if ((vval = env->get_val(env, CGI_CONTENT_TYPE)) != NULL) {
		/* If cgi content type isn't
		 * "application/x-www-form-urlencoded", return
		 */
		if (strncmp(vval, HTTP_CONT_APP_TYPE,
			    strlen(HTTP_CONT_APP_TYPE)) != 0) {
			if (strncmp(vval, HTTP_CONT_MULTIPART_TYPE,
				    strlen(HTTP_CONT_MULTIPART_TYPE)) == 0) {
				cgi->flag |= CGI_PUTFILE;
			}
		}
	}

	/* If the content length is more than buffer length  */
	if ((vval = env->get_val(env, CGI_CONTENT_LENGTH)) != 0) {
		unsigned long long len = atoll(vval);

		if (len >= CGI_RCVBUF_LEN) {
			cgi->flag |= CGI_BUF_OVERFLOW;
		}
	}
}

/* Purpose: parse record string
 * Note   : the record string format:
 *              name=value&name=value
 * In     : cgi object
 * Return : 0: error
 *          1: ok
 */
static void cgi_init_parse_rec(cgi_t *cgi)
{
	cgi_rec_t *rec;
	char *start, *end;
	
        if (cgi->reclen == 0) {
		return;
        }
	if (cgi->recbuf[cgi->reclen-1] != '&') {
		cgi->recbuf[cgi->reclen++] = '&';
		cgi->recbuf[cgi->reclen] = '\0';
	}

	/* Combine the values of same field */
	rec = cgi->cgirec;
	end = start = cgi->recbuf;
	while (*start != '\0') {
		char *vname, *vval;
		
		/* Get a cgi variable       */
		if ((end = strchr(start, '&')) == NULL)
			break;
		else
			*end = '\0';
		
		/* Get the name and value   */
		vname = start;
		if ((vval = strchr(vname, '=')) != NULL) {
			*vval++ = '\0';
			str_decode_url(vval, strlen(vval), 
				       vval, strlen(vval)+1);
			/*
			{
				//only for cgi_form parse filename with '&'
				char *tmp;
				int i = 0;
				tmp = vval;
				while(1){
					if(tmp[i] == '\0')
						break;
					if(tmp[i] == 0xfe){
						printf("!!!!!!!!!!!!!!!!!\n");
						tmp[i] = '&';
					}
					i++;
				}
			}
			*/
			/* Add the vname and vval into cgi_rec_t */
			rec->add_val(rec, vname, vval);
		} /* else { ignore } */
		
		start = end + 1;
	}

	cgi->reclen = 0;
}

/* Purpose: finish the current cgi to end new cgi
 *          1. destroy cgi environment variable list
 *          2. destroy cgi record vairable list
 * In     : *cgi: cgi object
 * Return : void
 */
static void cgi_end(cgi_t *cgi)
{
	cgi->flush(cgi);

	/* 1. destroy cgi environment list */
	cgi->cgienv->destroy(cgi->cgienv);

	/* 2. destroy cgi record list      */
	cgi->cgirec->destroy(cgi->cgirec);

	close(cgi->sockfd);
	cgi->sockfd = -1;
}

static char *cgi_test_write_file(cgi_t *cgi, int dfd, char *start, const char *bound, unsigned long long *alen, int *per)
{
	char *e = NULL;
        int flen, num, cnt,base = 0;
        char boundary[64],tmpbuf[64];
 
        /* add "\r\n--" before boundary */
        memset(boundary, 0, 64);
        sprintf(boundary, "%s%s", "\r\n--", bound);
        /* Write file              */
        cnt = (*alen / CGI_RCVBUF_LEN) / 100;
        num = 0;
        do {
                flen = &cgi->recbuf[cgi->reclen] - start;
 
                {
                        char *s = start;
                        int leftlen,boundlen;
                        boundlen = strlen(boundary);
                        while (s < ((char *)start + flen)) {
                                if (*s == *((char *)boundary)) {
                                        leftlen = flen - (s - start);
                                        if(boundlen <= leftlen){
                                                if(memcmp(s, boundary, boundlen) == 0){
                                                        e = s;
                                                        break;
                                                }
                                        }else{
                                                memcpy(tmpbuf, s, leftlen);
                                                flen -= leftlen;
                                                base = leftlen;
                                                break;
                                        }
                                }
                                s++;
                        }
                }

		if (e) {
                        flen = e - start;
                }
                if (write_n(dfd, start, flen) != flen) {
                        return NULL;
                }
 
                if (e == NULL) {
                        int len, rlen;
 
                        if (*alen <= 0) {
                                start = cgi->recbuf;
                                cgi->recbuf[0] = '\0';
                                cgi->reclen = 0;
                                break;
                        }
                        memcpy(cgi->recbuf, tmpbuf, base);
                        rlen = (*alen < (CGI_RCVBUF_LEN - 1)) ?
                                *alen : (CGI_RCVBUF_LEN - 1);
                        len = socket_read_anylen(cgi->sockfd, cgi->recbuf+base,
                                         (CGI_RCVBUF_LEN - 1)-base, NULL, 3);
                        if (len <= 0) {
                                return NULL;
                        }
                        cgi->reclen = len + base;
                        start = cgi->recbuf;
                        *alen -= len ;
                        base = 0;
                        /* Update percentage  */
                        if (num >= cnt) {
                                num = 0;
                                (*per)++;
                        } else
                                num++;
                } else {
                        start = e;
                        break;
                }
        } while(1);
        
 	if(strncmp(start, "\r\n--", 4) == 0)
		start += 4;

        /* Move the data to the begin of the buffer */
        flen = &cgi->recbuf[cgi->reclen] - start;
        memcpy(cgi->recbuf, start, flen);
	 memset(&cgi->recbuf[flen], 0, (CGI_RCVBUF_LEN-flen));
	 if(flen != 0)
        	cgi->reclen = flen;
 
        return cgi->recbuf;
}

/* Purpose: handle putting file form
 * In     : *cgi: the cgi object
 *          *dpath: download path
 *          *per: the percentage to transfer file
 * Return : FAILURE: 0
 *          SUCCESS: 1
 */
static int cgi_form(cgi_t *cgi, const char *dpath, int *per)
{
	cgi_env_t *env;
	char bound[LINE_MAX], dbuf[CGI_RCVBUF_LEN], field[LINE_MAX];
	char *s, *e;
	unsigned long long alllen;
	int pos, dfd = -1;

        if ((dpath == NULL) || (per == NULL)) {
                return 0;
        }

	/* Get the boudary string  */
	env = cgi->cgienv;
	if ((s = env->get_val(env, CGI_CONTENT_TYPE)) == NULL) {
                return 0;
	}
	if ((e = strchr(s, ';')) == NULL) {
                return 0;
	} else {
		*e = '\0';
	}
	if (strcmp(s, HTTP_CONT_MULTIPART_TYPE) != 0) {
                return 0;
	} else {
		s = e + 2;
	}
	if (strncmp(s, "boundary", 8) != 0) {
                return 0;
	} else {
		strcpy(bound, s+9); /* Move the next character of '=' */
	}
	
	/* Get the data length   */
	s = env->get_val(env, CGI_CONTENT_LENGTH);
	if (s == NULL) {
                return 0;
	}
	alllen = atoll(s);

	/* Parse data            */
	dbuf[0] = '\0';
	pos = 0;

	if (cgi->reclen >= alllen)
		alllen = 0;
	else
		alllen -= cgi->reclen;

	s = cgi->recbuf;
	s += 2;  /* Skip '--'   */

	dfd = -1;
	while (1) {
		/* If the data less than boundary length,
		 * read more data from socket
		 */
		if ((cgi->reclen < 512) && (alllen > 0)) {
			int len;
			int rlen;

			rlen = (CGI_RCVBUF_LEN - cgi->reclen - 1);
			rlen = (alllen < rlen) ? alllen : rlen;

			len = socket_read(cgi->sockfd,
					  &cgi->recbuf[cgi->reclen], 
					  rlen, NULL, 3);
			if (len < 0) {
				goto CGI_ERR;
			}
			cgi->reclen += len;
			cgi->recbuf[cgi->reclen] = '\0';

			alllen -= len;
		}
		//printf("123:%d\n", cgi->reclen);
		if (strncmp(s, bound, strlen(bound)) != 0)
			break;
		
		/* Parse the multi-data protocol    */
		s += strlen(bound) + 2; /* Skip boundary string and "\r\n" */
		if (strncmp(s, HTTP_CONT_DISPOS_EHDR,
			    strlen(HTTP_CONT_DISPOS_EHDR)) != 0) {
			break; /* the last boundary */
		} else
			s += strlen(HTTP_CONT_DISPOS_EHDR) + 2;
		if (strncmp(s, HTTP_CONT_DISPOS_FORMDATA,
			    strlen(HTTP_CONT_DISPOS_FORMDATA)) != 0) {
			goto CGI_ERR;
		} else
			s += strlen(HTTP_CONT_DISPOS_FORMDATA) + 2;

		/* Get the "name" field       */
		if (strncmp(s, "name=", 5) != 0) {
			goto CGI_ERR;
		} else
			s += 5 + 1; /* Skip 'name="' */
		if ((e = strchr(s, '\"')) == NULL) {
			goto CGI_ERR;
		} else
			*e = '\0';

		pos += sprintf(&dbuf[pos], "%s=", s);
		if (*(e+1) == '\r') { /* Parse non-file form field */
			s = e + 5;    /* Skip ("\r\n\r\n) string   */
			if ((e = strchr(s, '\r')) == NULL) {
				goto CGI_ERR;
			} else
				*e = '\0';
			pos += sprintf(&dbuf[pos], "%s&", s);
			s = (e + 4); /* Skip \r\n--  */
			continue;
		}

		/* Parse file field          */
		s = e + 3;     /* Skip '"; ' */
		if (strncmp(s, "filename=", 9) != 0) {
			goto CGI_ERR;
		} else
			s += 9 + 1; /* Skip 'filename=' */
		if ((e = strchr(s, '\"')) == NULL) {
			goto CGI_ERR;
		} else
			*e = '\0';
	
		/* Create tmp file          */
		str_decode_url(s, strlen(s), s, strlen(s)+1);
		{
			char *t;
			
			/* If In windows and use IE browse, only 
			 * get the file name, kill the path
			 */
			t = env->get_val(env, CGI_REMOTE_IDENT);
			if (strstr(t, "Windows") &&
			    (strstr(t, "Firefox") == NULL)) {
				t = strrchr(s, '\\');
				if (t != NULL)
					s = t + 1;
			}
		}
		{
			char *tmp_buf = NULL;
			str_encode_url(s, strlen(s), &tmp_buf);
			if(tmp_buf != NULL){
				pos += sprintf(&dbuf[pos], "%s&", tmp_buf);
				free(tmp_buf);
			}else
				pos += sprintf(&dbuf[pos], "%s&", s);
		}
		
		sprintf(field, "%s/%s", dpath, s);
		dfd = open(field, O_RDWR|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
		if (dfd == -1) {
			goto CGI_ERR;
		}
		s = e + 1;
		s = strstr(s, "\r\n\r\n"); /* Skip "Content-Type" */
		if (s == NULL) {
			goto CGI_ERR;
		} else
			s += 4; /* Skip \r\n\r\n */

		/* Write file               */
		s = cgi_test_write_file(cgi, dfd, s, bound, &alllen, per);
		if (s == NULL) {
			goto CGI_ERR;
		}

		if(alllen){
			int flen = 0;
			if(alllen >= (CGI_RCVBUF_LEN - 1-cgi->reclen)){
				goto CGI_ERR;
			}
	
			flen =socket_read_anylen(cgi->sockfd, &cgi->recbuf[cgi->reclen],
                                         alllen, NULL, 3);
			if(flen != alllen){
				goto CGI_ERR;
			}
			cgi->reclen += flen;
			alllen = 0;
		}
	//	printf("end\n");
	}
	
        if (dfd != -1) {
		close(dfd);
        }

	/* Parse cgi record    */
	strcpy(cgi->recbuf, dbuf);
	cgi->reclen = pos;
	
	cgi->cgi_parse_rec(cgi);
        return 1;

CGI_ERR:
        if (dfd != -1) {
		close(dfd);
}
        return 0;
}

/* Purpose: Read the form variable that the length is more than 1 cgi buffer
 * Note   : the function allocate memory space for returnning string,
 *          the caller function need free them
 * In     : *cgi: the cgi object
 *        : *vname: the variable name
 * Return : NULL: Error
 *          the variable value string
 */
static char *cgi_form_read_fvar(cgi_t *cgi, const char *vname)
{
	char *tmpbuf = NULL, *s, *cgis, *e, *vval = NULL;
	int alllen, rlen, len;
	cgi_env_t *env = cgi->cgienv;

	/* Get the data length     */
	s = env->get_val(env, (const char *)CGI_CONTENT_LENGTH);
	if (s == NULL) {
		return NULL;
	}
	alllen = atoll(s);

	/* Get the whole form data */
	tmpbuf = (char *)malloc(alllen + 1);
	if (tmpbuf == NULL)
		return NULL;
	memcpy(tmpbuf, cgi->recbuf, cgi->reclen);
	rlen = alllen - cgi->reclen;

	len = socket_read(cgi->sockfd, &tmpbuf[cgi->reclen], 
			  rlen, NULL, 3);
	if (len < 0) {
		goto CGI_ERR;
	} else
		tmpbuf[alllen] = '&';
	/* Parse the whole form data */
	cgi->reclen = 0;
	s = tmpbuf;
	cgis = cgi->recbuf;
	e = strchr(s, '&');
	while (e != NULL) {
		char *tstr;
		if ((tstr = strchr(s, '=')) == NULL) {
			goto CGI_ERR;
		} else
			*tstr = '\0';
		if (strcmp(s, vname) == 0) {
			rlen = e - tstr;
			vval = calloc(rlen+1, sizeof(char));
			if (vval == NULL)
				goto CGI_ERR;
			memcpy(vval, tstr+1, rlen-1);
			vval[rlen-1] = '\0';
		} else {
			*tstr = '=';
			rlen = e - s + 1;
			memcpy(cgis, s, rlen);
			cgis += rlen;
			cgi->reclen += rlen;
		}
		s = e + 1;
		e = strchr(s, '&');
	}
	cgi->cgi_parse_rec(cgi);

	if (tmpbuf)
		free(tmpbuf);

	str_decode_url(vval, strlen(vval), vval, strlen(vval)+1);
	return vval;

CGI_ERR:
	if (tmpbuf)
		free(tmpbuf);
	if (vval)
		free(vval);
	return NULL;
}

/* Purpose: Output cgi header to web client
 * In     : *cgi: the cgi object
 *        :  len: the HTML content length don't include HTML header
 *           type: 
 *          #define CGI_HEADER_TYPE_HTML           (1)
 *          #define CGI_HEADER_TYPE_XML            (2)
 * Return : 0: Error
 *          1: Ok
 */
static int cgi_init_header(cgi_t *cgi, unsigned long long len, int type)
{
	if (len < 0)
		len = 0;

	/* Output the cgi header */
	if (!(cgi->flag & CGIH_INIT)) {
                if (!cgi->printf(cgi, "HTTP/1.1 200 OK\r\n")) {
			goto CGI_ERR;
                }
                if (!cgi->printf(cgi, "Server: vshttpd\r\n")) {
			goto CGI_ERR;
                }
                if (!cgi->printf(cgi, "Cache-Control: no-cache\r\n")) {
			goto CGI_ERR;
                }
                if (!cgi->printf(cgi, "Pragma: no-cache\r\n")) {
			goto CGI_ERR;
                }
                if (!cgi->printf(cgi, "Expires: 0\r\n")) {
			goto CGI_ERR;
                }
                if (!cgi->printf(cgi, "Content-length: %lld\r\n", len)) {
			goto CGI_ERR;
                }
		if (type == CGI_HEADER_TYPE_XML) {
                        if (!cgi->printf(cgi, "Content-type: text/xml;charset=UTF-8\r\n")) {
				goto CGI_ERR;
                        }
		} else { /* html */
                        if (!cgi->printf(cgi, "Content-type: text/html\r\n")) {
				goto CGI_ERR;
		}
                }
                if (!cgi->printf(cgi, "Connection: close\r\n")) {
			goto CGI_ERR;
                }
		cgi->flag |= CGIH_INIT;

		/* Output the session   */
		if (cgi->flag & SESS_INIT) {
			cgi->sess_output(cgi); /* Output session */
		}
		if (!cgi->printf(cgi, "\r\n"))
			goto CGI_ERR;
		if (!cgi->flush(cgi))
			goto CGI_ERR;
	}
        return 1;

CGI_ERR:
        return 0;
}

/* Purpose: Output redirect header to web client
 * In     : *cgi: the cgi object
 *          *url: the URI address
 * Return : FAILURE: 0
 *          SUCCESS: 1
 */
static int cgi_redirect(cgi_t *cgi, const char *url)
{
	char *brows;
	cgi_env_t *env;

        if ((cgi == NULL) || (url == NULL)) {
                return 0;
        }
	env = cgi->cgienv;

	if (cgi->flag & CGIH_INIT) {
                return 0;
	}
	
        if(!cgi->printf(cgi, "HTTP/1.1 301 Moved Permanently\r\n")) {
		goto CGI_ERR;
        }
        if((brows = env->get_val(env, CGI_REMOTE_IDENT)) == NULL) {
		goto CGI_ERR;
        }
	if(strstr(brows, "IE")){
                if (!cgi->printf(cgi, "Connection: close\r\n")) {
			goto CGI_ERR;
                }
                if (!cgi->printf(cgi, "Location: %s\r\n\r\n", url)) {
			goto CGI_ERR;
                }
	}else if(strstr(brows, "Firefox")){
                if (!cgi->printf(cgi, "Location: %s\r\n\r\n", url)) {
			goto CGI_ERR;
                }
                if (!cgi->printf(cgi, "Connection: close\r\n")) {
			goto CGI_ERR;
                }
	}else{
                if (!cgi->printf(cgi, "Connection: close\r\n")) {
			goto CGI_ERR;
                }
                if (!cgi->printf(cgi, "Location: %s\r\n\r\n", url)) {
			goto CGI_ERR;
	}
        }

        if (!cgi->flush(cgi)) {
		goto CGI_ERR;
        }

        return 1;
	
CGI_ERR:
        return 0;
}

/* Purpose: Find a session in the session list
 *             Exist: ok
 *             haven't session: create a session, ok
 *             Don't find: error
 * In     : *cgi: the cgi object
 * Return : 0: error
 *          1: ok
 */
static int cgi_sess_start(cgi_t *cgi)
{
	cgi_sess_tab_t *stab;
	cgi_sess_t *sess;
	char *name, *val = NULL;

	stab = cgi->stab;
	/* Isn't session ID       */
	if (cgi->sessbuf[0] == '\0') {
		goto NEW_SESS;
	}

	/* Find the session       */
	name = cgi->sessbuf;
	if ((name = strstr(name, CGI_SESSID_NAME)) == NULL) {
		goto NEW_SESS;
	}
	
	/* Set value starting address  */
	val = name + strlen(CGI_SESSID_NAME) + 1;
	/* Set string terminated '\0'  */
	for (; (*name != ';') && (*name != '\0'); name++);
	if (*name == ';')
		*name++ = '\0';

	/* Check session ID length */
	if (strlen(val) != SESS_ID_LEN) {
		val = NULL;
		goto NEW_SESS;
	}

	sess = cgi->stab->find(cgi->stab, val);
        if (sess) {
		goto NEW_FINISH;
	}

NEW_SESS:
	if ((sess = stab->new(stab, val)) == NULL) {
		return 0;
	}

NEW_FINISH:
	sess->expire = stab->expire;
	cgi->sess = sess; /* Got a new session       */
	cgi->flag |= SESS_INIT; /* Start the session */

	return 1;
}

/* Purpose: Destroy the session from session list
 * In     : *cgi: the cgi object
 * Return : 0: error
 *          1: ok
 */
static int cgi_sess_destroy(cgi_t *cgi)
{
	if (cgi->sess != NULL) {
		cgi->stab->cfree(cgi->stab, cgi->sess);
   //             SYS_LOG(io_log, "destroy a session: ID = %s\n",
                    //    cgi->sess->sessid);
	}
	cgi->flag &= SESS_NOT_INIT;
	
	return 1;
}

/* Purpose: Output session cookie header
 * In     : *cgi: the cgi object
 * Return : 0: error
 *          1: ok
 */
static int cgi_sess_output(cgi_t *cgi)
{
	/* Output session cookie header */
	if (!cgi->printf(cgi, "Set-cookie: %s=%s;\r\n",
                         CGI_SESSID_NAME, cgi->sess->sessid)) {
		return 0;
        }
	
	return 1;
}

/* Purpose: printf information into socket fd
 * Note   : If the outputing data is more than 4k,
 *          the buffer will be overflow, and return -1;
 *
 * In     : *cgi: the cgi object
 *          *fmt: the outputing format string
 * Return : -1: error
 *          the length to output
 */
static int cgi_printf(cgi_t *cgi, const char *fmt, ...)
{
	va_list ap;
	int ret;

        va_start(ap, fmt);
        ret = cgi_vprintf(cgi, fmt, ap);
        va_end(ap);

	return ret;
}

/* Purpose: printf information into socket fd
 * Note   : If the outputing data is more than 4k,
 *          the buffer will be overflow, and return -1;
 *
 * In     : *cgi: the cgi object
 *          *fmt: the outputing format string
 *          ap: the arguments list
 *
 * Return : -1: error
 *          the length to output
 */
static int cgi_vprintf(cgi_t *cgi, const char *fmt, va_list ap)
{
	int len, slen;
#define CGI_PRINT_BUF_LEN       (4100)
	char tmpbuf[CGI_PRINT_BUF_LEN], *tstr; /* tmp buffer */

        if ((cgi == NULL) || (fmt == NULL)) {
		return -1;
        }
        len = vsnprintf(tmpbuf, CGI_PRINT_BUF_LEN-1, fmt, ap);
	if (len >= CGI_PRINT_BUF_LEN) {
		return -1;
	} else
		tmpbuf[len] = '\0';

	tstr = tmpbuf;
	slen = len;
	while (slen > 0) {
		int rlen;

		rlen = CGI_SNDBUF_LEN - cgi->sendlen - 1;
		rlen = (slen < rlen) ? slen : rlen;
		memcpy(&cgi->sendbuf[cgi->sendlen], tstr, rlen);
		cgi->sendlen += rlen;
		if (cgi->sendlen >= (CGI_SNDBUF_LEN - 2)) {
			if (!cgi->flush(cgi)) {
				return -1;
			}
		}
		tstr += rlen;
		slen -= rlen;
	}
	
	return len;
}

/* Purpose: Direct output data into socket buffer to avoid
 *          redundance data copy
 * In     : *cgi: the cgi object
 *          *buf: the buffer
 *          len: the buffer length
 * Return : -1: error
 *          the length to output
 */
static int cgi_output(cgi_t *cgi, const char *buf, int len)
{
	int slen;
	const char *tstr;

        if ((buf == NULL) || (len <= 0)) {
		return -1;
        }
	tstr = buf;
	slen = len;
	while (slen > 0) {
		int rlen;

		rlen = CGI_SNDBUF_LEN - cgi->sendlen - 1;
		rlen = (slen < rlen) ? slen : rlen;
		memcpy(&cgi->sendbuf[cgi->sendlen], tstr, rlen);
		cgi->sendlen += rlen;
		if (cgi->sendlen >= (CGI_SNDBUF_LEN - 2)) {
			if (!cgi->flush(cgi)) {
				return -1;
			}
		}
		tstr += rlen;
		slen -= rlen;
	}

	return len;
}

/* Purpose: flush standard I/O buffer into socket
 * In     : *cgi: the cgi object
 * Return : 0: error
 *          1: ok
 */
static int cgi_flush(cgi_t *cgi)
{
	if (socket_write(cgi->sockfd, cgi->sendbuf,
			 cgi->sendlen, 1) == -1) {
		cgi->sendlen = 0; /* Lost buffer data */
		return 0;
	}

	cgi->sendlen = 0;
	return 1;
}

/* Purpose: Clear cgi record
 * In     : *cgi: cgi record object
 * Return : void 
 */
static void cgi_clear(cgi_t *rec)
{
	rec->flag = CGI_FLAG_CLR;
	if (rec->sockfd != -1) {
		close(rec->sockfd);
		rec->sockfd = -1;
	}
	rec->envbuf[0] = '\0';
	rec->envlen = 0;
	rec->recbuf[0] = '\0';
	rec->reclen = 0;
	rec->sendbuf[0] = '\0';
	rec->sendlen = 0;
	rec->sessbuf[0] = '\0';

	rec->stab = NULL;
}

/* Purpose: Create cgi record table(list) object
 * In     : **ctab: the pointer to store the CGI table object
 *          max: the Max. cgi record pool
 * Return : NULL: error
 *          the cgi table pointer
 */
cgi_tab_t *cgi_tab_new(cgi_tab_t **ctab, int max)
{
	cgi_tab_t *tmp;
	cgi_t *cgi;
	int i;

        if ((ctab == NULL) || (max < 0)) {
		return NULL;
        }
	
/* Allocate the memory space */
	tmp = (cgi_tab_t *)calloc(1, sizeof(cgi_tab_t));
	if (tmp == NULL) {
		return NULL;
	} else {
		*ctab = tmp;
	}
	INIT_LIST_HEAD(&tmp->head);
	pthread_mutex_init(&tmp->lock, NULL);
	
/* Initialize data          */
	/* CGI list                */
	for (i = 0; i < max; i++) {
		if ((cgi = cgi_new(&cgi)) == NULL)
			goto CGITAB_ERR;
		list_add(&cgi->node, &tmp->head);
	}

	/* session table object    */
	if ((tmp->stab = cgisess_tab_new(&tmp->stab)) == NULL)
		goto CGITAB_ERR;

        /* Initialize ways         */
	tmp->delete = cgi_tab_delete;

	tmp->alloc = cgi_tab_alloc;
	tmp->cfree = cgi_tab_free;

	return tmp;

CGITAB_ERR:
	if (tmp != NULL) {
		cgi_tab_delete(tmp);
		free(tmp);
	}
	return NULL;
}

/* Purpose: Delete cgi record table object
 * In     : *ctab: the cgi table pointer
 * Return : void
 */
static void cgi_tab_delete(cgi_tab_t *ctab)
{
	cgi_t *rec, *tmp;

	/* Free CGI list            */
	pthread_mutex_lock(&ctab->lock);
	if (!list_empty(&ctab->head)) {
		list_for_each_entry_safe(rec, tmp, &ctab->head, node) {
			list_del(&rec->node);
			rec->delete(rec);
		}
	}
	pthread_mutex_unlock(&ctab->lock);
	/* Free session table list  */
	if (ctab->stab != NULL)
		ctab->stab->delete(ctab->stab);

	free(ctab);
}

/* Purpose: Allocate a cgi record from CGI table
 * In     : *ctab: the CGI table object
 * Return : NULL: error
 *          other: the valid cgi object pointer
 */
static cgi_t *cgi_tab_alloc(cgi_tab_t *ctab)
{
	cgi_t *rec = NULL;

	/* Finding a unused cgi record      */
	pthread_mutex_lock(&ctab->lock);

	if (!list_empty(&ctab->head)) {
		list_for_each_entry(rec, &ctab->head, node) {
			if (!(rec->flag & CGI_USED)) {
				cgi_clear(rec);
				rec->flag = CGI_USED;
				break;
			}
		}
	}

	pthread_mutex_unlock(&ctab->lock);
	if (rec && (rec->flag & CGI_USED))
		return rec;

	printf("found no cgi\n");
	return NULL;
}

/* Purpose: Free a cgi record into CGI table
 * In     : *ctab: the CGI table object
 *          *cgi: the CGI object to free
 * Return :  void
 */
static void cgi_tab_free(cgi_tab_t *ctab, cgi_t *cgi)
{
	cgi_t *rec = NULL;

	if (cgi == NULL)
		return;
	pthread_mutex_lock(&ctab->lock);

	list_for_each_entry(rec, &ctab->head, node) {
		if (rec == cgi) {
			cgi_clear(rec);
			rec->flag = CGI_UNUSED;
			break;
		}
	}

	pthread_mutex_unlock(&ctab->lock);
}

/* Purpose: Add a cgi variable record including name and value
 *          into the head list.
 * In     : *head: the cgi list header's address
 *          *n: the cgi variable name
 *          *v: the cgi variable value
 * Return : 0: error
 *          1: ok
 */
static int cgi_add_val(struct list_head *head, const char *n, const char *v)
{
	cgi_val_t *rec;

        if ((head == NULL) || (n == NULL) || (v == NULL)) {
                return 0;
        }

        if ((rec = calloc(1, sizeof(cgi_val_t))) == NULL) {
                return 0;
        }

	INIT_LIST_HEAD(&rec->node);
	rec->name = (char *)n;
	rec->val = (char *)v;
	
	list_add_tail(&rec->node, head);
	return SUCCESS;
}

/* Purpose: Get a value of cgi variable
 * In     : *head: the cgi list header
 *          *n: the cgi variable name
 * Return : NULL: error
 *          the value string address of cgi variable
 */
static char *cgi_get_val(struct list_head *head, const char *n)
{
	cgi_val_t *rec;
	struct list_head *pos;

        if ((head == NULL) || (n == NULL)) {
		return NULL;
        }

	list_for_each(pos, head) {
		rec = list_entry(pos, cgi_val_t, node);
		if (strcmp(rec->name, n) == 0)
			return rec->val;
	}

	return NULL;
}

/* Purpose: Destroy the cgi list record, but reserve the object
 * In     : *head: the cgi list header's address
 * Return : void
 */
static void cgi_destroy(struct list_head *head)
{
	cgi_val_t *rec, *tmp;

        if (head == NULL) {
		return;
        }
	
	/* Free the head list  */
	if (!list_empty(head)) {
		list_for_each_entry_safe(rec, tmp, head, node) {
			list_del(&rec->node);
			free(rec);
		}
	}
	INIT_LIST_HEAD(head);
}

/* Purpose: cgi handler
 * In     : *p: cgi object pointer
 *          *h: callback function for cgi handler
 *          *d: callback function parameters for cgi handler
 * Return : 0: error
 *          1: ok
 */
void prgcgi_main_handler(void *data)
{
	cgi_env_t *env;
	char *pname;
	cgi_t *cgi = (cgi_t *)data;
	
	cgi->cgi_parse_env(cgi);
	env = cgi->cgienv;
	if ((pname = env->get_val(env, CGI_SCRIPT_NAME)) == NULL) {
		return;
	}

	if(strcmp(pname, "download.csp") != 0){
		return;
	}

/* Initialize CGI             */
	cgi->cgi_init(cgi);   /* initialize     */
	cgi->sess_start(cgi); /* start session  */
#if 0
	cgi->cgi_form(cgi);   /* get form info. */
#endif
/* Handler CGI                */
	default_cgi_handler(cgi);
/* Finish  CGI                */
	cgi->cgi_end(cgi);

	return ;
}


