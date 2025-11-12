#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include <stdlib.h>

#include "worker_thread.h"
#include "tasks_implem.h"

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
        terminate_task(active_task);
    }
}
