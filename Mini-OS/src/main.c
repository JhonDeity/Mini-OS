#include "shell.h"
#include "background.h"
#include "utils.h"

int main() {
    char input[MAX_INPUT];
    char *args[MAX_ARGS];
    pid_t pid;
    int status;

    if (signal(SIGINT, ignore_signal) == SIG_ERR) {
        perror("Error al establecer el manejador de SIGINT");
    }
    
    while (1) {
        wait_for_background_jobs();
        printf("mini-sheell@usuario> ");
        fflush(stdout);

        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("\n");
            break;
        }

        input[strcspn(input, "\n")] = '\0';
        int i = 0;
        args[i] = strtok(input, " \t\n");
        while (args[i] != NULL && i < MAX_ARGS - 1) {
            i++;
            args[i] = strtok(NULL, " \t\n");
        }
        args[i] = NULL;

        if (args[0] == NULL) continue;
        if (strcmp(args[0], "salir") == 0) {
            printf("Saliendo ...\n");
            break;
        }

        if (strcmp(args[0], "cd") == 0) {
            char *dir = args[1] ? args[1] : getenv("HOME");
            if (!dir) fprintf(stderr, "cd: No se pudo determinar el directorio HOME.\n");
            else if (chdir(dir) != 0) perror("cd");
            continue;
        }

        int background = 0;
        if (i > 0 && strcmp(args[i-1], "&") == 0) {
            background = 1;
            args[i-1] = NULL;
        }

        pid = fork();
        if (pid < 0) {
            perror("Error en fork");
            continue;
        } 
        else if (pid == 0) {
            signal(SIGINT, SIG_DFL);
            execvp(args[0], args);
            perror("Error al ejecutar el comando");
            exit(1);
        } 
        else {
            if (background) {
                bg_job_t *job = malloc(sizeof(bg_job_t));
                if (job) {
                    job->pid = pid;
                    snprintf(job->cmd, CMD_STR_LEN, "%s", args[0]);
                    pthread_t tid;
                    pthread_create(&tid, NULL, monitor_background, job);
                    pthread_detach(tid);
                    pthread_mutex_lock(&print_mutex);
                    printf("[PID %d] ejecutándose en segundo plano...\n", pid);
                    pthread_mutex_unlock(&print_mutex);
                }
            } else {
                waitpid(pid, &status, 0);
            }
        }
    }
    return 0;
}