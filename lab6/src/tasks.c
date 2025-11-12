#include <stdlib.h>
#include <sys/syscall.h>
#include <stdatomic.h>
#include <linux/futex.h>
#include <unistd.h>

#include "tasks.h"
#include "tasks_implem.h"
#include "debug.h"
#include "utils.h"

system_state_t sys_state;

__thread task_t *active_task;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t non_full = PTHREAD_COND_INITIALIZER;
pthread_cond_t non_empty = PTHREAD_COND_INITIALIZER;
tasks_queue_t* tqueue = NULL;
_Atomic bool barrier;
_Atomic int nb_executed_tasks;

void runtime_init(void)
{
    rand_generator_init();

    initialize_mutex(&mutex, NULL);
    initialize_cond(&non_empty, NULL);
    initialize_cond(&non_full, NULL);
    __atomic_store_n(&barrier, 0, __ATOMIC_RELAXED);
    __atomic_store_n(&nb_executed_tasks, 0, __ATOMIC_RELAXED);
    create_queues();
    create_thread_pool();


    sys_state.task_counter = 0;    
}

void runtime_init_with_deps(void)
{
#ifndef WITH_DEPENDENCIES
    fprintf(stderr, "ERROR: dependencies are not supported by the runtime. This application cannot be executed\n");
    exit(EXIT_FAILURE);
#endif

    runtime_init();
}



void runtime_finalize(void)
{
    task_waitall();
    PRINT_DEBUG(1, "Terminating ... \t Total task count: %lu \n", sys_state.task_counter);
    delete_queues();
}


task_t* create_task(task_routine_t f)
{
    task_t *t = malloc(sizeof(task_t));

    t->task_id = ++sys_state.task_counter;    
    t->fct = f;
    t->step = 0;

    t->tstate.input_list = NULL;
    t->tstate.output_list = NULL;

#ifdef WITH_DEPENDENCIES
    t->tstate.output_from_dependencies_list = NULL;
    t->task_dependency_count = 0;
    t->parent_task = NULL;
#endif
    
    t->status = INIT;
    
    PRINT_DEBUG(10, "task created with id %u\n", t->task_id);    
    
    return t;
}

void submit_task(task_t *t)
{
    t->status = READY;

#ifdef WITH_DEPENDENCIES    
    if(active_task != NULL){
        t->parent_task = active_task;
        active_task->task_dependency_count++;
        
        PRINT_DEBUG(100, "Dependency %u -> %u\n", active_task->task_id, t->task_id);
    }
#endif
    
    dispatch_task(t);
}


void task_waitall(void)
{
    __atomic_store_n(&barrier, 1, __ATOMIC_RELAXED);
    while(__atomic_load_n(&nb_executed_tasks, __ATOMIC_RELAXED) < sys_state.task_counter){
        __atomic_store_n(&barrier, 1, __ATOMIC_RELAXED);
        PRINT_DEBUG(100, "SYSCALL!\n");
        syscall(SYS_futex, &barrier, FUTEX_WAIT, 1, NULL, NULL, 0);
    }
    return ;
}