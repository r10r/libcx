## TODO

## PING heart beat / connection timeout

* when connection receives no data a PING is sent
* a timeout timer is start that closes the connection if 
* neither the PONG nor any other data was received in that time

### API

* implement a nice api (like send/receive)


### Performance Benchmark

* Multithreaded Benchmarking
* Fix problems when one connection is blocking other connections
* Equally share send/receiv between connections (lower send size ?)

### Buffering

* optimize buffering
* disable buffer shifting and growing
* use fixed read size and allocate a buffer per frame, as soon as we have parsed the header

### UTF8

* Implement UTF8 decoding/encoding: http://bjoern.hoehrmann.de/utf-8/decoder/dfa/


### Misc

* Remove ssize_t types as they are not C99 conform (only posix)



