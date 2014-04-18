#ifndef _CX_MPD_SERVICE_H
#define _CX_MPD_SERVICE_H

#include "rpc/jsrpc.h"

/* namespace  declaration */
#undef RPC
#define RPC(action, cmd) RPC_ ## action(MusicPlayerDaemon, cmd)

/* export each function + method definition */
RPC(public, play)
RPC(public, pause)

#define MusicPlayerDaemon_methods \
	RPC(public_name, play), \
	RPC(public_name, pause)

#endif
