/* errexit.c - errexit */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

/*------------------------------------------------------------------------
 * errexit - print an error message and exit
 *------------------------------------------------------------------------
 */
int errexit(const char *format, ...)
{
    va_list args;

    /* -------------------------------------------------------------------
     * 1. Inicializar argumentos y escribir mensaje en stderr
     * ------------------------------------------------------------------- */
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);

    /* -------------------------------------------------------------------
     * 2. Terminar la ejecución del programa (código de error 1)
     * ------------------------------------------------------------------- */
    exit(1);
}