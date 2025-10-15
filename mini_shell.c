#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_INPUT 100
#define MAX_ARGS 10

int main() {
    char input[MAX_INPUT];
    char *args[MAX_ARGS];
    pid_t pid;
    int status;

    while (1) {
        // Mostrar prompt
        printf("mini-sheell@usuario> ");
        fflush(stdout);

        // Leer comando
        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("\n");
            break; //en el caso de error
        }

        // Quitar salto de línea
        input[strcspn(input, "\n")] = '\0';

        // Si el usuario escribe "salir"
        if (strcmp(input, "salir") == 0) {
            printf("Saliendo ...\n");
            break;
        }

        // Dividir el comando en argumentos
        int i = 0;
        args[i] = strtok(input, " ");
        while (args[i] != NULL && i < MAX_ARGS - 1) {
            i++;
            args[i] = strtok(NULL, " ");
        }
        args[i] = NULL;

        // COntinua si no se escribe nada
        if (args[0] == NULL) {
            continue;
        }

        // Detectar ejecución en segundo plano
        int background = 0;
        if (i > 0 && strcmp(args[i - 1], "&") == 0) {
            background = 1;
            args[i - 1] = NULL; // eliminar '&' de los argumentos
        }
        //Detecta redirecciones y pipes
        int redir_out = 0, redir_append = 0, redir_in = 0, has_pipe = 0;
        char *file_out = NULL, *file_in = NULL;
        char *args1[MAX_ARGS], *args2[MAX_ARGS];

        // verifica si hay tuberias (|)
        int j = 0, k = 0;
        for (i = 0; args[i] != NULL; i++) {
            if (strcmp(args[i], "|") == 0) {
                has_pipe = 1;
                args[j] = NULL;
                i++;
                while (args[i] != NULL) {
                    args2[k++] = args[i++];
                }
                args2[k] = NULL;
                break;
            }
            args1[j++] = args[i];
        }
        args1[j] = NULL;

        //SI NO hay pipe, procesar redirecciones
        if (!has_pipe) {
            for (i = 0; args[i] != NULL; i++) {
                if (strcmp(args[i], ">") == 0) {
                    redir_out = 1;
                    file_out = args[i + 1];
                    args[i] = NULL;
                    break;
                } else if (strcmp(args[i], ">>") == 0) {
                    redir_append = 1;
                    file_out = args[i + 1];
                    args[i] = NULL;
                    break;
                } else if (strcmp(args[i], "<") == 0) {
                    redir_in = 1;
                    file_in = args[i + 1];
                    args[i] = NULL;
                    break;
                }
            }
        }
        // Crear proceso hijo
        pid = fork();

        if (pid < 0) {
            perror("Error en fork");
        } 
        else if (pid == 0) {
            // Proceso hijo: ejecutar comando
            if (has_pipe) {
                // PIPE
                int fd[2];
                pipe(fd);
                pid_t pid2 = fork();
                if (pid2 == 0) {
                    // Primer comando
                    dup2(fd[1], STDOUT_FILENO);
                    close(fd[0]);
                    close(fd[1]);
                    execvp(args1[0], args1);
                    perror("Error en execvp 1");
                    exit(1);
                } else {
                    // Segundo comando
                    dup2(fd[0], STDIN_FILENO);
                    close(fd[0]);
                    close(fd[1]);
                    execvp(args2[0], args2);
                    perror("Error en execvp 2");
                    exit(1);
                }
            }

            // Redirección de salida >
            if (redir_out) {
                int fd = open(file_out, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                dup2(fd, STDOUT_FILENO);
                close(fd);
            }

            // Redirección de salida doble >>
            if (redir_append) {
                int fd = open(file_out, O_WRONLY | O_CREAT | O_APPEND, 0644);
                dup2(fd, STDOUT_FILENO);
                close(fd);
            }

            // Redirección de entrada <
            if (redir_in) {
                int fd = open(file_in, O_RDONLY);
                if (fd < 0) {
                    perror("No se puede abrir el archivo");
                    exit(1);
                }
                dup2(fd, STDIN_FILENO);
                close(fd);
            }

            // Ejecutar comando normal
            execvp(args[0], args);
            perror("Error al ejecutar el comando");
            exit(1);
        } 
        else {
            // Proceso padre: esperar al hijo
            if (background) {
                printf("[PID %d] ejecutándose en segundo plano...\n", pid);
            } else {
                waitpid(pid, &status, 0);
            }
        }
    }

    return 0;
}