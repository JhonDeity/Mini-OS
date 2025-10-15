#ifndef SHELL_H
#define SHELL_H

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

typedef struct {
    pid_t pid;
    char cmd[CMD_STR_LEN];
} bg_job_t;

extern pthread_mutex_t print_mutex;

#endif