/* passiveTCP.c - passiveTCP */

int passivesock(const char *service, const char *transport, int qlen);

/*------------------------------------------------------------------------
 * passiveTCP - create a passive socket for use in a TCP server
 *------------------------------------------------------------------------
 */
int passiveTCP(const char *service, int qlen)
{
    return passivesock(service, "tcp", qlen);
}