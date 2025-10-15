#include "background.h"

pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;

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