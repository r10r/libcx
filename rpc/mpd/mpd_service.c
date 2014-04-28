#include "mpd_service.h"

static struct mpd_connection* mpd_connection = NULL;

static int
connect(struct mpd_connection** conn, RPC_Request* request, StringBuffer* result_buffer)
{
	if (*conn == NULL)
	{
		*conn = mpd_connection_new(NULL, 0, 30000);
		if (*conn == NULL)
		{
			request->error = ERR_OOM;
			StringBuffer_cat(result_buffer, api_strerror(ERR_OOM));
			return -1;
		}

		if (mpd_connection_get_error(*conn) != MPD_ERROR_SUCCESS)
		{
			request->error = jsrpc_ERROR_INTERNAL;
			StringBuffer_printf(result_buffer, "MPD connection error:", mpd_connection_get_error_message(*conn));
			mpd_connection_free(*conn);
			*conn = NULL;
			return -1;
		}
	}
	return 1;
}

/* true if command was successful, false if error occured */
static bool
mpd_response_check_success(struct mpd_connection** conn, RPC_Request* request, StringBuffer* result_buffer)
{
	if (mpd_connection_get_error(mpd_connection) != MPD_ERROR_SUCCESS)
	{
		request->error = jsrpc_ERROR_INTERNAL;
		StringBuffer_printf(result_buffer, "MPD connection error:", mpd_connection_get_error_message(*conn));
		mpd_connection_clear_error(mpd_connection);
		return false;
	}
	return true;
}

RPC_method(play)
{
	if (connect(&mpd_connection, request, result_buffer) == 1)
	{
		bool success = mpd_run_play(mpd_connection);

		if (mpd_response_check_success(&mpd_connection, request, result_buffer))
			StringBuffer_cat(result_buffer, JSONRPC_BOOLEAN(success));
	}
}

RPC_method(pause)
{
	if (connect(&mpd_connection, request, result_buffer) == 1)
	{
		bool success = mpd_run_pause(mpd_connection, 1);

		if (mpd_response_check_success(&mpd_connection, request, result_buffer))
			StringBuffer_cat(result_buffer, JSONRPC_BOOLEAN(success));
	}
}

RPC_method(send_message)
{
	if (connect(&mpd_connection, request, result_buffer) == 1)
	{
		bool success = mpd_run_send_message(mpd_connection,
						    RPC_get_param(send_message, channel),
						    RPC_get_param(send_message, message));

		if (mpd_response_check_success(&mpd_connection, request, result_buffer))
			StringBuffer_cat(result_buffer, JSONRPC_BOOLEAN(success));
	}
}

RPC_method(add)
{
	if (connect(&mpd_connection, request, result_buffer) == 1)
	{
		bool success = mpd_run_add(mpd_connection, RPC_get_param(add, uri));

		if (mpd_response_check_success(&mpd_connection, request, result_buffer))
			StringBuffer_cat(result_buffer, JSONRPC_BOOLEAN(success));
	}
}

RPC_method(next)
{
	if (connect(&mpd_connection, request, result_buffer) == 1)
	{
		bool success = mpd_run_next(mpd_connection);

		if (mpd_response_check_success(&mpd_connection, request, result_buffer))
			StringBuffer_cat(result_buffer, JSONRPC_BOOLEAN(success));
	}
}

static const char* const STATUS_FORMAT =
	JSRPC_KEYPAIR("song_id", "%d")
	"," JSRPC_KEYPAIR("volume", "%d")
	"," JSRPC_KEYPAIR("random", "%d");

static const char* const SONG_FORMAT =
	JSRPC_KEYPAIR("track", JSONRPC_STRING)
	"," JSRPC_KEYPAIR("album", JSONRPC_STRING)
	"," JSRPC_KEYPAIR("artist", JSONRPC_STRING)
	"," JSRPC_KEYPAIR("duration", "%u")
	"," JSRPC_KEYPAIR("uri", JSONRPC_STRING);


static void
print_song_json(RPC_Request* request, StringBuffer* result_buffer, struct mpd_song* song)
{
	jsrpc_write_append(SONG_FORMAT,
			   mpd_song_get_tag(song, MPD_TAG_TITLE, 0),
			   mpd_song_get_tag(song, MPD_TAG_ALBUM, 0),
			   mpd_song_get_tag(song, MPD_TAG_ARTIST, 0),
			   mpd_song_get_duration(song),
			   mpd_song_get_uri(song));
}

RPC_method(status)
{
	if (connect(&mpd_connection, request, result_buffer) == 1)
	{
		struct mpd_status* status = mpd_run_status(mpd_connection);
		if (mpd_response_check_success(&mpd_connection, request, result_buffer))
		{
			jsrpc_write_append_simple("{");
			jsrpc_write_append(STATUS_FORMAT,
					   mpd_status_get_song_id(status),
					   mpd_status_get_volume(status),
					   mpd_status_get_random(status));
		}
		mpd_status_free(status);

		struct mpd_song* song = mpd_run_current_song(mpd_connection);
		if (mpd_response_check_success(&mpd_connection, request, result_buffer) && song)
		{
			jsrpc_write_append_simple(",");
			print_song_json(request, result_buffer, song);
		}
		mpd_song_free(song);

		jsrpc_write_append_simple("}");
	}
}

RPC_method(playlists)
{
	if (connect(&mpd_connection, request, result_buffer) == 1)
	{
		mpd_send_list_playlists(mpd_connection);

		if (mpd_response_check_success(&mpd_connection, request, result_buffer))
		{
			struct mpd_playlist* playlist = NULL;

			while ((playlist = mpd_recv_playlist(mpd_connection)) != NULL)
			{
				printf("playlist: %s\n", mpd_playlist_get_path(playlist));
				mpd_playlist_free(playlist);
			}

			// check if mpd_recv_playlist returned NULL because of an error
			if (mpd_response_check_success(&mpd_connection, request, result_buffer))
				// send response
				printf("Success\n");
		}
	}
}

RPC_method(playlist)
{
	if (connect(&mpd_connection, request, result_buffer) == 1)
	{
		mpd_send_list_playlist_meta(mpd_connection, RPC_get_param(playlist, playlist_name));

		if (mpd_response_check_success(&mpd_connection, request, result_buffer))
		{
			jsrpc_write_append_simple("[");
			struct mpd_song* song = NULL;
			int count = 0;
			while ((song = mpd_recv_song(mpd_connection)) != NULL)
			{
				jsrpc_write_append_simple((count == 0) ? "{" : ",{");
				print_song_json(request, result_buffer, song);
				jsrpc_write_append_simple("}");
				mpd_song_free(song);
				count++;
			}
			jsrpc_write_append_simple("]");
		}
		else
			// check if mpd_recv_playlist returned NULL because of an error
			mpd_response_check_success(&mpd_connection, request, result_buffer);
	}
}

/* save playlist */

/* load playlist */

/* modify playlist (delete/add/move tracks see playlist.h) */

/*
 * modify playlist on client only ? send back to server on save ?
 *
 * queue each operation, send as RPC batch on save ?
 */

/* convert playlist to itunes playlist ? */
