/*
 * Cliente FTP Concurrente (Multisesión)
 * Autor: Yasid Jimenez
 *
 * Descripción:
 * Implementa un cliente FTP compatible con RFC 959.
 * Soporta concurrencia real mediante Multisesión: cada transferencia
 * crea un proceso hijo que abre una conexión de control totalmente nueva.
 */

#define _DEFAULT_SOURCE

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define LINELEN 128

/* --- PROTOTIPOS DE FUNCIONES EXTERNAS (Modulos) --- */
int errexit(const char *format, ...);
int connectTCP(const char *host, const char *service);
int passiveTCP(const char *service, int qlen);

/* --- VARIABLES GLOBALES PARA MULTISESIÓN --- */
/* Necesarias para clonar la sesión en los hijos */
char g_host[64];
char g_service[8];
char g_user[32];
char g_pass[32];

/*------------------------------------------------------------------------
 * sendCmd - Envía comandos FTP al servidor y lee la respuesta inicial
 *------------------------------------------------------------------------
 */
void sendCmd(int s, char *cmd, char *res)
{
    int n;
    n = strlen(cmd);
    cmd[n] = '\r';
    cmd[n + 1] = '\n';

    write(s, cmd, n + 2);
    n = read(s, res, LINELEN);

    if (n >= 0)
        res[n] = '\0';
}

/*------------------------------------------------------------------------
 * auto_login - Realiza el login automático (usado por procesos hijos)
 *------------------------------------------------------------------------
 */
int auto_login(int s)
{
    char cmd[128], res[128];
    int code = 0;

    read(s, res, LINELEN); /* Leer Banner de bienvenida */

    sprintf(cmd, "USER %s", g_user);
    sendCmd(s, cmd, res);

    sprintf(cmd, "PASS %s", g_pass);
    sendCmd(s, cmd, res);

    sscanf(res, "%d", &code);
    return (code == 230);
}

/*------------------------------------------------------------------------
 * pasivo - Negociación MODO PASIVO (Retorna socket de datos conectado)
 *------------------------------------------------------------------------
 */
int pasivo(int s_control)
{
    int sdata;
    char cmd[128], res[128], *p;
    char host[64], port[8];
    int h1, h2, h3, h4, p1, p2;

    sprintf(cmd, "PASV");
    sendCmd(s_control, cmd, res);

    p = strchr(res, '(');
    if (p == NULL)
        return -1;

    sscanf(p + 1, "%d,%d,%d,%d,%d,%d", &h1, &h2, &h3, &h4, &p1, &p2);
    snprintf(host, 64, "%d.%d.%d.%d", h1, h2, h3, h4);
    snprintf(port, 8, "%d", p1 * 256 + p2);

    sdata = connectTCP(host, port);
    return sdata;
}

/*------------------------------------------------------------------------
 * ayuda - Muestra el menú de ayuda
 *------------------------------------------------------------------------
 */
void ayuda()
{
    printf("\n--- Cliente FTP Multisesion ---\n"
           "    dir             - lista directorio (sincrono)\n"
           "    get <archivo>   - descarga en SEGUNDO PLANO\n"
           "    put <archivo>   - subida en SEGUNDO PLANO\n"
           "    cd <dir>        - cambiar directorio\n"
           "    pwd             - ver directorio actual (EXTRA)\n"
           "    mkdir <nombre>  - crear directorio (EXTRA)\n"
           "    delete <nombre> - borrar archivo (EXTRA)\n"
           "    quit            - salir\n\n");
}

/*------------------------------------------------------------------------
 * handle_sigchld - Limpieza de procesos Zombies
 *------------------------------------------------------------------------
 */
