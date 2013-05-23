#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
#include <iconv.h>
#include <assert.h>
#include <sys/stat.h> 
#include <fcntl.h>
#include <zlib.h>
#include <string.h>
#include <stdio.h>
#include "common.h"


int inf(const char *src, int srcLen, const char *dst, int dstLen)
{
        MCEMT_DBG("inf\n");
        z_stream strm;
        strm.zalloc=NULL;
        strm.zfree=NULL;
        strm.opaque=NULL;

        strm.avail_in = srcLen;
        strm.avail_out = dstLen;
        strm.next_in = (Bytef *)src;
        strm.next_out = (Bytef *)dst;
        MCEMT_DBG("inf\n");
        int err=-1, ret=-1;
        err = inflateInit2(&strm, MAX_WBITS+16);
        if (err == Z_OK){
                err = inflate(&strm, Z_FINISH);
                if (err == Z_STREAM_END){
                        ret = strm.total_out;
                }
                else{
                        inflateEnd(&strm);
                        return err;
                }
        }else{
                inflateEnd(&strm);
                return err;
        }
        MCEMT_DBG("inf\n");
        inflateEnd(&strm);
       // printf("%s\n", dst);
        return err;
}



int receive_data(int s, char *buf, int length, int timeout)
{
        MCEMT_DBG("receive_data starts\n");
        int n;
        struct pollfd fds[1]; /* for the poll */
        fds[0].fd = s;
        fds[0].events = POLLIN;
        n = poll(fds, 1, timeout);

        if (n < 0) {
                fprintf(stderr, "poll error");
                return 0;
        } else if (n == 0) {
                return 0;
        }
        n = recv(s, buf, length, 0);
        if (n < 0) {
                fprintf(stderr, "recv");
                return 0;
        }
        MCEMT_DBG("receive_data finishes\n");
        return n;
}



int tr_opensock(const char *host)
{
        int s;
        struct hostent *hp;
        struct sockaddr_in dest;
        struct hostent hostinfo;
        char   dns_buff[8192];
        int rc;
 
        gethostbyname_r(host, &hostinfo, dns_buff, 8192, &hp, &rc);
        if (hp == NULL) {
                herror(host);
                return 0;
        }
        if (hp->h_addr == NULL)
                return 0;
        memcpy(&dest.sin_addr, hp->h_addr, sizeof(dest.sin_addr));
        memset(dest.sin_zero, 0, sizeof(dest.sin_zero));
        s = socket(PF_INET, SOCK_STREAM, 0);
        if(s < 0) {
                perror("socket");
                return 0;
        }

        dest.sin_family = AF_INET;
        dest.sin_port = htons(9091);
        
	if(connect(s, (struct sockaddr *)&dest, sizeof(struct sockaddr_in)) < 0){
                perror("connect");
                close(s);
                return 0;
        }
 
        return s;
}



int xl_opensock(const char *host)
{
        int s;
        struct hostent *hp;
        struct sockaddr_in dest;
        struct hostent hostinfo;
        char   dns_buff[8192];
        int rc;
 
        gethostbyname_r(host, &hostinfo, dns_buff, 8192, &hp, &rc);
        if (hp == NULL) {
                herror(host);
                return 0;
        }
        if (hp->h_addr == NULL)
                return 0;
        memcpy(&dest.sin_addr, hp->h_addr, sizeof(dest.sin_addr));
        memset(dest.sin_zero, 0, sizeof(dest.sin_zero));
        s = socket(PF_INET, SOCK_STREAM, 0);
        if(s < 0) {
                perror("socket");
                return 0;
        }

        dest.sin_family = AF_INET;
        dest.sin_port = htons(9005);
        
	if(connect(s, (struct sockaddr *)&dest, sizeof(struct sockaddr_in)) < 0){
                perror("connect");
                close(s);
                return 0;
        }
 
        return s;
}

