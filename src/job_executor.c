#include <job_executor.h>

#include <assert.h>
#include <pthread.h>
#include <stdlib.h>

bool job_executor_new(JobExecutor *executor, size_t worker_count) {
	assert(executor != NULL);
	assert(worker_count != 0);

	executor->workers = malloc(worker_count * sizeof(*executor->workers));
	if (executor->workers == NULL) {
		return false;
	}

	executor->worker_count = worker_count;
	executor->job_queue    = job_queue_new();
	pthread_mutex_init(&executor->mutex, NULL);
	executor->stop = false;

	return true;
}
void *job_executor_worker(void *data) {
	JobExecutor *executor = data;
	assert(executor != NULL);

	while (true) {
		pthread_mutex_lock(&executor->mutex);

		while (job_queue_is_empty(&executor->job_queue) || executor->stop) {
			pthread_cond_wait(&executor->jobs_available, &executor->mutex);
		}

		if (executor->stop) {
			break;
		}

		Job job;
		job_queue_dequeue(&executor->job_queue, &job);

		pthread_mutex_unlock(&executor->mutex);

		job.function(job.data);
	}

	return NULL;
}
bool job_executor_start(JobExecutor *executor) {
	assert(executor != NULL);

	for (size_t i = 0; i < executor->worker_count; i++) {
		if (pthread_create(&executor->workers[i], NULL, job_executor_worker, executor) != 0) {
			job_executor_stop(executor);
			for (size_t j = 0; j < i; j++) {
				pthread_join(executor->workers[j], NULL);
			}
			return false;
		}
	}

	for (size_t i = 0; i < executor->worker_count; i++) {
		pthread_join(executor->workers[i], NULL);
	}

	return true;
}
void job_executor_stop(JobExecutor *executor) {
	assert(executor != NULL);

	pthread_mutex_lock(&executor->mutex);

	executor->stop = true;

	pthread_cond_broadcast(&executor->jobs_available);

	pthread_mutex_unlock(&executor->mutex);

	for (size_t i = 0; i < executor->worker_count; i++) {
		pthread_join(executor->workers[i], NULL);
	}
}
bool job_executor_submit(JobExecutor *executor, Job job) {
	assert(executor != NULL);

	pthread_mutex_lock(&executor->mutex);

	if (!job_queue_enqueue(&executor->job_queue, job)) {
		pthread_mutex_unlock(&executor->mutex);
		return false;
	}

	pthread_cond_broadcast(&executor->jobs_available);

	pthread_mutex_unlock(&executor->mutex);

	return true;
}
