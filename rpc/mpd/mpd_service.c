#include "mpd_service.h"

static struct mpd_connection* mpd_connection = NULL;

#define jsrpc_write_error(request, code, message) \
	StringBuffer_printf(&(request)->response_buffer, JSONRPC_ERROR, (request)->id, code, message);

static int
connect(RPC_Request* request, struct mpd_connection** conn)
{
	if (*conn == NULL)
	{
		*conn = mpd_connection_new(NULL, 0, 30000);
		if (*conn == NULL)
		{
			fprintf(stderr, "%s\n", "Out of memory"); // TODO use debug macro
			jsrpc_write_error(request, ERR_OOM, api_strerror(ERR_OOM));
			return -1;
		}

		if (mpd_connection_get_error(*conn) != MPD_ERROR_SUCCESS)
		{
			// TODO use debug macro
			fprintf(stderr, "Failed to create connection: %s\n", mpd_connection_get_error_message(*conn));
			jsrpc_write_error(request, jsrpc_ERROR_INTERNAL, mpd_connection_get_error_message(*conn));
			mpd_connection_free(*conn);
			*conn = NULL;
			return -1;
		}
	}
	return 1;
}

static void
mpd_clear_error(RPC_Request* request, struct mpd_connection** conn)
{
	mpd_response_finish(mpd_connection);
	if (mpd_connection_get_error(mpd_connection) != MPD_ERROR_SUCCESS)
	{
		jsrpc_write_error(request, jsrpc_ERROR_INTERNAL, mpd_connection_get_error_message(*conn));
		mpd_connection_clear_error(mpd_connection);
	}
}

RPC(method, play)
{
	if (connect(request, &mpd_connection) == 1)
	{
		bool playing = mpd_send_play(mpd_connection);
		printf("Play %d\n", playing);
		mpd_clear_error(request, &mpd_connection);
	}
}

RPC(method, pause)
{
	if (connect(request, &mpd_connection) == 1)
	{
		bool paused = mpd_send_pause(mpd_connection, 0);
		printf("Paused %d\n", paused);
		mpd_clear_error(request, &mpd_connection);
	}
}

RPC(single_string_param, send_message, 0, message, 0)
RPC(method, send_message)
{
	if (connect(request, &mpd_connection) == 1)
	{
		bool paused = mpd_run_send_message(mpd_connection, "foo", RPC(get_param, send_message, message));
		printf("Message send %d\n", paused);
		mpd_clear_error(request, &mpd_connection);
	}
}

RPC(export_without_params, play);
RPC(export_without_params, pause);
RPC(export, send_message);
