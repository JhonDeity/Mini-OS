#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

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
            break; //en el caso de error o control + D
        }

        // Quitar salto de línea
        input[strcspn(input, "\n")] = '\0';

        // Si el usuario escribe "exit", salir
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

        // Crear proceso hijo
        pid = fork();

        if (pid < 0) {
            perror("Error en fork");
        } 
        else if (pid == 0) {
            // Proceso hijo: ejecutar comando
            execvp(args[0], args);
            perror("Error al ejecutar el comando");
            exit(1);
        } 
        else {
            // Proceso padre: esperar al hijo
            waitpid(pid, &status, 0);
        }
    }

    return 0;
}