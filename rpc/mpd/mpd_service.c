#include "mpd_service.h"

static struct mpd_connection* mpd_connection = NULL;

static int
connect(struct mpd_connection** conn)
{
	if (*conn == NULL)
	{
		*conn = mpd_connection_new(NULL, 0, 30000);
		if (*conn == NULL)
		{
			fprintf(stderr, "%s\n", "Out of memory");
			return -1;
		}

		if (mpd_connection_get_error(*conn) != MPD_ERROR_SUCCESS)
		{
			fprintf(stderr, "Failed to create connection: %s\n", mpd_connection_get_error_message(*conn));
			mpd_connection_free(*conn);
			*conn = NULL;
			return -1;
		}
	}
	return 1;
}

static void
mpd_clear_error()
{
	mpd_response_finish(mpd_connection);
	if (mpd_connection_get_error(mpd_connection) != MPD_ERROR_SUCCESS)
	{
		fprintf(stderr, "Connection error: %s\n", mpd_connection_get_error_message(mpd_connection));
		mpd_connection_clear_error(mpd_connection);
	}
}

RPC(method, play)
{
	if (connect(&mpd_connection) == 1)
	{
		bool playing = mpd_send_play(mpd_connection);
		printf("Play %d\n", playing);
		mpd_clear_error();
	}
}

RPC(method, pause)
{
	if (connect(&mpd_connection) == 1)
	{
		bool paused = mpd_send_pause(mpd_connection, 0);
		printf("Paused %d\n", paused);
		mpd_clear_error();
	}
}

RPC(params, send_message)
{
	{ "message", get_param_value_string, 0 }
};
RPC(method, send_message)
{
	if (connect(&mpd_connection) == 1)
	{
		bool paused = mpd_run_send_message(mpd_connection, "foo", (const char*)request->params[0]);
		printf("Message send %d\n", paused);
		mpd_clear_error();
	}
}

RPC(export_without_params, play);
RPC(export_without_params, pause);
RPC(export, send_message);
