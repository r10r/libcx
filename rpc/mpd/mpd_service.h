#ifndef _CX_MPD_SERVICE_H
#define _CX_MPD_SERVICE_H

#include "rpc/jsrpc.h"

#include <mpd/client.h>
#include <mpd/status.h>
#include <mpd/song.h>
#include <mpd/entity.h>
#include <mpd/search.h>
#include <mpd/tag.h>
#include <mpd/idle.h>

#include <mpd/connection.h>

/* namespace  declaration */
#define RPC_NAME MusicPlayerDaemon_

/* export each function + method definition */
RPC_export_without_params(play)
RPC_export_without_params(pause)

RPC_set_param_string(send_message, 0, channel, 0)
RPC_set_param_string(send_message, 1, message, 0)
RPC_param_list(send_message)
{
	&RPC_param(send_message, channel),
	&RPC_param(send_message, message)
};
RPC_export(send_message)

RPC_single_string_param(add, 0, uri, 0)
RPC_export(add)

RPC_export_without_params(next)
RPC_export_without_params(status)
RPC_export_without_params(playlists)

RPC_single_string_param(playlist, 0, playlist_name, 0)
RPC_export(playlist)

/* export methods */
#define MusicPlayerDaemon_methods \
	RPC_public_name(play), \
	RPC_public_name(pause), \
	RPC_public_name(send_message), \
	RPC_public_name(add), \
	RPC_public_name(next), \
	RPC_public_name(status), \
	RPC_public_name(playlists), \
	RPC_public_name(playlist)


/* error definitions key,value pairs */
static const char* const api_errlist[] = {
	"Unknown error code"
	"Out of memory",
};

static int api_nerr = 2;

/* custom RPC errors -32000 to -32099 */
enum api_error
{
	ERR_OOM = -32000,
};

#define api_strerror(code) \
	(((code + 32000) < api_nerr) ? api_errlist[code + 32000] : api_errlist[0])

#endif
