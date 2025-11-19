#include <stdio.h>
#include <stdlib.h>
#include <linux/futex.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <assert.h>

#include "tasks_queue.h"
#include "tasks_implem.h"
#include "debug.h"

tasks_queue_t* create_tasks_queue(void)
{
    tasks_queue_t *q = (tasks_queue_t*) malloc(sizeof(tasks_queue_t));

    q->task_buf_size = QUEUE_CAPACITY;
    q->task_buffer = (task_t**) malloc(sizeof(task_t*) * q->task_buf_size);

    q->index = 0;

    return q;
}


void free_tasks_queue(tasks_queue_t *q)
{
    /* IMPORTANT: We chose not to free the queues to simplify the
     * termination of the program (and make debugging less complex) */
    
    /* free(q->task_buffer); */
    /* free(q); */
}


void enqueue_task(tasks_queue_t *q, task_t *t)
{
    pthread_mutex_lock(&mutex);
    if(q->index == q->task_buf_size) {
        q->task_buf_size += QUEUE_CAPACITY;
        task_t **temp = realloc(q->task_buffer, sizeof(task_t) * q->task_buf_size);
        if (temp == NULL) {
            exit(EXIT_FAILURE);  
        }
        q->task_buffer = temp;  
    }
    PRINT_DEBUG(100,"Inserted a task successfully! Number of elements: %d\n", q->index + 1);
    q->task_buffer[q->index] = t;
    q->index++;
    pthread_cond_broadcast(&non_empty);
    pthread_mutex_unlock(&mutex);
}


task_t* dequeue_task(tasks_queue_t *q)
{
    pthread_mutex_lock(&mutex);
    while(q->index <= 0){
        if(__sync_bool_compare_and_swap(&barrier, 1, 0) && __atomic_load_n(&last_parent_processed, __ATOMIC_RELAXED)){
            PRINT_DEBUG(100, "SYSCALL!\n");
            syscall(SYS_futex, &barrier, FUTEX_WAKE, 1, NULL, NULL, 0);
        }
        pthread_cond_wait(&non_empty, &mutex);
    }
    task_t *t = q->task_buffer[q->index-1];
    q->index--;
    pthread_mutex_unlock(&mutex);
    return t; 
}