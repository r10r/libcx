#include "mpd_service.h"

static struct mpd_connection* mpd_connection = NULL;

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
	if (mpd_connection_get_error(mpd_connection) != MPD_ERROR_SUCCESS)
	{
		jsrpc_write_error(jsrpc_ERROR_INTERNAL, mpd_connection_get_error_message(*conn));
		mpd_connection_clear_error(mpd_connection);
		return false;
	}
	return true;
}

RPC(method, play)
{
	if (connect(request, &mpd_connection) == 1)
	{
		bool success = mpd_run_play(mpd_connection);

		if (mpd_response_check_success(request, &mpd_connection))
			jsrpc_write_simple_response(JSONRPC_RESPONSE_BOOLEAN(success));
	}
}

RPC(method, pause)
{
	if (connect(request, &mpd_connection) == 1)
	{
		bool success = mpd_run_pause(mpd_connection, 1);

		if (mpd_response_check_success(request, &mpd_connection))
			jsrpc_write_simple_response(JSONRPC_RESPONSE_BOOLEAN(success));
	}
}

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
		bool success = mpd_run_next(mpd_connection);

		if (mpd_response_check_success(request, &mpd_connection))
			jsrpc_write_simple_response(JSONRPC_RESPONSE_BOOLEAN(success));
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
print_song_json(RPC_Request* request, struct mpd_song* song)
{
	jsrpc_write_append(SONG_FORMAT,
			   mpd_song_get_tag(song, MPD_TAG_TITLE, 0),
			   mpd_song_get_tag(song, MPD_TAG_ALBUM, 0),
			   mpd_song_get_tag(song, MPD_TAG_ARTIST, 0),
			   mpd_song_get_duration(song),
			   mpd_song_get_uri(song));
}

RPC(method, status)
{
	if (connect(request, &mpd_connection) == 1)
	{
		struct mpd_status* status = mpd_run_status(mpd_connection);
		if (mpd_response_check_success(request, &mpd_connection))
		{
			jsrpc_write_response(JSONRPC_RESPONSE, JSONRPC_RESULT_OBJECT_START);
			jsrpc_write_append(STATUS_FORMAT,
					   mpd_status_get_song_id(status),
					   mpd_status_get_volume(status),
					   mpd_status_get_random(status));
			mpd_status_free(status);
		}

		struct mpd_song* song = mpd_run_current_song(mpd_connection);
		if (mpd_response_check_success(request, &mpd_connection) && song)
		{
			jsrpc_write_append_simple(",");
			print_song_json(request, song);
			mpd_song_free(song);
		}

		jsrpc_write_append_simple(JSONRPC_RESULT_OBJECT_END);
	}
}

RPC(method, playlists)
{
	if (connect(request, &mpd_connection) == 1)
	{
		mpd_send_list_playlists(mpd_connection);

		if (mpd_response_check_success(request, &mpd_connection))
		{
			struct mpd_playlist* playlist;

			while ((playlist = mpd_recv_playlist(mpd_connection)) != NULL)
			{
				printf("playlist: %s\n", mpd_playlist_get_path(playlist));
				mpd_playlist_free(playlist);
			}

			// check if mpd_recv_playlist returned NULL because of an error
			if (mpd_response_check_success(request, &mpd_connection))
				// send response
				printf("Success\n");
		}
	}
}

RPC(method, playlist)
{
	if (connect(request, &mpd_connection) == 1)
	{
		mpd_send_list_playlist_meta(mpd_connection, RPC(get_param, playlist, playlist_name));

		if (mpd_response_check_success(request, &mpd_connection))
		{
			struct mpd_song* song;

			// FIXME decouple serialization format from RPC method ?
			// --> no just write a new service implementation ;) (really!)
			jsrpc_write_response(JSONRPC_RESPONSE, JSONRPC_RESULT_ARRAY_START);

			// TODO check previous character in buffer, determine whether
			// a delimiter is needed or not ? {[: --> no comma, }]\" --> comma

			int count = 0;
			while ((song = mpd_recv_song(mpd_connection)) != NULL)
			{
				jsrpc_write_append_simple((count == 0) ? "{" : ",{");
				print_song_json(request, song);
				jsrpc_write_append_simple("}");
				mpd_song_free(song);
				count++;
			}

			jsrpc_write_append_simple(JSONRPC_RESULT_ARRAY_END);

			// check if mpd_recv_playlist returned NULL because of an error
			mpd_response_check_success(request, &mpd_connection);
		}
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
