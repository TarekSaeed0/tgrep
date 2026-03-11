#include <task_queue.h>

#include <assert.h>
#include <stddef.h>

TaskQueue thread_queue_new(void) {
	return (TaskQueue){ .head = NULL, .tail = NULL };
}
void task_queue_enqueue(TaskQueue *queue, Task *task) {
	assert(queue != NULL);
	assert(task != NULL);

	task->next = NULL;

	if (queue->tail != NULL) {
		queue->tail->next = task;
	} else {
		queue->head = task;
	}

	queue->tail = task;
}
Task *task_queue_dequeue(TaskQueue *queue) {
	assert(queue != NULL);

	if (queue->head == NULL) {
		return NULL;
	}

	Task *task  = queue->head;
	queue->head = queue->head->next;

	if (queue->head == NULL) {
		queue->tail = NULL;
	}

	return task;
}