void handle_sigchld(int sig)
{
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

/*------------------------------------------------------------------------
 * main - Programa Principal
 *------------------------------------------------------------------------
 */
int main(int argc, char *argv[])
{
    char cmd[128], res[128];
    char data[LINELEN + 1];
    char prompt[64], *ucmd, *arg;
    int s_main, sdata, n;

    /* Configuración por defecto */
    strcpy(g_host, "localhost");
    strcpy(g_service, "21"); /* Puerto FTP default */

    if (argc > 1) strcpy(g_host, argv[1]);
    if (argc > 2) strcpy(g_service, argv[2]);

    /* -------------------------------------------------------------------
     * 1. Conexión Inicial (Padre)
     * ------------------------------------------------------------------- */
    s_main = connectTCP(g_host, g_service);
    n = read(s_main, res, LINELEN);
    if (n > 0) {
        res[n] = '\0';
        printf("%s", res);
    }

    /* -------------------------------------------------------------------
     * 2. Loop de Login Principal
     * ------------------------------------------------------------------- */
    while (1) {
        printf("Usuario: ");
        scanf("%s", g_user);
        sprintf(cmd, "USER %s", g_user);
        sendCmd(s_main, cmd, res);
        printf("%s", res);

        /* Limpiar buffer stdin */
        int c;
        while ((c = getchar()) != '\n' && c != EOF);

        char *p = getpass("Password: ");
        strcpy(g_pass, p);
        sprintf(cmd, "PASS %s", g_pass);
        sendCmd(s_main, cmd, res);
        printf("%s", res);

        int code = 0;
        if (sscanf(res, "%d", &code) == 1 && code == 230)
            break;
        printf("Login incorrecto.\n");
    }

    signal(SIGCHLD, handle_sigchld);
    ayuda();

    /* -------------------------------------------------------------------
     * 3. Loop de Comandos
     * ------------------------------------------------------------------- */
    while (1) {
        printf("ftp> ");
        if (fgets(prompt, sizeof(prompt), stdin) != NULL) {
            
            prompt[strcspn(prompt, "\n")] = 0;
            ucmd = strtok(prompt, " ");
            if (ucmd == NULL) continue;

            /* --- COMANDO: DIR (Síncrono) --- */
            if (strcmp(ucmd, "dir") == 0) {
                sdata = pasivo(s_main);
                if (sdata < 0) {
                    printf("Error en PASV\n");
                    continue;
                }

                sprintf(cmd, "LIST");
                sendCmd(s_main, cmd, res);

                while ((n = recv(sdata, data, LINELEN, 0)) > 0) {
                    fwrite(data, 1, n, stdout);
                }
                close(sdata);
                read(s_main, res, LINELEN);
                printf("\nTransmision finalizada.\n");

            /* --- COMANDO: GET (Asíncrono / Multisesión) --- */
            } else if (strcmp(ucmd, "get") == 0) {
                arg = strtok(NULL, " ");
                if (!arg) {
                    printf("Uso: get <archivo>\n");
                    continue;
                }

                pid_t pid = fork();

                if (pid == 0) {
                    /* ==============================
                     * PROCESO HIJO: NUEVA SESIÓN
                     * ============================== */
                    close(s_main); /* Cerrar socket heredado */

                    int s_child = connectTCP(g_host, g_service);
                    if (s_child < 0) exit(1);

                    if (!auto_login(s_child)) exit(1);

                    sdata = pasivo(s_child);
                    if (sdata < 0) exit(1);

                    sprintf(cmd, "RETR %s", arg);
                    sendCmd(s_child, cmd, res);

                    int code = 0;
                    sscanf(res, "%d", &code);
                    if (code >= 500) {
                        printf("Error: %s\n", res);
                        exit(1);
                    }

                    FILE *fp = fopen(arg, "wb");
                    if (!fp) exit(1);

                    printf("[Hijo %d] Descargando '%s'...\n", getpid(), arg);
                    while ((n = recv(sdata, data, LINELEN, 0)) > 0) {
                        fwrite(data, 1, n, fp);
                    }
                    fclose(fp);
                    close(sdata);
                    close(s_child);

                    printf("[Hijo %d] Fin '%s'.\n", getpid(), arg);
                    exit(0);
                } else {
                    printf("[Padre] Descarga '%s' en proceso %d\n", arg, pid);
                }

            /* --- COMANDO: PUT (Asíncrono / Multisesión) --- */
            } else if (strcmp(ucmd, "put") == 0) {
                arg = strtok(NULL, " ");
                if (!arg) {
                    printf("Uso: put <archivo>\n");
                    continue;
                }
                if (access(arg, R_OK) == -1) {
                    perror("Archivo local");
                    continue;
                }

                pid_t pid = fork();

                if (pid == 0) {
                    /* ==============================
                     * PROCESO HIJO: NUEVA SESIÓN
                     * ============================== */
                    close(s_main);
                    int s_child = connectTCP(g_host, g_service);
                    if (s_child < 0 || !auto_login(s_child)) exit(1);

                    sdata = pasivo(s_child);
                    sprintf(cmd, "STOR %s", arg);
                    sendCmd(s_child, cmd, res);

                    FILE *fp = fopen(arg, "rb");
                    printf("[Hijo %d] Subiendo '%s'...\n", getpid(), arg);

                    while ((n = fread(data, 1, LINELEN, fp)) > 0) {
                        send(sdata, data, n, 0);
                    }
                    fclose(fp);
                    close(sdata);
                    close(s_child);
                    printf("[Hijo %d] Subida Fin '%s'.\n", getpid(), arg);
                    exit(0);
                } else {
                    printf("[Padre] Subida '%s' en proceso %d\n", arg, pid);
                }

            /* --- COMANDO: PWD (Extra) --- */
            } else if (strcmp(ucmd, "pwd") == 0) {
                sprintf(cmd, "PWD");
                sendCmd(s_main, cmd, res);
                printf("%s", res);

            /* --- COMANDO: MKDIR (Extra) --- */
            } else if (strcmp(ucmd, "mkdir") == 0) {
                arg = strtok(NULL, " ");
                if (!arg) {
                    printf("Uso: mkdir <nombre_directorio>\n");
                    continue;
                }
                sprintf(cmd, "MKD %s", arg);
                sendCmd(s_main, cmd, res);
                printf("%s", res);

            /* --- COMANDO: DELETE (Extra) --- */
            } else if (strcmp(ucmd, "delete") == 0) {
                arg = strtok(NULL, " ");
                if (!arg) {
                    printf("Uso: delete <archivo>\n");
                    continue;
                }
                sprintf(cmd, "DELE %s", arg);
                sendCmd(s_main, cmd, res);
                printf("%s", res);

            /* --- COMANDO: QUIT --- */
            } else if (strcmp(ucmd, "quit") == 0) {
                sprintf(cmd, "QUIT");
                sendCmd(s_main, cmd, res);
                exit(0);

            /* --- COMANDO: CD --- */
            } else if (strcmp(ucmd, "cd") == 0) {
                arg = strtok(NULL, " ");
                sprintf(cmd, "CWD %s", arg);
                sendCmd(s_main, cmd, res);
                printf("%s", res);

            } else {
                printf("Comando desconocido.\n");
            }
        }
    }
}