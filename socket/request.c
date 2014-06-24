#include "request.h"

void
Request_init(Request* request)
{
	request->status = REQUEST_STARTED;
	request->started_at = cx_alloc(sizeof(struct timeval));
	request->finished_at = cx_alloc(sizeof(struct timeval));

	int ret = gettimeofday(request->started_at, NULL);
	XASSERT(ret == 0, "gettimeofday should return 0");
	request->priority = 0; // currently unused
	request->data = NULL;

	// TODO optionally generate request id (UUID) ?
//	request->id;
}

Request*
Request_new(void* data)
{
	Request* request = cx_alloc(sizeof(Request));

	Request_init(request);
	request->data = data;
	return request;
}

void
Request_free_members(Request* request)
{
	cx_free(request->started_at);
	cx_free(request->finished_at);
}

void
Request_free(Request* request)
{
	Request_free_members(request);
	cx_free(request);
}

void
Request_stop(Request* request)
{
	int ret = gettimeofday(request->finished_at, NULL);

	XASSERT(ret == 0, "gettimeofday should return 0");
}
