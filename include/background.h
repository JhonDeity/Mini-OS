#ifndef BACKGROUND_H
#define BACKGROUND_H

#include "shell.h"

void wait_for_background_jobs();
void *monitor_background(void *arg);

#endif