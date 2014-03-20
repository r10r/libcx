#ifndef _REQUEST_H
#define _REQUEST_H

#include <stdio.h>
#include <sys/time.h> // gettimeofday

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
	struct timeval *started_at;
	struct timeval *finished_at;
	int priority; /* scheduling priority */

	// FIXME payload should be dynamic
	Message *message;
};

Request*
Request_new(unsigned long id);

void
Request_free(Request *request);

#endif
