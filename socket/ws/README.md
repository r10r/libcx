## TODO

* optimize pings (only two bytes, no StringBuffer required) 

## deflate extension

http://tools.ietf.org/html/draft-ietf-hybi-permessage-compression-18
https://www.igvita.com/2013/11/27/configuring-and-optimizing-websocket-compression/
http://ruby-doc.org/stdlib-1.9.3/libdoc/zlib/rdoc/Zlib/Deflate.html

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


### 

http://stackoverflow.com/questions/13040752/websockets-udp-and-benchmarks
http://blog.artillery.com/2012/06/websocket-performance.html

