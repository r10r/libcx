#include "connection.h"

static void
free_response(void* data)
{
	Response_free((Response*)data);
}

Connection*
Connection_new(ConnectionCallbacks* callbacks)
{
	Connection* conn = cx_alloc(sizeof(Connection));

	conn->callbacks = callbacks;
	conn->response_queue = Queue_new();
	((List*)conn->response_queue)->f_node_data_free = free_response;
	return conn;
}

void
Connection_free(Connection* conn)
{
	Queue_free(conn->response_queue);
	cx_free(conn);
}
