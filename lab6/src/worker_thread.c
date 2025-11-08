#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include <stdlib.h>

#include "worker_thread.h"

void* worker_routine(void* arg){
    return NULL;
}
task_t* fetch(void){
    return NULL;
}
task_return_value_t run(task_t*){
    task_return_value_t t = TASK_COMPLETED;
    return t;
}
