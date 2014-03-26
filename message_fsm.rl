#include "message.h"

%%{
	machine message_fsm;
	
	# keeps parser machine state in a struct
	access parser->;
	variable p parser->buffer_position;
	variable pe parser->buffer_end;
	variable eof parser->eof;
	
	LF = '\r'? '\n';
	SP = [ \t];
	
	protocol_value = graph* >{SetMarker(parser)} %{EmitEvent(parser, P_PROTOCOL_VALUE)};
	
	protocol_line = protocol_value (SP+ protocol_value)*;
	
	header_name = (alnum | '-' | '_' )+;
	
	header_value = print*;
	
	header = ( header_name  >{SetMarker(parser)} %{EmitEvent(parser, P_HEADER_NAME)} ) 
				SP* ':' SP* 
			 	   ( header_value >{SetMarker(parser)} %{EmitEvent(parser, P_HEADER_VALUE)} );
	
	envelope = (header LF)*;
	
	body = any* >{SetMarker(parser)} %{EmitEvent(parser, P_BODY)};
	
	main :=  ( protocol_line LF envelope (LF body)? ) ${CountToken(parser)};  
}%%

%% write data;

void
RagelParser_parse(RagelParser* parser)
{
	if (parser->iterations == 0)
		%% write init;

	RagelParser_update(parser);

	%% write exec;
	
	parser->iterations++;
}
