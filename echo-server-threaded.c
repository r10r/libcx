#include "server.h"
#include "libcx-workqueue/pool.h"

WorkerPool *pool;
ev_signal sigint_watcher;
ev_timer shutdown_timer;

/* check if all workers are released and work is done */
static void
shutdown_cb(ev_loop *loop, ev_timer *w, int revents)
{
	if (pool->pending_work->length == 0 &&
	    (pool->idle_workers->length == pool->workers->length))
		ev_break(loop, EVBREAK_ALL);
}

/* handle SIGINT callback (starts the shutdown timer) */
static void
sigint_cb(ev_loop *loop, ev_signal *w, int revents)
{
	ev_timer *schedule_work_timer = (ev_timer *) malloc(sizeof(ev_timer));
	/* stop adding work */
	printf("Starting shutdown timer. Waiting for pending requests\n");
	ev_timer_stop(loop, schedule_work_timer);
	shutdown_timer.data = pool;
	ev_timer_init(&shutdown_timer, shutdown_cb, 0., .01);
	ev_timer_again(EV_DEFAULT, &shutdown_timer);
}

static void
setup_callbacks(WorkerPool *pool)
{
	/* handle SIGINT */
	sigint_watcher.data = pool;
	ev_signal_init(&sigint_watcher, sigint_cb, SIGINT);
	ev_signal_start(EV_DEFAULT, &sigint_watcher);
}

void
worker_watch_connection(Worker *worker)
{
	/* get notified on incoming connection */
	ev_io *connection_watcher = (ev_io *) malloc(sizeof(ev_io));
	ev_io_init(connection_watcher, on_connection, server_fd, EV_READ);
	ev_io_start(worker->loop, connection_watcher);
	XFLOG("Worker[%d] - waiting for connection\n", worker->id);
}

int
main(int argc, char** argv)
{
	int num_threads = atoi(argv[1]);

	/* SIGINT is handled by a callback */
	signal(SIGINT, SIG_IGN);

	/* use the default event loop unless you have special needs */
	ev_loop *loop = EV_DEFAULT;
	pool = WorkerPool_new();
	pool->loop = loop;
	pool->num_workers = num_threads;
	pool->f_worker_init = worker_watch_connection;

	server_fd = unix_socket_connect("/tmp/echo.sock");
	if (server_fd == SOCKET_CONNECT_FAILED)
	{
		fprintf(stderr, "Error on socket connection. Exit now\n");
		exit(1);
	}

	setup_callbacks(pool);
	WorkerPool_start(pool);
	ev_run(loop, 0);
	WorkerPool_free(pool);

	return 0;
}
