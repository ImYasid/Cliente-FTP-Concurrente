/* connectTCP.c - connectTCP */

int connectsock(const char *host, const char *service, const char *transport);

/*------------------------------------------------------------------------
 * connectTCP - connect to a specified TCP service on a specified host
 *------------------------------------------------------------------------
 */
int connectTCP(const char *host, const char *service )
{
    return connectsock( host, service, "tcp");
}