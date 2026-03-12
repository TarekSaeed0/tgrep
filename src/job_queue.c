#include <job_queue.h>

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

JobQueue job_queue_new(void) {
	return (JobQueue){ .head = NULL, .tail = NULL };
}
bool job_queue_enqueue(JobQueue *queue, Job job) {
	assert(queue != NULL);

	JobQueueNode *node = malloc(sizeof(*node));
	if (node == NULL) {
		return false;
	}

	node->job  = job;
	node->next = NULL;

	if (queue->tail != NULL) {
		queue->tail->next = node;
	} else {
		queue->head = node;
	}

	queue->tail = node;

	return true;
}
bool job_queue_dequeue(JobQueue *queue, Job *job) {
	assert(queue != NULL);

	if (queue->head == NULL) {
		return false;
	}

	JobQueueNode *node = queue->head;
	queue->head        = queue->head->next;

	if (queue->head == NULL) {
		queue->tail = NULL;
	}

	*job = node->job;
	free(node);

	return true;
}
bool job_queue_is_empty(const JobQueue *queue) {
	assert(queue != NULL);

	return queue->head == NULL;
}
