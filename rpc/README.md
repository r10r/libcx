* json rpc to struct conversion ?
* create an adapter for existing apis

## JSON RPC 2.0 Questions

* Errors on Notifications (e.g invalid method)

-> http://www.jsonrpc.org/specification#notification 

	A server must not reply to a notification, be it an error or not

If there was an error in detecting the id in the 
Request object (e.g. Parse error/Invalid Request), it MUST be Null.

### Validation
* TODO handle parameter validation (greater than, lower then, not null, ....)
* handle validation transparently in the deserialize callback ?
* attach validator to param ?
* some small parser would be nice when using validation strings
* http://stackoverflow.com/questions/2641473/initialize-static-array-of-structs-in-c


## Response

* write to a memory buffer
* the buffer is flushed to the output stream
* streaming response: write -> flush -> clear 

* http://stackoverflow.com/questions/1716296/why-does-printf-not-flush-after-the-call-unless-a-newline-is-in-the-format-strin
 * http://stackoverflow.com/questions/11529500/setvbuf-for-file-descriptor-created-by-open-in-c
 * http://stackoverflow.com/questions/12450066/flushing-buffers-in-c
 * http://stackoverflow.com/questions/2171412/flushing-the-socket-streams-in-c-socket-programming
 
	 SO_SNDBUF       set buffer size for output
	 SO_RCVBUF       set buffer size for input
 