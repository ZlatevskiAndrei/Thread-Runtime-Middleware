#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include <stdlib.h>
#include <linux/futex.h>
#include <sys/syscall.h>
#include <unistd.h>

#include "worker_thread.h"
#include "tasks_implem.h"
#include "debug.h"

void* worker_routine(void* arg){
    while(1){
        active_task = fetch();
        run(active_task);
    }
    return NULL;
}
task_t* fetch(void){
    return get_task_to_execute();
}


void run(task_t* task){
    task_return_value_t ret = exec_task(task);
    if (ret == TASK_COMPLETED){
        __sync_fetch_and_add(&nb_executed_tasks, 1); 
        terminate_task(task);
    }
    else if (ret == TASK_TO_BE_RESUMED) {
        //task->step++;
        
        pthread_mutex_lock(&task->mutex_dependency);
        
        if (task->task_dependency_done == task->task_dependency_count) {
            task->status = READY;
            pthread_mutex_unlock(&task->mutex_dependency);
            dispatch_task(task);
        } 
        else {
            task->status = WAITING;
            pthread_mutex_unlock(&task->mutex_dependency);
        }
    }
        
       
    }
 
 
