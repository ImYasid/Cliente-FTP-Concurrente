# Makefile para Cliente FTP Concurrente
# Estudiante: Yasid Jimenez

CC = gcc
# _DEFAULT_SOURCE es necesario para funciones como getpass y evitar warnings
CFLAGS = -Wall -g -D_DEFAULT_SOURCE

# Nombre del ejecutable final (Basado en tu Apellido e Inicial)
TARGET = JimenezY-clienteFTP

# Lista de objetos: Tu programa principal + Módulos del profesor
# Asegúrate de que tu archivo principal se llame 'JimenezY-clienteFTP.c'
OBJS = $(TARGET).o connectsock.o connectTCP.o passivesock.o passiveTCP.o errexit.o

all: $(TARGET)

# Regla para enlazar (link) todos los objetos en el ejecutable final
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

# Regla genérica para compilar cualquier .c a .o
%.o: %.c
	$(CC) $(CFLAGS) -c $<

# Limpieza de archivos temporales y ejecutable
clean:
	rm -f $(TARGET) $(OBJS)