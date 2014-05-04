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

	RPC_Request* request = RPC_Request_new();

	StringBuffer_printf(&request->request_buffer, JSONRPC_REQUEST, "66", "hello", "name", "World");
	dispatch_request(request, hello_world_methods);

	StringBuffer_printf(&request->request_buffer, JSONRPC_REQUEST_POS, "\"foobar\"", "lonely", "World");
	dispatch_request(request, hello_world_methods);

	StringBuffer_printf(&request->request_buffer, JSONRPC_NOTIFICATION, "hello", "name", "World");
	dispatch_request(request, hello_world_methods);

	RPC_Request_free(request);
}
