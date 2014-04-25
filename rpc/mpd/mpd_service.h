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
#undef RPC
#define RPC(action, ...) RPC_ ## action(MusicPlayerDaemon, __VA_ARGS__)

/* export each function + method definition */
RPC(export_without_params, play)
RPC(export_without_params, pause)

RPC(set_param_string, send_message, 0, channel, 0)
RPC(set_param_string, send_message, 1, message, 0)
RPC(param_list, send_message)
{
	&RPC(param, send_message, channel),
	&RPC(param, send_message, message)
};
RPC(export, send_message)

RPC(single_string_param, add, 0, uri, 0)
RPC(export, add)

RPC(export_without_params, next)
RPC(export_without_params, status)
RPC(export_without_params, playlists)

RPC(single_string_param, playlist, 0, playlist_name, 0)
RPC(export, playlist)

/* export methods */
#define MusicPlayerDaemon_methods \
	RPC(public_name, play), \
	RPC(public_name, pause), \
	RPC(public_name, send_message), \
	RPC(public_name, add), \
	RPC(public_name, next), \
	RPC(public_name, status), \
	RPC(public_name, playlists), \
	RPC(public_name, playlist)


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
