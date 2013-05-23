/******************************************************************************
 * Copyright (C) 2007 by IOVST
 * 
 * File: ht_file.c
 *
 * Date: 2007-06-08
 *
 * Author: Liu Yong, <liuyong@iovst.com>
 *
 * Descriptor:
 *   
 * Note:
 *  The file handle program
 *
 * Version: 0.1
 *
 * Modified:
 *
 ******************************************************************************/
/*-----------------------------------------------------------------------------
  System header files 
------------------------------------------------------------------------------*/
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>

/*-----------------------------------------------------------------------------
  Local header files
------------------------------------------------------------------------------*/
#include "ht_def.h"
#include "httpd.h"
#include "common.h"

/*-----------------------------------------------------------------------------
  Data structures
------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
  Global variables and constants
------------------------------------------------------------------------------*/
static ht_mrec_t def_mime_types[] = {
	{HTTP_MIME_HTML,sizeof(HTTP_MIME_HTML)-1,"text/html" },
	{HTTP_MIME_HTM,	sizeof(HTTP_MIME_HTM)-1, "text/html" },
	{HTTP_MIME_TXT,	sizeof(HTTP_MIME_TXT)-1, "text/plain"},
	{HTTP_MIME_CSS, sizeof(HTTP_MIME_CSS)-1, "text/css"  },
	{HTTP_MIME_ICO,	sizeof(HTTP_MIME_ICO)-1, "image/x-icon"},
	{HTTP_MIME_GIF,	sizeof(HTTP_MIME_GIF)-1, "image/gif" },
	{HTTP_MIME_JPG,	sizeof(HTTP_MIME_JPG)-1, "image/jpeg"},
	{HTTP_MIME_JPEG,sizeof(HTTP_MIME_JPEG)-1,"image/jpeg"},
	{HTTP_MIME_PNG,	sizeof(HTTP_MIME_PNG)-1, "image/png" },
	{HTTP_MIME_SVG,	sizeof(HTTP_MIME_SVG)-1, "image/svg+xml"},
	{HTTP_MIME_TORRENT,sizeof(HTTP_MIME_TORRENT)-1,"application/x-bittorrent"},
	{HTTP_MIME_WAV, sizeof(HTTP_MIME_WAV)-1, "audio/x-wav"},
	{HTTP_MIME_MP3,	sizeof(HTTP_MIME_MP3)-1, "audio/x-mp3"},
	{HTTP_MIME_MID,	sizeof(HTTP_MIME_MID)-1, "audio/mid"  },
	{HTTP_MIME_M3U,	sizeof(HTTP_MIME_M3U)-1, "audio/x-mpegurl"},
	{HTTP_MIME_RAM,	sizeof(HTTP_MIME_RAM)-1, "audio/x-pn-realaudio"},
	{HTTP_MIME_RA,	sizeof(HTTP_MIME_RA)-1,  "audio/x-pn-realaudio"},
	{HTTP_MIME_DOC,	sizeof(HTTP_MIME_DOC)-1, "application/msword"},
	{HTTP_MIME_EXE,	sizeof(HTTP_MIME_EXE)-1, "application/octet-stream"},
	{HTTP_MIME_ZIP,	sizeof(HTTP_MIME_ZIP)-1, "application/x-zip-compressed"},
	{HTTP_MIME_XLS,	sizeof(HTTP_MIME_XLS)-1, "application/excel"},
	{HTTP_MIME_TGZ,	sizeof(HTTP_MIME_TGZ)-1, "application/x-tar-gz"},
	{HTTP_MIME_TARGZ, sizeof(HTTP_MIME_TARGZ)-1, "application/x-tar-gz"},
	{HTTP_MIME_TAR,	sizeof(HTTP_MIME_TAR)-1, "application/x-tar"},
	{HTTP_MIME_GZ,	sizeof(HTTP_MIME_GZ)-1,  "application/x-gunzip"},
	{HTTP_MIME_ARJ,	sizeof(HTTP_MIME_ARJ)-1, "application/x-arj-compressed"},
	{HTTP_MIME_RAR,	sizeof(HTTP_MIME_RAR)-1, "application/x-arj-compressed"},
	{HTTP_MIME_RTF,	sizeof(HTTP_MIME_RTF)-1, "application/rtf"},
	{HTTP_MIME_PDF,	sizeof(HTTP_MIME_PDF)-1, "application/pdf"},
	{HTTP_MIME_MPG,	sizeof(HTTP_MIME_MPG)-1, "video/mpeg"     },
	{HTTP_MIME_MPEG,sizeof(HTTP_MIME_MPEG)-1,"video/mpeg"     },
	{HTTP_MIME_ASF,	sizeof(HTTP_MIME_ASF)-1, "video/x-ms-asf"	},
	{HTTP_MIME_AVI,	sizeof(HTTP_MIME_AVI)-1, "video/x-msvideo"},
	{HTTP_MIME_BMP,	sizeof(HTTP_MIME_BMP)-1, "image/bmp"      },
	{NULL,		0,	NULL				}
};

