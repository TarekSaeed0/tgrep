#ifndef TASK_QUEUE_H_INCLUDED
#define TASK_QUEUE_H_INCLUDED

#include <task.h>

typedef struct {
	Task *head;
	Task *tail;
} TaskQueue;

TaskQueue thread_queue_new(void);
void task_queue_enqueue(TaskQueue *queue, Task *task);
Task *task_queue_dequeue(TaskQueue *queue);

#endif // TASK_QUEUE_H_INCLUDED
