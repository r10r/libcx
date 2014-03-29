#include "request.h"

Request*
Request_new(unsigned long id)
{
	Request *request = malloc(sizeof(Request));

	request->id = id;
	request->status = REQUEST_STARTED;
	request->started_at = malloc(sizeof(struct timeval));
	request->finished_at = malloc(sizeof(struct timeval));

	int ret = gettimeofday(request->started_at, NULL);
	XASSERT(ret == 0, "gettimeofday should return 0");
	request->priority = 0; // currently unused
	request->userdata = NULL;
	return request;
}

void
Request_free(Request *request)
{
	free(request->started_at);
	free(request->finished_at);
	free(request);
}
