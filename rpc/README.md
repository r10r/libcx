## TODO

* Improve tests !!! 
* Create tests for all test cases listed in the JSON RPC 2.0 spec
--> request <--- response 

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
 
 
### JSON Schema

* optional JSON Schema support
* complex parameter type description / structs

http://json-schema.org/examples.html
http://www.simple-is-better.org/json-rpc/jsonrpc20-schema-service-descriptor.html

Struct <-> JSON Object conversion

* document return value (same as parameter macros ?)

## GObject

* https://developer.gnome.org/gobject/stable/index.html
* http://stef.thewalter.net/2010/10/this-arent-benchmarks-youre-looking-for.html


		{ "name", type, method_used_for_deserialization, flags }
		
		{ "name", typeinfo, flags }
		{ "name", { type, method_used_for_deserialization}, flags }


### Typesafe Parameter retrieval

### Debugging

	clang -E rpc/hello_service.c | uncrustify -c ~/.uncrustify.cfg  | less

### Batch Requests


* option for the service to multiplex multiple requests ?
* pass a list of requests to the service ?
* service generates a deferable response that returns data when all requests have been handled.

* MPD for example can multiplex commands (with mpd_command_list_begin/end)

## TODO 

* test-suite (like autobahn test suite for websockets) to test protocol implementation


### Test Requests


{"jsonrpc": "2.0", "id": 66, "method": "play", "params" : ["Nice"]}
{"jsonrpc": "2.0", "id": 66, "method": "get_person"}
{"jsonrpc": "2.0", "id": null, "error": "get_person"}
{ "firstname": "Max", "lastname": "Mustermann", "age": 33 }

[1, {"jsonrpc": "2.0", "id": 66, "method": "print_person", "params": [{ "firstname": "Max", "lastname": "Mustermann", "age": 33 }]}]


## Error Handling

* A specific error code is required when the error can be handled.
* A uid is required to link an error response to the application log.
* The debug information (file, line, func) is required to spot 
  the exact code location where the error was emitted.

For internal method errors we generate a UID and log
the provided message together with the UID and debug information to stderr.

After executing a method we have to check for an internal error, if so
we set the UID as reason and set the errry to INTERNAL.

The method wrapper can check the cx_err_code value for the method it calls
and append additional error information to the request


### JSON RPC 2.0 Error Codes

* Development Errors: Invalid Requests / Invalid Method Calls
* Application Errors:  -32603 (Internal Error)
    * application logic / resource errors (memory allocation, connections ...)
    * can not be handled by the client ?

An application error can be a Policy Violation 
    - (rate) limits

An error during a method invocation failure.

Provide a proper error message when the error can be handled.

Dump errornous requests / responses ?

* Application error codes   

    * Errors that can be handled automatically by the application
    * Errors that require user interaction
    
    * Errors that are purely informal and require administrator interaction (e.g permament connection failures to the backend)
    
        --> can be anonymous
        --> only specify what part of the system does not work
            e.g "The MPD backend is currently unavailable."

#### UID

- simple version: Unix Timestamp + Threadsafe Counter
- complex version: libuuid '#include <uuid/uuid.h>'

Does the simple version do information disclosure (e.g the number of errors occured) ?

#### Error Response

        {
          "jsonrpc": "2.0",
          "id": null,
          "error": {
            "data": {
              "token": "5FF0E3EB-7258-4370-A125-1A58ADBFB2A2"
              "details": "invalid token near 'foo'"
            },
            "message": "Parse error",
            "code": -32700
          }
        }