#ifndef JOB_QUEUE_H_INCLUDED
#define JOB_QUEUE_H_INCLUDED

#include <job.h>

#include <stdbool.h>

typedef struct JobQueueNode {
	Job job;
	struct JobQueueNode *next;
} JobQueueNode;
typedef struct JobQueue {
	JobQueueNode *head;
	JobQueueNode *tail;
} JobQueue;

JobQueue job_queue_new(void);
void job_queue_drop(JobQueue *queue);
bool job_queue_enqueue(JobQueue *queue, Job job);
bool job_queue_dequeue(JobQueue *queue, Job *job);
bool job_queue_is_empty(const JobQueue *queue);

#endif // JOB_QUEUE_H_INCLUDED
