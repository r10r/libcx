#ifndef _CX_REQUEST_H
#define _CX_REQUEST_H

#include <stdio.h>
#include <sys/time.h> // gettimeofday

#include "base/base.h"

typedef enum cx_request_status_t
{
	REQUEST_STARTED,
	REQUEST_FINISHED,
	REQUEST_ERROR
} RequestStatus;

typedef struct cx_request_t Request;
typedef size_t F_GetPayload (Request* request, const char** payload_ptr);
#define MESSAGE_INITIAL_BUFFER_SIZE 1024

struct cx_request_t
{
	/* some kind of id  */
	int error;
	const char* id;
	RequestStatus status;
	struct timeval* started_at;
	struct timeval* finished_at;
	int priority; /* scheduling priority */

	F_GetPayload* f_get_payload;

	void* data;
	void* userdata;
};

void
Request_init(Request* request);

Request*
Request_new(void* data);

void
Request_free_members(Request* request);

void
Request_free(Request* request);

void
Request_stop(Request* request);

void
Request_set_data(Request* request, void* data);

void*
Request_get_data(Request* request);

void
Request_set_userdata(Request* request, void* userdata);

void*
Request_get_userdata(Request* request);

#define Request_log(request) \
	XFDBG("Request[%p] duration:%f msec", (void*)request, timeval_diff(request->started_at, request->finished_at))

#endif
