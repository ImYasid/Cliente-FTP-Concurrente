/* connectTCP.c - connectTCP */

int connectsock(const char *host, const char *service, const char *transport);

/*------------------------------------------------------------------------
 * connectTCP - connect to a specified TCP service on a specified host
 *------------------------------------------------------------------------
 */
int connectTCP(const char *host, const char *service)
{
    /* -------------------------------------------------------------------
     * Solicitar conexión socket especificando protocolo "tcp"
     * ------------------------------------------------------------------- */
    return connectsock(host, service, "tcp");
}