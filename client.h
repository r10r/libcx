#ifndef _CX_CLIENT_H
#define _CX_CLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <assert.h>
#include <unistd.h> /* close */

#include "libcx-base/debug.h"

void
send_data(int fd, const char* data);

int
client_connect(const char* sock_path);

void
receive_response(int sock);

#endif
