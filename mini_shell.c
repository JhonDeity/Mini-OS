#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h> 
#include <signal.h> 
#include <pthread.h>  

#define MAX_INPUT 100
#define MAX_ARGS 10
#define CMD_STR_LEN 256 

void wait_for_background_jobs();
void ignore_signal(int signo);
void *monitor_background(void *arg);   
pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
    pid_t pid;
    char cmd[CMD_STR_LEN];
} bg_job_t;

void ignore_signal(int signo) {
    (void)signo;
}

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

        // Leer comando
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


        if (args[0] == NULL) {
            continue;
        }

        if (strcmp(args[0], "salir") == 0) {
            printf("Saliendo ...\n");
            break;
        }

        if (strcmp(args[0], "cd") == 0) {
            char *dir = args[1];
            if (dir == NULL) {
                dir = getenv("HOME"); 
            }
            if (dir == NULL) {
                fprintf(stderr, "cd: No se pudo determinar el directorio HOME.\n");
            } else if (chdir(dir) != 0) {
                perror("cd"); 
            }
            continue; 
        }

        // Detectar ejecucion en segundo plano
        int background = 0;
        int last_arg_index = i - 1;
        if (last_arg_index >= 0 && args[last_arg_index] != NULL && strcmp(args[last_arg_index], "&") == 0) {
            background = 1;
            args[last_arg_index] = NULL; 
            i = last_arg_index;
        }

        // --- PARSING DE REDIRECCIONES Y PIPES ---
        
        int redir_out = 0, redir_append = 0, redir_in = 0, has_pipe = 0;
        char *file_out = NULL, *file_in = NULL;
        char *args1[MAX_ARGS], *args2[MAX_ARGS];
        
        // Verifica si hay tuberías (|)
        int j = 0, k = 0;
        for (int l = 0; args[l] != NULL; l++) {
            if (strcmp(args[l], "|") == 0) {
                has_pipe = 1;
                args1[j] = NULL; 
                l++;
                while (args[l] != NULL) {
                    args2[k++] = args[l++];
                }
                args2[k] = NULL;
                break;
            }
            args1[j++] = args[l];
        }
        args1[j] = NULL;

        //SI NO hay pipe, procesar redirecciones
        if (!has_pipe) {
            int final_args_count = 0;
            for (int l = 0; args[l] != NULL; l++) { // Usamos 'args' original aquí
                if (strcmp(args[l], ">") == 0) {
                    redir_out = 1;
                    file_out = args[l + 1];
                    args[l] = NULL;
                    break;
                } else if (strcmp(args[l], ">>") == 0) {
                    redir_append = 1;
                    file_out = args[l + 1];
                    args[l] = NULL;
                    break;
                } else if (strcmp(args[l], "<") == 0) {
                    redir_in = 1;
                    file_in = args[l + 1];
                    args[l] = NULL;
                    break;
                }
                args1[final_args_count++] = args[l];
            }
            args1[final_args_count] = NULL;

            // Mover la lista final de args1 a args para la ejecución si no hay pipe
            int l = 0;
            while (args1[l] != NULL) {
                args[l] = args1[l];
                l++;
            }
            args[l] = NULL;
        }

        pid = fork();

        if (pid < 0) {
            perror("Error en fork");
            continue;
        } 
        else if (pid == 0) {

            signal(SIGINT, SIG_DFL);
            
            if (has_pipe) {

                int fd[2];
                if (pipe(fd) == -1) { 
                    perror("pipe"); 
                    exit(1); 
                }
                
                pid_t pid2 = fork();
                if (pid2 == 0) {

                    close(fd[0]); 
                    dup2(fd[1], STDOUT_FILENO);
                    close(fd[1]);
                    execvp(args1[0], args1);
                    perror("Error en execvp 1");
                    exit(1);
                } else {

                    close(fd[1]); 
                    dup2(fd[0], STDIN_FILENO);
                    close(fd[0]);
                    
                    waitpid(pid2, NULL, 0); 
                    
                    execvp(args2[0], args2);
                    perror("Error en execvp 2");
                    exit(1);
                }
            }

            if (redir_out || redir_append) {
                int flags = O_WRONLY | O_CREAT;
                if (redir_out) flags |= O_TRUNC;
                if (redir_append) flags |= O_APPEND;

                int fd = open(file_out, flags, 0644);
                if (fd < 0) { perror("Error al abrir archivo de salida"); exit(1); }
                dup2(fd, STDOUT_FILENO);
                close(fd);
            }

            if (redir_in) {
                int fd = open(file_in, O_RDONLY);
                if (fd < 0) {
                    perror("No se puede abrir el archivo de entrada");
                    exit(1);
                }
                dup2(fd, STDIN_FILENO);
                close(fd);
            }

            execvp(args[0], args);
            perror("Error al ejecutar el comando");
            exit(1);
        } 
        else {
            if (background) {
                char cmdbuf[CMD_STR_LEN] = {0};
                int idx = 0;
                for (int t = 0; args[t] != NULL && idx + 1 < CMD_STR_LEN; t++) {
                    int n = snprintf(cmdbuf + idx, CMD_STR_LEN - idx, "%s ", args[t]);
                    if (n < 0) break;
                    idx += n;
                }

                bg_job_t *job = malloc(sizeof(bg_job_t));
                if (!job) {
                    fprintf(stderr, "Error al asignar memoria para job\n");
                } else {
                    job->pid = pid;
                    strncpy(job->cmd, cmdbuf, CMD_STR_LEN - 1);
                    job->cmd[CMD_STR_LEN - 1] = '\0';

                    pthread_t tid;

                    if (pthread_create(&tid, NULL, monitor_background, job) != 0) {
                        perror("Error al crear hilo monitor");
                        free(job);
                    } else {
                        pthread_detach(tid);
                        pthread_mutex_lock(&print_mutex);
                        printf("[PID %d] ejecutándose en segundo plano...\n", pid);
                        pthread_mutex_unlock(&print_mutex);
                    }
                }
            } else {
                waitpid(pid, &status, 0);
            }
        }
    }

    return 0;
}

void wait_for_background_jobs() {
    pid_t pid;
    int status;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        pthread_mutex_lock(&print_mutex); 
        fprintf(stderr, "\n[PID %d] Tarea en segundo plano terminada.\n", pid);
        pthread_mutex_unlock(&print_mutex);
    }
}


void *monitor_background(void *arg) {
    if (!arg) return NULL;
    bg_job_t *job = (bg_job_t *)arg;
    pid_t pid = job->pid;
    int status;

    pid_t r = waitpid(pid, &status, 0);
    pthread_mutex_lock(&print_mutex);
    if (r == -1) {
        fprintf(stderr, "\n[PID %d] finalizó (error: %s).\n", pid, strerror(errno));
    } else if (WIFEXITED(status)) {
        fprintf(stderr, "\n[PID %d] Tarea terminada. Salida: %d. Comando: %s\n",
                pid, WEXITSTATUS(status), job->cmd);
    } else if (WIFSIGNALED(status)) {
        fprintf(stderr, "\n[PID %d] Terminó por señal %d. Comando: %s\n",
                pid, WTERMSIG(status), job->cmd);
    }
    pthread_mutex_unlock(&print_mutex);

    free(job);   
    return NULL;
}