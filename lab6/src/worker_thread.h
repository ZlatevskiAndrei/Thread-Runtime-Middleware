#ifndef __WORKER_THREAD__
#define __WORKER_THREAD__


#include "tasks_types.h"
#include "tasks_implem.h"
#include "tasks_queue.h"


void* worker_routine(void* arg);
task_t* fetch(void);
void run(task_t*);


#endif