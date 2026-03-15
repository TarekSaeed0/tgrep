#include "job_queue.h"
#include <job_executor.h>

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

void *job_executor_worker(void *data) {
	JobExecutor *executor = data;
	assert(executor != NULL);

	while (true) {
		pthread_mutex_lock(&executor->mutex);

		while (job_queue_is_empty(&executor->job_queue) && executor->status == JOB_EXECUTOR_STATUS_RUNNING) {
			pthread_cond_wait(&executor->update, &executor->mutex);
		}

		if (job_queue_is_empty(&executor->job_queue) || executor->status == JOB_EXECUTOR_STATUS_STOPPING) {
			pthread_mutex_unlock(&executor->mutex);
			break;
		}

		Job job;
		job_queue_dequeue(&executor->job_queue, &job);

		pthread_mutex_unlock(&executor->mutex);

		job.function(executor, job.data);
	}

	return NULL;
}

bool job_executor_new(JobExecutor *executor, size_t worker_count) {
	assert(executor != NULL);
	assert(worker_count != 0);

	executor->workers = malloc(worker_count * sizeof(*executor->workers));
	if (executor->workers == NULL) {
		return false;
	}

	executor->worker_count = worker_count;
	executor->job_queue    = job_queue_new();

	if (pthread_mutex_init(&executor->mutex, NULL) != 0) {
		job_queue_drop(&executor->job_queue);
		free(executor->workers);
		return false;
	}

	if (pthread_cond_init(&executor->update, NULL) != 0) {
		pthread_mutex_destroy(&executor->mutex);
		job_queue_drop(&executor->job_queue);
		free(executor->workers);
		return false;
	}

	executor->status = JOB_EXECUTOR_STATUS_RUNNING;

	for (size_t i = 0; i < executor->worker_count; i++) {
		if (pthread_create(&executor->workers[i], NULL, job_executor_worker, executor) != 0) {
			pthread_mutex_lock(&executor->mutex);
			executor->status = JOB_EXECUTOR_STATUS_STOPPING;
			pthread_cond_broadcast(&executor->update);
			pthread_mutex_unlock(&executor->mutex);

			for (size_t j = 0; j < i; j++) {
				pthread_join(executor->workers[j], NULL);
			}

			pthread_cond_destroy(&executor->update);
			pthread_mutex_destroy(&executor->mutex);
			job_queue_drop(&executor->job_queue);
			free(executor->workers);

			return false;
		}
	}

	return true;
}
void job_executor_drop(JobExecutor *executor) {
	assert(executor != NULL);

	for (size_t i = 0; i < executor->worker_count; i++) {
		pthread_join(executor->workers[i], NULL);
	}

	pthread_cond_destroy(&executor->update);
	pthread_mutex_destroy(&executor->mutex);
	job_queue_drop(&executor->job_queue);
	free(executor->workers);
}
void job_executor_stop(JobExecutor *executor) {
	assert(executor != NULL);

	pthread_mutex_lock(&executor->mutex);
	if (executor->status != JOB_EXECUTOR_STATUS_RUNNING && executor->status != JOB_EXECUTOR_STATUS_DRAINING) {
		pthread_mutex_unlock(&executor->mutex);
		return;
	}

	executor->status = JOB_EXECUTOR_STATUS_STOPPING;
	pthread_cond_broadcast(&executor->update);
	pthread_mutex_unlock(&executor->mutex);
}
void job_executor_drain(JobExecutor *executor) {
	assert(executor != NULL);

	pthread_mutex_lock(&executor->mutex);
	if (executor->status != JOB_EXECUTOR_STATUS_RUNNING) {
		pthread_mutex_unlock(&executor->mutex);
		return;
	}

	executor->status = JOB_EXECUTOR_STATUS_DRAINING;
	pthread_cond_broadcast(&executor->update);
	pthread_mutex_unlock(&executor->mutex);
}
bool job_executor_submit(JobExecutor *executor, Job job) {
	assert(executor != NULL);

	pthread_mutex_lock(&executor->mutex);

	if (executor->status != JOB_EXECUTOR_STATUS_RUNNING) {
		pthread_mutex_unlock(&executor->mutex);
		return false;
	}

	if (!job_queue_enqueue(&executor->job_queue, job)) {
		pthread_mutex_unlock(&executor->mutex);
		return false;
	}

	if (job_queue_is_empty(&executor->job_queue)) {
		pthread_cond_signal(&executor->update);
	}

	pthread_mutex_unlock(&executor->mutex);

	return true;
}
bool job_executor_should_stop(JobExecutor *executor) {
	assert(executor != NULL);

	pthread_mutex_lock(&executor->mutex);
	bool should_stop = executor->status == JOB_EXECUTOR_STATUS_STOPPING;
	pthread_mutex_unlock(&executor->mutex);

	return should_stop;
}
