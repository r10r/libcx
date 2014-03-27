#include "message_parser.h"

%%{
	machine message_fsm;
	
	# keeps parser machine state in a struct
	access parser->;
	variable p parser->buffer_position;
	variable pe parser->buffer_end;
	variable eof parser->eof;
	
	action SetMarker{SetMarker(parser);}
	action CountToken{CountToken(parser);}
	action ProtocolValue{EmitEvent(parser, P_PROTOCOL_VALUE);}
	action HeaderName{EmitEvent(parser, P_HEADER_NAME);}	
	action HeaderValue{EmitEvent(parser, P_HEADER_VALUE);}
	# FIXME multipass must jump out of the machine after emitting body start
	# using fbreak results in dead code
	action BodyStart{EmitEvent(parser, P_BODY_START); }
	
	LF = '\r'? '\n';
	SP = [ \t];
	
	protocol_value = graph* >SetMarker %ProtocolValue;
	
	protocol_line = protocol_value (SP+ protocol_value)*;
	header_name = (alnum | '-' | '_' )+;
	header_value = print*;
	
	header = header_name >SetMarker %HeaderName
			 SP* ':' SP*
			 header_value >SetMarker %HeaderValue;
	
	envelope = (header LF)*;
	
	# body is optional and always separated by a linefeed
	main := ( ( protocol_line LF envelope ) (LF >CountToken >SetMarker >BodyStart)? )$CountToken;
}%%

%% write data;

void
message_fsm_parse(RagelParser* parser)
{
	if (parser->iterations == 0)
		%% write init;

	RagelParser_update(parser);

	%% write exec;
	
	parser->iterations++;
}
