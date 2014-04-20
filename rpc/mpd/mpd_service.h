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
#define RPC(action, cmd) RPC_ ## action(MusicPlayerDaemon, cmd)

/* export each function + method definition */
RPC(public, play)
RPC(public, pause)
RPC(public, send_message)

#define MusicPlayerDaemon_methods \
	RPC(public_name, play), \
	RPC(public_name, pause), \
	RPC(public_name, send_message)

#endif
