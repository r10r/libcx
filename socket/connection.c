#include "connection.h"

Connection*
Connection_new(int fd)
{
	Connection* connection = cx_alloc(sizeof(Connection));

	Connection_init(connection, fd);
	return connection;
}

static void
free_response(void* data)
{
	Response_free((Response*)data);
}

void
Connection_init(Connection* connection, int fd)
{
	connection->fd = fd;
	connection->response_queue = Queue_new();
	((List*)connection->response_queue)->f_node_data_free = free_response;
}

void
Connection_free(Connection* conn)
{
	Queue_free(conn->response_queue);
	cx_free(conn);
}
