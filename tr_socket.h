#ifndef TR_SOCKET_H
#define TR_SOCKET_H

extern int receive_data(int s, char *buf, int length, int timeout);
extern int tr_opensock(const char *host);
extern int inf(const char *src, int srcLen, const char *dst, int dstLen);
extern int xl_opensock(const char *host);

#endif 

