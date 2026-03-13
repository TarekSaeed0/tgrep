#ifndef JOB_H_INCLUDED
#define JOB_H_INCLUDED

struct JobExecutor;
typedef struct Job {
	void (*function)(struct JobExecutor *executor, void *data);
	void *data;
} Job;

#endif // JOB_H_INCLUDED
