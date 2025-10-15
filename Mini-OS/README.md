# 🐚 Mini Shell en C

Este proyecto implementa un **mini shell (intérprete de comandos)** desarrollado en **C**, con soporte para **ejecución de procesos**, **pipes**, **redirección de entrada/salida**, **tareas en segundo plano**, y una **estructura modular** para facilitar su mantenimiento.

---

## 🧩 Estructura del Proyecto

- 📁 **src/** → código fuente (.c)
- 📁 **include/** → headers (.h)
- 🧾 **README.md** → documentación

## ⚙️ Características del Mini Shell

- 🧠 **Interfaz interactiva:** muestra un *prompt* (`mini-shell@usuario>`) y espera comandos del usuario.  
- 🚀 **Ejecución de comandos externos:** permite ejecutar cualquier programa del sistema
- 🧱 **Comandos internos:**
  - `cd`: cambia de directorio.  
  - `salir`: termina la ejecución del shell.  
  - `help`: muestra una lista de comandos disponibles y su descripción.  
- 🔁 **Redirección de entrada/salida:**
  - `>`  redirige la salida estándar a un archivo.  
  - `>>` agrega la salida al final de un archivo existente.  
  - `<`  redirige la entrada estándar desde un archivo.  
- 🔗 **Soporte de tuberías (pipes):**  
  Permite conectar comandos con `|`, por ejemplo:
  ```bash
  ls | grep ".c"
## 🧰 Compilación y Ejecución

### 🔹 Requisitos
- Sistema operativo tipo **Unix/Linux**  
- Compilador **gcc** instalado  

### 🔹 Clonar el repositorio
Primero clona el proyecto desde GitHub:
```bash
git clone https://github.com/usuario/minishell.git
cd minishell
gcc src/*.c -Iinclude -o mini-shell -lpthread
./mini-shell
