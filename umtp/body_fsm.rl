#include "message_parser.h"

void
body_fsm_parse(RagelParser* parser);

%%{
	machine body_fsm;
	
	# keeps parser machine state in a struct
	access parser->;
	variable p parser->buffer_position;
	variable pe parser->buffer_end;
	variable eof parser->eof;
	
	action SetMarker{SetMarker(parser);}
	action CountToken{CountToken(parser);}
	action TestEvent{EmitEvent(parser, 1);}
	
	# lose implementation of the BASE64 based PEM format
	# see http://tools.ietf.org/html/rfc1421
	LF = '\r'? '\n';

	main := (('XYZ' >SetMarker %TestEvent LF)* LF )$CountToken;
}%%

%% write data;

void
body_fsm_parse(RagelParser* parser)
{
	if (RagelParser_firstrun(parser))
		%% write init;
	
	RagelParser_update(parser);

	%% write exec;
	
	// Do not dereference parser->buffer_position here!
	// It points to unitiailized memory (S_last(parser->buffer) + 1)
	
	// FIXME iterations is incremented twice on the first run 
	// when called from withing the previous parser
	parser->iterations++;
}
