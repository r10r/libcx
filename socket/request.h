#ifndef _CX_REQUEST_H
#define _CX_REQUEST_H

#include <stdio.h>
#include <sys/time.h> // gettimeofday

#include <libcx/base/base.h>
#include <libcx/base/uid.h>

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
	char id[CX_UID_LENGTH];
	RequestStatus status;
	struct timeval* started_at;
	struct timeval* finished_at;
	int priority; /* scheduling priority (FIXME currently unused) */

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
	XFDBG("Request[%s] duration:%f msec", request->id, timeval_diff(request->started_at, request->finished_at))

#define CCXLOG(_req, _msg) \
	XFLOG("request[%s] - " _msg, ((Request*)_req)->id)

#define CCXFLOG(_req, _fmt, ...) \
	XFLOG("request[%s] - " _fmt, ((Request*)_req)->id, __VA_ARGS__)

#define CCXDBG(_req, _msg) \
	XFDBG("request[%s] - " _msg, ((Request*)_req)->id)

#define CCXFDBG(_req, _fmt, ...) \
	XFDBG("request[%s] - " _fmt, ((Request*)_req)->id, __VA_ARGS__)

#define CCXERR(_req, _msg) \
	XFERR("request[%s] - " _msg, ((Request*)_req)->id)

#define CCXFERR(_req, _fmt, ...) \
	XFERR("request[%s] - " _fmt, ((Request*)_req)->id, __VA_ARGS__)

#endif
