#include <job_executor.h>

#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MAXIMUM_JOB_DURATION (5 * 1000 * 1000)

atomic_uint total_duration = 0;

void work(void *data) {
	long i = (long)data;

	printf("Started job %ld\n", i);

	int random            = rand(); // NOLINT
	unsigned int duration = (unsigned int)((double)random / RAND_MAX * MAXIMUM_JOB_DURATION);
	usleep(duration);

	printf("Finished job %ld\n", i);

	atomic_fetch_add(&total_duration, duration);
}

int main(void) {
	srand((unsigned)time(NULL)); // NOLINT

	JobExecutor executor;
	job_executor_new(&executor, 5);

	job_executor_start(&executor);

	for (int i = 0; i < 10; i++) {
		Job job = { .function = work, .data = (void *)(uintptr_t)i };
		job_executor_submit(&executor, job);
	}

	job_executor_wait(&executor);
}
