#ifndef _CX_ECHO_WORKER_H
#define _CX_ECHO_WORKER_H

#include "socket/connection_worker.h"

#define CONNECTION_BUFFER_LENGTH 1024

ConnectionWorker*
EchoWorker_new(ConnectionCallbacks* connection_callbacks);

#endif
