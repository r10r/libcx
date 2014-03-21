#include "libcx-base/test.h"
#include "libcx-base/base.h"
#include "unix_server.h"

/* worker calls the request handler for queued requests */
static Request*
my_request_handler(Request *request)
{
	// cast request to extended request

	// process the request

	// start request specific io watchers ?

	return request;
}

int
main(int argc, char** argv)

{
	Server *server = (Server*)UnixServer_new("/tmp/echo.sock");

	server->worker_count = 4;
	server->f_request_handler = my_request_handler;
	int ret = Server_start(server);


	Server_stop(server);
}
