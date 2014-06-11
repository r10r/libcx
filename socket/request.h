#ifndef _CX_REQUEST_H
#define _CX_REQUEST_H

#include <stdio.h>
#include <sys/time.h> // gettimeofday

#include "../umtp/message.h"

typedef enum cx_request_status_t
{
	REQUEST_STARTED,
	REQUEST_FINISHED,
	REQUEST_ERROR
} RequestStatus;

typedef struct cx_request_t Request;

#define MESSAGE_INITIAL_BUFFER_SIZE 1024

struct cx_request_t
{
	/* some kind of id  */
	const char* id;
	int error;
	RequestStatus status;
	struct timeval* started_at;
	struct timeval* finished_at;
	int priority; /* scheduling priority */

	void* userdata;
};

void
Request_init(Request* request);

Request*
Request_new(const char* id);

void
Request_free_members(Request* request);

void
Request_free(Request* request);

void
Request_stop(Request* request);

#define Request_log(request) \
	XFDBG("Request[%p] duration:%f msec", (void*)request, timeval_diff(request->started_at, request->finished_at))

#endif
