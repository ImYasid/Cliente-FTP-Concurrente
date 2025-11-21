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

**Ejemplos:**

* **Local**

  ```bash
  ./JimenezY-clienteFTP 127.0.0.1
  ```

* **Red Local**

  ```bash
  ./JimenezY-clienteFTP 192.168.1.50
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

puedes lanzar múltiples operaciones sin esperar a que terminen las anteriores.
Las transferencias (`get` y `put`) se ejecutan en **segundo plano**, manteniendo la conexión de control libre.

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

## 🧭 Comandos Implementados

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

---

## 📌 Notas Técnicas

* Todas las transferencias (`RETR` y `STOR`) se ejecutan en **modo pasivo (PASV)**.
* El cliente mantiene la **conexión de control no bloqueante**, permitiendo interacción continua.
* Cada transferencia se ejecuta en un **hilo independiente**, permitiendo concurrencia real.
* Los comandos administrativos (`cd`, `pwd`, `delete`, `mkdir`) son síncronos.