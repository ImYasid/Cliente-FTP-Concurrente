# 🌐 Cliente FTP Concurrente (RFC 959)

Este proyecto implementa un **cliente FTP concurrente** desarrollado en **C**, siguiendo estrictamente la especificación del **RFC 959**.
Su principal característica es la capacidad de realizar **múltiples transferencias simultáneas** (subidas y descargas) sin bloquear la conexión de control ni la interacción del usuario.

---

## 🚀 Instrucciones de Ejecución

Para ejecutar el cliente en un entorno **Linux o WSL**, sigue estos pasos:

### 1. Compilación

El proyecto utiliza **Makefile** para compilar todos los módulos de red y enlazarlos con la lógica principal.

```bash
make
```

Esto generará el ejecutable:

```
JimenezY-clienteFTP
```

---

### 2. Ejecución del Cliente

Ejecuta el binario especificando la IP del servidor FTP:

```bash
./JimenezY-clienteFTP <IP_SERVIDOR>
```

**Ejemplo:**

* **Local**

  ```bash
  ./JimenezY-clienteFTP 127.0.0.1
  ```

---

### 3. Autenticación

Cuando el cliente lo solicite, ingresa tu **usuario** y **contraseña** del servidor FTP.

---

### 4. Operación Concurrente

Cuando aparezca el prompt:

```
ftp>
```

puedes ejecutar múltiples comandos sin esperar a que finalicen los anteriores.
Las transferencias (`get` y `put`) se ejecutan en **segundo plano**, manteniendo siempre libre la conexión de control.

---

## 🧭 Comandos Implementados

```markdown
| Comando           | Descripción                                | Tipo               |
| ----------------- | ------------------------------------------ | ------------------ |
| `dir`             | Lista el directorio actual                 | Síncrono           |
| `get <archivo>`   | Descarga un archivo                        | **Asíncrono** (BG) |
| `put <archivo>`   | Sube un archivo                            | **Asíncrono** (BG) |
| `cd <dir>`        | Cambia el directorio remoto                | —                  |
| `pwd`             | Muestra el directorio remoto actual        | Extra              |
| `mkdir <nombre>`  | Crea un directorio                         | Extra              |
| `delete <nombre>` | Elimina un archivo                         | Extra              |
| `quit`            | Finaliza la sesión FTP y cierra el cliente | —                  |
```

---
## 📁 Estructura del Proyecto

```
CLIENTE-FTP-CONCURRENTE/
├── .gitignore                 # Archivos ignorados (.o, binarios, etc.)
├── JimenezY-clienteFTP.c      # Código principal (main + lógica multisesión)
├── Makefile                   # Compilación completa del proyecto
├── README.md                  # Documentación
├── connectsock.c              # Creación de sockets cliente
├── connectTCP.c               # Wrapper TCP para cliente
├── passivesock.c              # Creación de sockets servidor (modo pasivo)
├── passiveTCP.c               # Wrapper TCP para servidor
└── errexit.c                  # Manejo de errores y abortos controlados

```
---

## 📡 Servidor FTP Utilizado (vsftpd)

Para realizar las pruebas se utilizó **vsftpd 3.0.5**, identificado en los logs como:

```
220 (vsFTPd 3.0.5)
```

Es uno de los servidores FTP más seguros y utilizados en Linux.

### Instalación Rápida

```bash
sudo apt update
sudo apt install vsftpd
```

### Configuración Esencial (Modo Activo)

Para permitir operaciones en **modo activo** —especialmente para pruebas con comandos como `pput`— se deben habilitar estas opciones en `/etc/vsftpd.conf`:

```ini
listen=YES
local_enable=YES
write_enable=YES

# Modo Activo
port_enable=YES
connect_from_port_20=YES
port_promiscuous=YES
```
