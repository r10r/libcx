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
	request->finished_at = NULL;
	request->priority = 0; // currently unused
	request->message = Message_new(MESSAGE_INITIAL_BUFFER_SIZE);
	return request;
}

void
Request_free(Request *request)
{
	free(request->started_at);
	free(request->finished_at);
	Message_free(request->message);
	free(request);
}
