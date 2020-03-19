#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "vmdsock.h"
#include "clock.h" // for ClockTics

#define HANDSHAKE 4
#define VERSION 2
#define HEADERSIZE 8
const int port = 10086;

typedef int     int32;

typedef struct { int32 type; int32 length; } Header;
typedef long long TimeTics;

int32 writen(void *s, const char *ptr, int32 n)
{
    int32 nleft;
    int32 nwritten;

    nleft = n;
    while (nleft > 0)
    {
        if ((nwritten = vmdsock_write(s, ptr, nleft)) <= 0)
        {
            if (errno == EINTR)
                nwritten = 0;
            else
                return -1;
        }
        nleft -= nwritten;
        ptr += nwritten;
    }
    return n;
}

int32 htonl(int32 h)
{
    int32 n = 0;
    ((char *)&n)[0] = (h >> 24) & 0x0FF;
    ((char *)&n)[1] = (h >> 16) & 0x0FF;
    ((char *)&n)[2] = (h >> 8) & 0x0FF;
    ((char *)&n)[3] = h & 0x0FF;
    return n;
}

void fill_header(Header *header, int32 type, int32 length)
{
    header->type = htonl(type);
    header->length = htonl(length);
}

int handshake(void *s)
{
    Header header;
    fill_header(&header, HANDSHAKE, 1);
    header.length = VERSION;   /* Not byteswapped! */
    return (writen(s, (char *)&header, HEADERSIZE) != HEADERSIZE);
}

template<class T>
int send(void *s, int32 n, const T *data)
{
    int rc;
    int32 size = n * sizeof(T);
    char *buf = (char *) malloc(sizeof(char) * size); 
    memcpy(buf, data, size);
    rc = (writen(s, buf, size) != size);
    free(buf);
    return rc;
}

// Returns IMD client socket
void* await_connect()
{
    vmdsock_init();
    void* serversock = vmdsock_create();
    void* clientsock = NULL;
    vmdsock_bind(serversock, port);
    printf("Waiting for IMD connection on port %d...\n", port); fflush(stdout);
    vmdsock_listen(serversock);
    while( !clientsock )
    {
        if( vmdsock_selread(serversock, 0) > 0 )
        {
            clientsock = vmdsock_accept(serversock);
            printf("Before handshake\n"); fflush(stdout);
            if (handshake(clientsock) != 0)
            {
                printf("Handshake succeed!\n"); fflush(stdout);
                clientsock = NULL;
            }
        }
    }
    printf(" Connected.\n"); fflush(stdout);
    vmdsock_destroy(serversock);
    return clientsock;
}

int main()
{
    printf("Start: \n");
    void* sock = await_connect();
    int* buff = new int[10] {101,2,3,4,5,6,7,8,9,100};
    TimeTics Tstart = tics();
    TimeTics Tstep = 10;
    
    while( true )
    {
        TimeTics now = tics();
        double tabs = (double)(now - Tstart)/(double)Tstep;
        double t = (tabs > 1.0) ? 1.0 : tabs;

        send<int>(sock, 10, buff);

        while ( vmdsock_selread(sock,0) > 0 )
        {
            printf("Receive something!\n"); fflush(stdout);
            break;
        }
    }

    return 0;
}