#include <job_executor.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MAXIMUM_JOB_DURATION (5 * 1000 * 1000)

void work(void *data) {
	long i = (long)data;

	printf("Started job %ld\n", i);

	int random            = rand(); // NOLINT
	unsigned int duration = (unsigned int)((double)random / RAND_MAX * MAXIMUM_JOB_DURATION);
	usleep(duration);

	printf("Finished job %ld\n", i);
}

int main(void) {
	srand((unsigned)time(NULL)); // NOLINT

	JobExecutor executor;
	job_executor_new(&executor, 5);

	for (int i = 0; i < 100; i++) {
		Job job = { .function = work, .data = (void *)(uintptr_t)i };
		job_executor_submit(&executor, job);
	}

	job_executor_start(&executor);
}
