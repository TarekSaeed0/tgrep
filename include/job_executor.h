#ifndef JOB_EXECUTOR_H_INCLUDED
#define JOB_EXECUTOR_H_INCLUDED

#include <job.h>
#include <job_queue.h>

#include <pthread.h>
#include <stdbool.h>

typedef enum {
	JOB_EXECUTOR_STATUS_RUNNING,
	JOB_EXECUTOR_STATUS_STOPPING,
	JOB_EXECUTOR_STATUS_WAITING,
} JobExecutorStatus;

typedef struct {
	pthread_t *workers;
	size_t worker_count;
	JobQueue job_queue;
	pthread_mutex_t mutex;
	pthread_cond_t update;
	JobExecutorStatus status;
} JobExecutor;

bool job_executor_new(JobExecutor *executor, size_t worker_count);
bool job_executor_start(JobExecutor *executor);
void job_executor_stop(JobExecutor *executor);
void job_executor_wait(JobExecutor *executor);
bool job_executor_submit(JobExecutor *executor, Job job);

#endif // JOB_EXECUTOR_H_INCLUDED
