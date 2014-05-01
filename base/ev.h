#ifndef _CX_EV_H
#define _CX_EV_H

#define EV_COMPAT3 0    /* disable pre 4.0 compatibility (enables ev_loop typedef) */
#include <ev.h>

#if (!defined (__linux__) && defined(__unix__)) || (defined(__APPLE__) && defined(__MACH__))
/* most stable on OSX 10.9.x, select fails when #fds > 1024 */
#define EVBACKEND EVBACKEND_KQUEUE
#else
#define EVBACKEND EVBACKEND_SELECT
#endif

#endif
