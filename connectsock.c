/* connectsock.c - connectsock */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#ifndef INADDR_NONE
#define INADDR_NONE 0xffffffff
#endif

extern int errno;
int errexit(const char *format, ...);

/*------------------------------------------------------------------------
 * connectsock - allocate & connect a socket using TCP or UDP
 *------------------------------------------------------------------------
 */
int connectsock(const char *host, const char *service, const char *transport)
{
    struct hostent  *phe;   /* pointer to host information entry    */
    struct servent  *pse;   /* pointer to service information entry */
    struct protoent *ppe;   /* pointer to protocol information entry*/
    struct sockaddr_in sin; /* an Internet endpoint address         */
    int s, type;            /* socket descriptor and socket type    */

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;

    /* -------------------------------------------------------------------
     * 1. Mapear nombre de servicio a número de puerto
     * ------------------------------------------------------------------- */
    if ( (pse = getservbyname(service, transport)) )
        sin.sin_port = pse->s_port;
    else if ( (sin.sin_port = htons((unsigned short)atoi(service))) == 0 )
        errexit("can't get \"%s\" service entry\n", service);

    /* -------------------------------------------------------------------
     * 2. Mapear nombre de host a dirección IP
     * ------------------------------------------------------------------- */
    if ( (phe = gethostbyname(host)) )
        memcpy(&sin.sin_addr, phe->h_addr, phe->h_length);
    else if ( (sin.sin_addr.s_addr = inet_addr(host)) == INADDR_NONE )
        errexit("can't get \"%s\" host entry\n", host);

    /* -------------------------------------------------------------------
     * 3. Mapear nombre de protocolo a número de protocolo
     * ------------------------------------------------------------------- */
    if ( (ppe = getprotobyname(transport)) == 0 )
        errexit("can't get \"%s\" protocol entry\n", transport);

    /* Usar el protocolo para elegir el tipo de socket */
    if (strcmp(transport, "udp") == 0)
        type = SOCK_DGRAM;
    else
        type = SOCK_STREAM;

    /* -------------------------------------------------------------------
     * 4. Asignar un socket
     * ------------------------------------------------------------------- */
    s = socket(PF_INET, type, ppe->p_proto);
    if (s < 0)
        errexit("can't create socket: %s\n", strerror(errno));

    /* -------------------------------------------------------------------
     * 5. Conectar el socket
     * ------------------------------------------------------------------- */
    if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
        errexit("can't connect to %s.%s: %s\n", host, service, strerror(errno));

    return s;
}