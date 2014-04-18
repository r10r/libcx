#include "mpd_service.h"

static void
play(const char* track)
{
	printf("Play %s\n", track);
}

static void
pause(const char* track)
{
	printf("Pause\n");
}

/* service methods */

RPC(params, play)
{
	{ "track", get_param_value_string, 0 }
};
RPC(method, play)
{
	play((char*)request->params[0]);
}

RPC(method, pause)
{
	pause((char*)request->params[0]);
}

RPC(export, play);
RPC(export_without_params, pause);
