#ifndef _REQUEST_H
#define _REQUEST_H

#include <stdio.h>
#include "libcx-base/debug.h"
#include "libcx-umtp/message.h"

typedef enum request_status_t
{
	REQ_STARTED,
	REQ_FINISHED,
	REQ_ERROR
} RequestStatus;

typedef struct request_t Request;

#define MESSAGE_INITIAL_BUFFER_SIZE 1024

struct request_t
{
	/* some kind of id  */
	unsigned long id;
	int error;
	RequestStatus status;
	struct timeval started_at;
	struct timeval finished_at;
	int priority; /* scheduling priority */

	// FIXME payload should be dynamic
	Message *message;
};

Worker*
Worker_new(unsigned long id);

void
Worker_free(Worker *worker);

int
Worker_start(Worker *worker);

void
Worker_stop(Worker *worker);

#endif
