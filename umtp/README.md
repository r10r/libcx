libcx-umtp
==========

## Parser Modularization

* Parsing a complex protocol with multiple machines 
  should be done using the ragel builtin *Parser Modularization (6.1)*

## Chaining

Calling another machine on the remaining input without jumping back.

### Method 1.

Using `fbreak` from a ragel action. 
But unfortunately this produces dead code error by the compiler with `-Wunused-code` enabled.

### Method 2.

The terminating character must be specified and the machine must be switched
in the event emmited by the incomming transition of the separator character.

	main := ( ( protocol_line LF envelope ) (LF >CountToken >SetMarker >BodyStart)? )$CountToken;

Since we match the incomming transition of the separator `LF` we must
first advance to the next character using `CountToken`, set the marker with `SetMarker`
and finally emit the event with `BodyStart`.