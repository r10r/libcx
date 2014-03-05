#include "server.h"

int
main(void)
{
	// use the default event loop unless you have special needs
	ev_loop *loop = EV_DEFAULT;

#if EV_COMPAT3
	puts("EV_COMPAT3 is set");
#endif

	server_fd = unix_socket_connect("/tmp/echo.sock");
	if (server_fd == SOCKET_CONNECT_FAILED)
	{
		fprintf(stderr, "Error on socket connection. Exit now\n");
		exit(1);
	}

	/* get notified on incomming connections */
	ev_io_init(&connection_watcher, on_connection, server_fd, EV_READ);
	ev_io_start(loop, &connection_watcher);

	/* simple timer, repeating every 5 seconds */
	ev_timer_init(&timeout_watcher, timeout_cb, 0., 5.);
	ev_timer_again(loop, &timeout_watcher);

	ev_run(loop, 0);

	return 0;
}
