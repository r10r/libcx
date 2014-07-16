#ifndef _CX_EV_H
#define _CX_EV_H

#define EV_COMPAT3 0    /* disable pre 4.0 compatibility (enables ev_loop typedef) */
#include <ev.h>

#if (!defined (__linux__) && defined(__unix__)) || (defined(__APPLE__) && defined(__MACH__))
/* most stable on OSX 10.9.x, select fails when #fds > 1024 */
#define EVBACKEND EVBACKEND_KQUEUE

/*
 * Timeouts are not properly scheduled on OSX without updating the clock.
 * The effect is that timeouts get negative and their callbacks are executed immediately,
 * which is definitely a bad thing to do.
 * It is required before calling ev_timer_agin / ev_timer_set to update the loop time.
 */
#define ev_timer_restart(loop, watcher, millis) \
	ev_now_update(loop); \
	state->timer_watcher.repeat = (float)(millis / 1000.0); \
	ev_timer_again(loop, watcher)

#else
#define EVBACKEND EVBACKEND_SELECT

#define ev_timer_restart(loop, watcher, millis) \
	state->timer_watcher.repeat = (float)(millis / 1000.0); \
	ev_timer_again(loop, watcher)

#endif

#endif
