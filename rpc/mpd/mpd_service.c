#include "mpd_service.h"

static struct mpd_connection* mpd_connection = NULL;

#define IS_REQUEST(request) \
	(request->id != NULL)

#define jsrpc_write_error(code, message) \
	if (IS_REQUEST(request)) StringBuffer_printf(&request->response_buffer, JSONRPC_ERROR, (request)->id, code, message)

#define jsrpc_write_simple_response(format) \
	if (IS_REQUEST(request)) StringBuffer_printf(&request->response_buffer, format, (request)->id)

static int
connect(RPC_Request* request, struct mpd_connection** conn)
{
	if (*conn == NULL)
	{
		*conn = mpd_connection_new(NULL, 0, 30000);
		if (*conn == NULL)
		{
			fprintf(stderr, "%s\n", "Out of memory"); // TODO use debug macro
			jsrpc_write_error(ERR_OOM, api_strerror(ERR_OOM));
			return -1;
		}

		if (mpd_connection_get_error(*conn) != MPD_ERROR_SUCCESS)
		{
			// TODO use debug macro
			fprintf(stderr, "Failed to create connection: %s\n", mpd_connection_get_error_message(*conn));
			jsrpc_write_error(jsrpc_ERROR_INTERNAL, mpd_connection_get_error_message(*conn));
			mpd_connection_free(*conn);
			*conn = NULL;
			return -1;
		}
	}
	return 1;
}

/* true if command was successful, false if error occured */
static bool
mpd_response_check_success(RPC_Request* request, struct mpd_connection** conn)
{
	bool success = mpd_response_finish(mpd_connection);

	// FIXME check response can be successful but connection has an error ?

	if (!success)
	{
		if (mpd_connection_get_error(mpd_connection) != MPD_ERROR_SUCCESS)
		{
			jsrpc_write_error(jsrpc_ERROR_INTERNAL, mpd_connection_get_error_message(*conn));
			mpd_connection_clear_error(mpd_connection);
		}
		else
			printf("Unsuccessful response but no error\n");
	}
	return success;
}

RPC(method, play)
{
	if (connect(request, &mpd_connection) == 1)
	{
		bool success = mpd_send_play(mpd_connection);

		if (mpd_response_check_success(request, &mpd_connection))
			jsrpc_write_simple_response(JSONRPC_RESPONSE_BOOLEAN(success));
	}
}

RPC(method, pause)
{
	if (connect(request, &mpd_connection) == 1)
	{
		bool success = mpd_send_pause(mpd_connection, 1);

		if (mpd_response_check_success(request, &mpd_connection))
			jsrpc_write_simple_response(JSONRPC_RESPONSE_BOOLEAN(success));
	}
}

RPC(set_param_string, send_message, 0, channel, 0)
RPC(set_param_string, send_message, 1, message, 0)
RPC(param_list, send_message)
{
	&RPC(param, send_message, channel),
	&RPC(param, send_message, message)
};
RPC(method, send_message)
{
	if (connect(request, &mpd_connection) == 1)
	{
		bool success = mpd_run_send_message(mpd_connection,
						    RPC(get_param, send_message, channel),
						    RPC(get_param, send_message, message));

		if (mpd_response_check_success(request, &mpd_connection))
			jsrpc_write_simple_response(JSONRPC_RESPONSE_BOOLEAN(success));
	}
}

RPC(single_string_param, add, 0, uri, 0)
RPC(method, add)
{
	if (connect(request, &mpd_connection) == 1)
	{
		bool success = mpd_run_add(mpd_connection, RPC(get_param, add, uri));

		if (mpd_response_check_success(request, &mpd_connection))
			jsrpc_write_simple_response(JSONRPC_RESPONSE_BOOLEAN(success));
	}
}

RPC(method, next)
{
	if (connect(request, &mpd_connection) == 1)
	{
		bool success = mpd_send_next(mpd_connection);

		if (mpd_response_check_success(request, &mpd_connection))
			jsrpc_write_simple_response(JSONRPC_RESPONSE_BOOLEAN(success));
	}
}

RPC(export_without_params, play);
RPC(export_without_params, pause);
RPC(export, send_message);
RPC(export, add);
RPC(export_without_params, next);
