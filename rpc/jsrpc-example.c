#include "jsrpc.h"
#include "hello_service.h"

int
main()
{
	RPC_Method hello_world_methods[] = { RPC_methods(HelloWorld), RPC_null };

	printf("Registered RPC methods:\n");
	int i;
	for (i = 0; hello_world_methods[i].name != NULL; i++)
		RPC_Method_log(&hello_world_methods[i]);


	/* we have the whole config file in memory.  let's parse it ... */
	char request[1024];
	RPC_Request req;

	snprintf(request, sizeof(request), JSONRPC_REQUEST, "66", "hello", "name", "World");
	dispatch_request(&req, hello_world_methods, request);

	snprintf(request, sizeof(request), JSONRPC_REQUEST_POS, "\"foobar\"", "lonely", "World");
	dispatch_request(&req, hello_world_methods, request);

	snprintf(request, sizeof(request), JSONRPC_NOTIFICATION, "hello", "name", "World");
	dispatch_request(&req, hello_world_methods, request);
}
