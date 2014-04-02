#include "request.h"

void
Request_init(Request *request)
{
	request->status = REQUEST_STARTED;
	request->started_at = malloc(sizeof(struct timeval));
	request->finished_at = malloc(sizeof(struct timeval));

	int ret = gettimeofday(request->started_at, NULL);
	XASSERT(ret == 0, "gettimeofday should return 0");
	request->priority = 0; // currently unused
	request->userdata = NULL;
}

Request*
Request_new(unsigned long id)
{
	Request *request = malloc(sizeof(Request));

	request->id = id;

	Request_init(request);
	return request;
}

void
Request_free_members(Request *request)
{
	free(request->started_at);
	free(request->finished_at);
}

void
Request_free(Request *request)
{
	Request_free_members(request);
	free(request);
}