/*-----------------------------------------------------------------------------
  Functions
------------------------------------------------------------------------------*/
static void ht_mime_delete(ht_mime_t *mime);
static const char *ht_mime_find(ht_mime_t *mime, const char *url);

/*-----------------------------------------------------------------------------
  Functions
------------------------------------------------------------------------------*/
/* Purpose: Create mime type object
 * In     : **mime: the pointer to store mime object
 * Return : the mime object address
 */
ht_mime_t *ht_mime_new(ht_mime_t **mime)
{
	ht_mime_t *tmp;

        if (mime == NULL) {
                return NULL;
        }

	tmp = (ht_mime_t *)calloc(1, sizeof(ht_mime_t));
        if (tmp == NULL) {
		return NULL;
        } else
		*mime = tmp;

	/* Initialize data  */
	tmp->type = def_mime_types;
	
	/* Initialize ways  */
	tmp->delete = ht_mime_delete;
	tmp->find = ht_mime_find;

	return tmp;
}

/* Purpose: Delete the mime type object
 * In     : *mime: the mime type object
 * Return : void
 */
static void ht_mime_delete(ht_mime_t *mime)
{
	if (mime != NULL)
		free(mime);
}

/* Purpose: find the mime type according to uri
 * In     : *mime: the mime type object
 *          *url: the URL address string
 *          len: the url length
 * Return : the mime type string for url
 */
static const char *ht_mime_find(ht_mime_t *mime, const char *url)
{
	ht_mrec_t *tmp;
	const char *s;

        if ((mime == NULL) || (url == NULL)) {
                return NULL;
        }

	/* Finding  */
	tmp = mime->type;
	s = url + strlen(url);
	while(tmp->ext != NULL) {
		const char *tp = s - tmp->extlen;

		if (*(tp-1) == '.') {
			if (strncasecmp(tmp->ext,tp,tmp->extlen) == 0) {
				return tmp->mime; /* Found */
			}
		}
		tmp++;
	}

	/* No luck  */
	return("text/plain");
}


/*-----------------------------------------------------------------------------
  Functions
------------------------------------------------------------------------------*/
/* Purpose: read file
 * In     : *data: thread data pointer
 *          head: only return file head message
 * Return : void
 */
