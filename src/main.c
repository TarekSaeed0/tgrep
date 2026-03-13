#include <job_executor.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define WORKER_COUNT (10)
#define JOB_COUNT (100)
#define ARRAY_SIZE (1000000000)
#define CHUNK_SIZE (ARRAY_SIZE / JOB_COUNT)

int *array;

int sum[JOB_COUNT] = { 0 };

void work(JobExecutor *executor, void *data) {
	(void)executor;

	size_t i = (size_t)data;

	printf("Started job %ld\n", i);

	for (size_t j = 0; j < CHUNK_SIZE; j++) {
		sum[i] += array[i * CHUNK_SIZE + j];
	}

	printf("Finished job %ld\n", i);
}

int main(void) {
	array = malloc(ARRAY_SIZE * sizeof(*array));
	for (size_t i = 0; i < ARRAY_SIZE; i++) {
		array[i] = rand() % ARRAY_SIZE; // NOLINT
	}

	struct timespec start, end;
	clock_gettime(CLOCK_MONOTONIC, &start);

	JobExecutor executor;
	job_executor_new(&executor, WORKER_COUNT);

	for (size_t i = 0; i < JOB_COUNT; i++) {
		Job job = { .function = work, .data = (void *)(uintptr_t)i };
		job_executor_submit(&executor, job);
	}

	job_executor_drain(&executor);
	job_executor_drop(&executor);

	int total_sum = 0;
	for (size_t i = 0; i < JOB_COUNT; i++) {
		total_sum += sum[i];
	}

	printf("Total sum: %d\n", total_sum);

	clock_gettime(CLOCK_MONOTONIC, &end);

	printf("Total execution time: %f seconds\n", (float)(end.tv_sec - start.tv_sec) + (float)(end.tv_nsec - start.tv_nsec) / 1e9);
}
