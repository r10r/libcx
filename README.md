## START

1. Server connects to socket
2. Server starts workers 

### Worker
* a Worker starts IO watcher to accept work/connections
-> in the init/start method

* a Worker gets signaled when a 

accept ~= get work
connection ~= work

* For each connection the worker starts an IO watcher to receive incomming data

a connection is a portion of work
a connection should be tested apart from the worker (hide accept call !!!)


* a Worker starts a timeout watcher that cycles through the active connections
  to check if the timeout has expired.  


### Connection
* a Worker has a list of connections (FIFO)
* a Worker has a connection handler

* each connection is identified by the connectino id (socket FD)


### Questions

* decouple worker from connection ?

A worker just does work !!!!
it calls a handler when there is work to do
A connection is a portion of work.


connection watcher is a kind of "work watcher"

use a worker prototoype and clone it when creating workers ?

### Worker

* a task might be assigned to a worker
* a worker might pick a task itself

a worker might check whether he accepts a task if he 
has a lot of pending tasks (leave the work to others)

Do not accept a connection when a lot of connections are served.
How does the round robin behave with multiple threads acdepting
on the same server socket.

### Load Balancing

http://domsch.com/linux/lpc2010/Scaling_techniques_for_servers_with_high_connection%20rates.pdf


[The SO_REUSEPORT socket option](http://lwn.net/Articles/542629/)


http://docs.python.org/2/library/socketserver.html

### Task

* a task might be a verification task
* a task might be splitted into several steps
* multiple tasks might be handled at once in the event loop

begin		-> CONNECTION
continue	-> DATA
continue	-> DATA
continue	-> DATA
finish		-> EOF
finish		-> CLOSE



	CONNECTION
	DATA
	DATA
	...
	EOF
	PROCESS
	CRL REQUEST ( proxy)
	HTTP PROXY REQUEST
	HTTP PROXY REQUEST

### CRL Proxy

http://www.syseleven.de/blog/1756/lastverteilung-und-caching-mit-nginx/
http://www.goitworld.com/how-to-use-nginx-proxy-cache-replace-squid/
http://fournines.wordpress.com/2011/12/02/improving-page-speed-cdn-vs-squid-varnish-nginx/
http://ef.gy/using-nginx-as-a-proxy-server

http://www.squid-cache.org/

use redis as CRL proxy ? (maybe the fastest)

http://wiki.nginx.org/HttpProxyModule#proxy_cache_key

### Pool Supervisor

* a supervisor that manages the pool

* iterate over workers and inspect their state
* (forcefully) stop/restart workers on timeout (pthread_cancel) / async send

* start new threads if work/connection backlog is to high

* get the number of open connections
* get the number of handled connections
* get the number of errors ?

* dump statistics for all workers

### Events


### Timeouts

http://stackoverflow.com/questions/2876024/linux-is-there-a-read-or-recv-from-socket-with-timeout
http://www.gnu.org/software/libc/manual/html_node/Accepting-Connections.html

### Backlog / Pending Connections

Self test the application by opening a connection to the socket ?
-> client will get ECONNREFUSED -> increase the number of threads ?

http://stackoverflow.com/questions/4814948/know-the-size-of-accept-pending-connections-queue-in-gnu-linux

http://publib.boulder.ibm.com/infocenter/aix/v6r1/index.jsp?topic=%2Fcom.ibm.aix.progcomm%2Fdoc%2Fprogcomc%2Fskt_check_ex.htm

[[PATCH iproute 1/2] ss: show send queue length on unix domain sockets](http://www.spinics.net/lists/netdev/msg227274.html)


[Maximum size of data using connectionless unix domain socket](http://developerweb.net/viewtopic.php?id=3605)

### Encapsulation Concepts

http://psomas.wordpress.com/2009/07/01/weird-kernel-macros-container_of/

* use container_of | offsetof for encapsulation
* use casting for inheritance
* use a void* userdata pointer


## Task

* a task should integrate in the event loop ?




### 

A server ~= a connector
* serves a single port / socket / protocol
* actually you don't mix protocols per port ;)


chaining handlers ?

List of handlers ?
Subrequests ?



### loop iteration

* process pending requests
* accept incomming connections
* receive data



### Questions

* send_buffer (send response)

#### What happens when the request is parsed ?

* it must be queued to the request queue
* it must be processed by the request handler
* the request handler in turn might issue subrequests (e.g CRL lookup) using IO watchers


#### When is the request queue handled ?

### Is pipelining required

* currently not 
* we must not stop the IO watcher when pipelining is required
* we need a request queue 

Separate concerns
-> better testable

#### IO Threads

* do the IO work, no compuational work

* receive request 
* fetch CRL
* send response

#### Task Threads

* do the heavy computational work
* they pull requests from a FIFO (one after another)

#### FIFO synchronisation

* Push should work independently from pop as long as there is more than one element


https://chromium.googlesource.com/chromium/chromium/+/master/base/threading


#### Thread Pool

* simply a linked list
* we don't assign work / workers pull work from the queue


connection has a fixed buffer !!!!
message buffer grows


### Pipelining
*implement Pipelining http://de.wikipedia.org/wiki/HTTP-Pipelining
* HTTP has a length header, this is difficult with streaming protocols
* with undefined length
* SMTP has the '.' terminator (maybe we could use that magic '.' too)
* We definitely need an END OF request terminator