void ht_read_file(thread_data_t *data, int head)
{
	int fd = -1;
	ht_errtable_t *errtab;
	struct stat st;
	int status;
	const char *msg;
	char *headval;
	char range[64]="";
	unsigned long long int cl; /* send length, support > 8G(2^32) */
	time_t curtime;
	struct tm curtm;
	long long int len;
	int i=0;

	errtab = data->httpd->errtab;
/*
 * Open the file
 */
 	for(i=0; data->uri[i] != '\0'; i++){
		if(data->uri[i] == '.'){
			if(data->uri[i+1] == '.'){
				errtab->send_error(errtab, HTTP_INTER_SERVER_ERR, data);
				return;
			}
		}
	}
	
	snprintf(data->sbuf, sizeof(data->sbuf), "%s%s",
		 data->httpd->wkhome, data->uri);
	if ((fd = open(data->sbuf, O_RDONLY, 0644)) == -1) {
		errtab->send_error(errtab, HTTP_INTER_SERVER_ERR, data);
		return;
	}

/*
 * Fill headers and send
 */
	/* Get the local file status   */
	if (fstat(fd, &st) == -1) {
		errtab->send_error(errtab, HTTP_INTER_SERVER_ERR, data);
		goto READ_ERR;
	}
	cl = st.st_size; /* file size */

	/* Get the content range */
	status = HTTP_OK;
	msg = errtab->get_name(errtab, HTTP_OK);
	
	headval = data->rhead->find(data->rhead, HTTP_CONT_RANGE_EHDR);
	if (headval != NULL) {
		int n;
		unsigned long long r1, r2;
		if ((n = sscanf(headval, "byte=%llu-%llu", &r1, &r2)) > 0) {
			status = HTTP_PARTIAL_CONTENT;

			/* move file position pointer */
			(void)lseek(fd, r1, SEEK_SET); 
			cl = (n==2) ? (r2-r1+1) : (cl - r1);
			snprintf(range, sizeof(range),
				 "Content-Range: bytes %llu-%llu/%llu\r\n",
				 r1, r1+cl-1, (unsigned long long)st.st_size);

			msg = errtab->get_name(errtab,
					       HTTP_PARTIAL_CONTENT);
		}
	}

	len = 0;
        /* Fill http response line   */
	len += sprintf(data->sbuf+len, "HTTP/1.1 %d %s\r\n", status, msg);

	/* Fill date header          */
	time(&curtime);
	localtime_r(&curtime, &curtm);
	len += sprintf(data->sbuf+len, "Date: ");
	len += (int)strftime(data->sbuf+len, (size_t)(THREAD_SNDBUF_LEN-len),
			     HT_RFC1123FMT, &curtm);
	len += sprintf(data->sbuf+len, "\r\n");

	/* Fill last modified header */
	localtime_r(&st.st_mtime, &curtm);
	len += sprintf(data->sbuf+len, "Last-Modified: ");
	len += (int)strftime(data->sbuf+len, (size_t)(THREAD_SNDBUF_LEN-len),
			     HT_RFC1123FMT, &curtm);
	len += sprintf(data->sbuf+len, "\r\n");

	/* Fill etag header          */
	len += sprintf(data->sbuf+len, "Etag: \"%lx.%lx\"\r\n", 
		       (unsigned long)st.st_mtime, 
		       (unsigned long)st.st_size);
	
	/* Fill Content-Type header  */
	{
		ht_mime_t *mime = data->httpd->mime;
		data->mime_type = mime->find(mime, data->uri);
		len += sprintf(data->sbuf+len, "Content-Type: %s\r\n", 
			       data->mime_type);
	}
	
	/* Fill Content-Length header*/
	len += sprintf(data->sbuf+len, "Content-Length: %llu\r\n", cl);
	
	/* Fill Connection header    */
	len += sprintf(data->sbuf+len, "Connection: close\r\n");
	
	/* Fill Content-Range header */
	if (headval != NULL) {
		len += sprintf(data->sbuf+len, "%s\r\n", range);
	} else {
		len += sprintf(data->sbuf+len, "\r\n");
	}
	data->sendlen = len;

	/* If only send header, return  */
	if (head) {
		socket_write(data->sock, data->sbuf, data->sendlen, 1);
		goto READ_ERR;
	}

/*
 * read data form local file and send to socket
 */
	len += cl; /* Add the header length */
	do {
		int retlen;
		/*
		 * Reading data
		 */
		/* count the length to read */
		retlen = (len > THREAD_SNDBUF_LEN) ? THREAD_SNDBUF_LEN : len;

		/* Read data                */
		retlen = read_n(fd, &data->sbuf[data->sendlen], 
				retlen - data->sendlen);
		if (retlen == -1) {
			goto READ_ERR;
		}
		data->sendlen += retlen;
		
		/*
		 * Write data
		 */
		retlen = socket_write(data->sock, 
				      data->sbuf, data->sendlen, 1);
		if ((retlen == -1) || (data->sendlen != retlen)) {
			goto READ_ERR;
		}

		/* After the data read from file and write to socket,
		 * substrat the transfer length.
		 */
		len -= retlen;
		data->sendlen = 0;

	} while(len > 0);
	
READ_ERR:
	close(fd);
	data->keepalive = THREAD_CON_CLOSE;
}

