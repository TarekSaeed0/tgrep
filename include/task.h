#ifndef TASK_H_INCLUDED
#define TASK_H_INCLUDED

typedef struct Task {
	void (*function)(void *);
	void *data;
	struct Task *next;
} Task;

Task *task_new(void (*function)(void *), void *data);

#endif // TASK_H_INCLUDED
