#include "request.h"

Request*
Request_new(unsigned long id)
{
	Request *request = malloc(sizeof(Request));
	request->id = id;
	request->status = REQ_STARTED;
	int ret = gettimeofday(&request->started_at);
	XASSERT(ret == 0, "gettimeofday should return 0");
	request->finished_at = NULL;
	request->priority = 0; // currently unused
	request->message = Message_new(MESSAGE_INITIAL_BUFFER_SIZE);
	return request;
}

void
Request_free(Request *request)
{
	Message_free(request->message);
	free(request);
}
