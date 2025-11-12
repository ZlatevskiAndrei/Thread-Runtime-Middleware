#include "tasks_implem.h"
#include "tasks_queue.h"
#include "debug.h"
#include "worker_thread.h"
#include <signal.h>

pthread_t* tids;

void create_queues(void)
{
    tqueue = create_tasks_queue();
}

void delete_queues(void)
{
    free_tasks_queue(tqueue);
}    

void initialize_mutex(pthread_mutex_t* mutex, int* type) {
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, *type);
    
    if (pthread_mutex_init(mutex, &attr) != 0) {
        PRINT_DEBUG(1,"pthread_mutex_init failed");
    }

    pthread_mutexattr_destroy(&attr);
}

void initialize_cond(pthread_cond_t *cond, pthread_condattr_t *attr) {
    int ret;

    if (attr == NULL) {
        ret = pthread_cond_init(cond, NULL);
    } else {
        ret = pthread_cond_init(cond, attr);
    }

    if (ret != 0) {
        PRINT_DEBUG(1,"pthread_cond_init failed");
    }
}

void create_thread_pool(void)
{
    pthread_attr_t tattr;
    int ret;

    ret = pthread_attr_init(&tattr); //with default values for now
    assert (ret == 0);
    tids = malloc(THREAD_COUNT*sizeof(pthread_t));
    assert(tids != NULL);

    for(int i = 0 ; i < THREAD_COUNT; i++){
        if(pthread_create (&tids[i], &tattr, worker_routine, NULL) != 0){
            PRINT_DEBUG(100,"Failed to create thread number %d\n",i);
        }
    }
}

void kill_thread_pool(){
    for(int i = 0 ; i < THREAD_COUNT; i++){
        PRINT_DEBUG(10,"Killing worker thread: %ld\n", tids[i]);
        int r = pthread_kill(tids[i], SIGINT);
        assert(!r);
    }    
}

void dispatch_task(task_t *t)
{
    enqueue_task(tqueue, t);
}

task_t* get_task_to_execute(void)
{
    return dequeue_task(tqueue);
}

unsigned int exec_task(task_t *t)
{
    t->step++;
    t->status = RUNNING;

    PRINT_DEBUG(10, "Execution of task %u (step %u)\n", t->task_id, t->step);
    
    unsigned int result = t->fct(t, t->step);
    
    return result;
}

void terminate_task(task_t *t)
{
    t->status = TERMINATED;
    
    PRINT_DEBUG(10, "Task terminated: %u\n", t->task_id);

#ifdef WITH_DEPENDENCIES
    if(t->parent_task != NULL){
        task_t *waiting_task = t->parent_task;
        waiting_task->task_dependency_done++;
        
        task_check_runnable(waiting_task);
    }
#endif

}

void task_check_runnable(task_t *t)
{
#ifdef WITH_DEPENDENCIES
    if(t->task_dependency_done == t->task_dependency_count){
        t->status = READY;
        dispatch_task(t);
    }
#endif
}
