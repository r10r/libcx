#include "jsrpc.h"
#include "hello_service.h"

int
main()
{
	RPC_Method hello_world_methods[] = { RPC_methods(HelloWorld), RPC_Method_none };

	printf("--- Registered RPC methods:\n");
	int i;
	for (i = 0; hello_world_methods[i].name != NULL; i++)
		RPC_Method_log(&hello_world_methods[i]);

	RPC_Request* request = RPC_Request_new();

	printf("--- Sending requests\n");

	StringBuffer_printf(&request->request_buffer, JSONRPC_REQUEST, "66", "hello", "myparam", "World");
	RPC_Request_dispatch(request, hello_world_methods);

	StringBuffer_printf(&request->request_buffer, JSONRPC_REQUEST_POS, "\"foobar\"", "lonely", "World");
	RPC_Request_dispatch(request, hello_world_methods);

	StringBuffer_printf(&request->request_buffer, JSONRPC_NOTIFICATION, "hello", "myparam", "World");
	RPC_Request_dispatch(request, hello_world_methods);

	RPC_Request_free(request);
}
