#ifndef __TASKS_IMPLEM_H__
#define __TASKS_IMPLEM_H__

#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include <stdlib.h>

#include "tasks_types.h"
#include "tasks_queue.h"

void create_queues(void);
void delete_queues(void);

void create_thread_pool(void);
void kill_thread_pool(void);
void initialize_mutex(pthread_mutex_t* mutex, int* type);
void initialize_cond(pthread_cond_t *cond, pthread_condattr_t *attr);

void dispatch_task(task_t *t);
task_t* get_task_to_execute(void);
unsigned int exec_task(task_t *t);
void terminate_task(task_t *t);

void task_check_runnable(task_t *t);

extern tasks_queue_t* tqueue;
extern pthread_mutex_t mutex;
extern pthread_cond_t non_full;
extern pthread_cond_t non_empty;


#endif
