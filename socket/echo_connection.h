#ifndef _CX_ECHO_WORKER_H
#define _CX_ECHO_WORKER_H

#include "connection_worker.h"

#define CONNECTION_BUFFER_LENGTH 1024

Connection*
EchoConnection_new(ConnectionCallbacks* callbacks);

#endif
