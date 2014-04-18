* json rpc to struct conversion ?
* create an adapter for existing apis

## JSON RPC 2.0 Questions

* Errors on Notifications (e.g invalid method)

-> http://www.jsonrpc.org/specification#notification 

	A server must not reply to a notification, be it an error or not



If there was an error in detecting the id in the 
Request object (e.g. Parse error/Invalid Request), it MUST be Null.



### Validation
TODO handle parameter validation (greater than, lower then, not null, ....)
// handle validation transparently in the deserialize callback ?
// or attach validator to param ?
// some small parser would be nice when using validation strings


// http://stackoverflow.com/questions/2641473/initialize-static-array-of-structs-in-c
