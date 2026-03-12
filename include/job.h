#ifndef JOB_H_INCLUDED
#define JOB_H_INCLUDED

typedef struct Job {
	void (*function)(void *);
	void *data;
} Job;

#endif // JOB_H_INCLUDED
