#include "connection.h"

static void
free_response(void* data)
{
	Response_free((Response*)data);
}

static void
connection_start_cb(Connection* conn)
{
	Connection_callback(conn, on_start);
}

Connection*
Connection_new(ConnectionCallbacks* callbacks)
{
	Connection* conn = cx_alloc(sizeof(Connection));

	conn->callbacks = callbacks;
	conn->response_queue = Queue_new();
	((List*)conn->response_queue)->f_node_data_free = free_response;

	/* default start callback */
	conn->f_start = connection_start_cb;

	return conn;
}

void
Connection_free(Connection* conn)
{
	if (conn->f_free_state)
		conn->f_free_state(conn->state);

	Queue_free(conn->response_queue);
	cx_free(conn);
}

void*
Connection_get_data(Connection* conn)
{
	return conn->data;
}

void
Connection_set_data(Connection* conn, void* data)
{
	conn->data = data;
}

void*
Connection_get_userdata(Connection* conn)
{
	return conn->userdata;
}

void
Connection_set_userdata(Connection* conn, void* userdata)
{
	conn->userdata = userdata;
}
