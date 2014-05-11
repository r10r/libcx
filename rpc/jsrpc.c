#include "jsrpc.h"

static void
RPC_Pipeline_process_request(RPC_RequestList* request_list, int nrequest, RPC_Method methods[])
{
	RPC_Request* request = request_list->requests + nrequest;

	/* lookup service method */
	RPC_Method* rpc_method = RPC_Request_lookup_method(request, methods);

	if (!rpc_method)
	{
		/* method not found */
		request->error = jsrpc_ERROR_METHOD_NOT_FOUND;
		StringBuffer_cat(request_list->result_buffer, "Method not found");
	}
	else
		rpc_method->method(request, request_list->result_buffer);

	/* append result to response buffer if request is not a notification */
	if (request->id)
	{
		if (nrequest > 0)
			StringBuffer_ncat(request_list->response_buffer, ",", 1);

		if (request->error)
		{
			StringBuffer_aprintf(request_list->response_buffer,
					     JSONRPC_RESPONSE_ERROR, request->id, request->error,
					     StringBuffer_value(request_list->result_buffer));
		}
		else
		{
			if (StringBuffer_used(request_list->result_buffer) > 0)
			{
				StringBuffer_aprintf(request_list->response_buffer,
						     JSONRPC_RESPONSE_SIMPLE, request->id,
						     StringBuffer_value(request_list->result_buffer));
			}
			else
			{
				// no result
				StringBuffer_aprintf(request_list->response_buffer,
						     JSONRPC_RESPONSE_NULL, request->id);
			}
		}
	}

	/* clear result buffer after each request */
	StringBuffer_clear(request_list->result_buffer);
}

void
RPC_RequestList_process(RPC_RequestList* request_list, RPC_Method methods[])
{
	printf("Dispatching request: \n<<<\n%s\n>>>\n", StringBuffer_value(request_list->request_buffer));

	RPC_Request_deserialize(request_list);

	/* check error */
	if (request_list->nrequests == -1)
	{
		StringBuffer_printf(request_list->response_buffer,
				    JSONRPC_RESPONSE_ERROR, JSONRPC_NULL,
				    jsrpc_ERROR_MALFORMED_JSON, StringBuffer_value(request_list->result_buffer));
	}
	else if (request_list->nrequests == 0)
	{
		StringBuffer_printf(request_list->response_buffer,
				    JSONRPC_RESPONSE_ERROR, JSONRPC_NULL,
				    jsrpc_ERROR_INVALID_REQUEST, StringBuffer_value(request_list->result_buffer));
	}
	else if (request_list->nrequests == 1)
		RPC_Pipeline_process_request(request_list, 0, methods);
	else if (request_list->nrequests > 1)
	{
		/* begin batch response */
		StringBuffer_ncat(request_list->response_buffer, "[", 1);

		int i;
		for (i = 0; i < request_list->nrequests; i++)
			RPC_Pipeline_process_request(request_list, i, methods);

		/* end batch response */
		StringBuffer_ncat(request_list->response_buffer, "]", 1);
	}

	/* return response buffer */
	printf("Response: \n<<<\n%s\n>>>\n", StringBuffer_value(request_list->response_buffer));
}
