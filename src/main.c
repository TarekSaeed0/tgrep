#define _XOPEN_SOURCE 700 // NOLINT
#include <ftw.h>
#include <job_executor.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define TEXT_STYLE_MATCH "\033[1;31m"
#define TEXT_STYLE_RESET "\033[0m"

#define MAXIMUM_OPEN_DIRECTORIES (10)

#define WORKER_COUNT (10)

typedef struct SearchLocation {
	const char *file_path;
	size_t line_number;
} SearchLocation;

const char *pattern;
size_t pattern_length;

JobExecutor executor;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void print_location(const SearchLocation *location) {
	printf("%s:%zu: ", location->file_path, location->line_number);
}

void search_line(const SearchLocation *location, const char *line) {
	const char *remaining = line;
	const char *match     = strstr(remaining, pattern);
	if (match != NULL) {
		pthread_mutex_lock(&mutex);
		print_location(location);
		if (pattern_length != 0 && isatty(STDOUT_FILENO)) {
			do {
				printf(
				    "%.*s" TEXT_STYLE_MATCH "%s" TEXT_STYLE_RESET,
				    (int)(match - remaining),
				    remaining,
				    pattern
				);
				remaining = match + pattern_length;
				match     = strstr(remaining, pattern);
			} while (match != NULL);
			printf("%s", remaining);
		} else {
			printf("%s", line);
		}
		pthread_mutex_unlock(&mutex);
	}
}

int search_file(const char *file_path) {
	FILE *file = fopen(file_path, "r");
	if (file == NULL) {
		perror("fopen");
		return -1;
	}

	SearchLocation location = {
		.file_path   = file_path,
		.line_number = 1,
	};

	char *line           = NULL;
	size_t line_capacity = 0;
	while (getline(&line, &line_capacity, file) != -1) {
		search_line(&location, line);
		location.line_number++;
	}

	free(line);

	(void)fclose(file);

	return 0;
}

void work(JobExecutor *_, void *data) {
	(void)_;
	char *file_path = data;

	search_file(file_path);

	free(file_path);
}

int search_directory_entry(const char *file_path, const struct stat *stat_buffer, int type_flag, struct FTW *ftw_buffer) {
	(void)stat_buffer, (void)ftw_buffer;

	bool is_regular_file = false;
	if (type_flag == FTW_F) {
		is_regular_file = true;
	} else if (type_flag == FTW_SL) {
		struct stat file_stat;
		if (stat(file_path, &file_stat) == 0 && S_ISREG(file_stat.st_mode)) {
			is_regular_file = true;
		}
	}

	if (is_regular_file) {
		Job job = {
			.function = work,
			.data     = strdup(file_path),
		};

		job_executor_submit(&executor, job);
	}

	return 0;
}
int search_directory(const char *directory_path) {
	if (nftw(directory_path, search_directory_entry, MAXIMUM_OPEN_DIRECTORIES, FTW_PHYS) != 0) {
		return -1;
	}

	return 0;
}

int main(int argc, char *argv[]) {
	if (argc < 2 || 3 < argc) {
		(void)fprintf(stderr, "Usage: tgrep PATTERN [PATH]\n");
		return EXIT_FAILURE;
	}

	pattern          = argv[1];
	pattern_length   = strlen(pattern);

	const char *path = ".";
	if (argc >= 3) {
		path = argv[2];
	}

	struct stat stat_buffer;
	if (stat(path, &stat_buffer) != 0) {
		perror("stat");
		return EXIT_FAILURE;
	}

	bool is_directory = S_ISDIR(stat_buffer.st_mode);

	if (!is_directory) {
		search_file(path);
	} else {
		struct timespec start, end;
		clock_gettime(CLOCK_MONOTONIC, &start);

		job_executor_new(&executor, WORKER_COUNT);

		search_directory(path);

		job_executor_drain(&executor);
		job_executor_drop(&executor);

		clock_gettime(CLOCK_MONOTONIC, &end);

		printf("Total execution time: %f seconds\n", (float)(end.tv_sec - start.tv_sec) + (float)(end.tv_nsec - start.tv_nsec) / 1e9);
	}

	return EXIT_SUCCESS;
}
