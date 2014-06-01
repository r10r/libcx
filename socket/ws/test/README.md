## > 1024 File selectors

* set limit with ulimit -n 1024
* https://github.com/locustio/locust/issues/121

Sounds like it could be caused by the limit of 1024 file-descriptiors on select(2) on BSD (and thus osx). libev should probably use kqueue rather than select on OSX but I guess that the usage of libev on OSX isn't that common so it well may still be using select.


* Backend on OSX must be either EVBACKEND_POLL / EVBACKEND_KQUEUE
* http://pod.tst.eu/http://cvs.schmorp.de/libev/ev.pod#OS_X_AND_DARWIN_BUGS
* _DARWIN_UNLIMITED_SELECT doesn't work on OSX 10.9.x ?

* build libev with debugging enabled
	
		export CFLAGS="-gdwarf-2 -g -O0 -fno-inline"
		./configure
		make 
		make install





https://github.com/tavendo/AutobahnTestSuite/blob/master/doc/usage.rst




## Scaling ?

http://www.serverframework.com/asynchronousevents/2010/12/one-million-tcp-connections.html


http://stackoverflow.com/questions/2332741/what-is-the-theoretical-maximum-number-of-open-tcp-connections-that-a-modern-lin




## Benchmark 

* http://danielmendel.github.io/blog/2013/04/07/benchmarkers-beware-the-ephemeral-port-limit/
* http://www.metabrew.com/article/a-million-user-comet-application-with-mochiweb-part-1