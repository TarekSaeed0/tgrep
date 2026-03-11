#include <task.h>

#include <assert.h>
#include <stdlib.h>

Task *task_new(void (*function)(void *), void *data) {
	assert(function != NULL);

	Task *task = malloc(sizeof(Task));
	if (task == NULL) {
		return NULL;
	}

	task->function = function;
	task->data     = data;
	task->next     = NULL;

	return task;
}
