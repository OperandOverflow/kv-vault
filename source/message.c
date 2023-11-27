/**
 * SD-07
 * 
 * Xiting Wang      <58183>
 * Goncalo Pinto    <58178>
 * Guilherme Wind   <58640>
*/

#include "message-private.h"

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

int write_all(int sock, void *buf, int len) {
    int bufsize = len;
    while(len>0) {
        int res = write(sock, buf, len); /* tmb funcionaria send */
        if(res<0) {
        if(errno == EINTR) continue;
            perror("write failed:");
            return res; /* Error != EINTR */
        }
        if(res == 0) 
            return res; /* Socket was closed */
        buf += res;
        len -= res;
    }
    return bufsize;
}

int read_all(int sock, void *buf, int len) {
    int bufsize = len;
    while(len>0) {
        int res = read(sock, buf, len);
        if(res<0) {
        if(errno == EINTR) continue;
            perror("read failed:");
            return res; /* Error != EINTR */
        }
        if(res == 0) 
            return res; /* Socket was closed */
        buf += res;
        len -= res;
    }
    return bufsize;
}