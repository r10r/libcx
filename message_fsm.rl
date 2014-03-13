#include "message.h"
#include "libcx-string/string.h"

%%{
	machine message_fsm;
	
	action PrintToken {
		XFLOG("Token[%ld] %c -> %p\n", 
			state->buffer_offset, *(state->buffer_position), state->buffer_position);
	}
	
	action SetMarker {
		XFLOG("Mark[%ld] %c -> %p\n", 
			state->buffer_offset, *(state->buffer_position), state->buffer_position);
		state->marker = state->buffer_position;
		state->marker_offset = 0;
	}
	
	action CountToken {
		state->buffer_offset++;
		state->marker_offset++;
	}
	
	action EventProtocolValue { _event(message, P_PROTOCOL_VALUE); }
	action EventHeaderName { _event(message, P_HEADER_NAME); }
	action EventHeaderValue { _event(message, P_HEADER_VALUE); }
	action EventBody { _event(message, P_BODY); }
	
	# keeps parser machine state in a struct
	access state->;
	variable p state->buffer_position;
	variable pe state->buffer_end;
	variable eof state->eof;
	
	LF = '\r'? '\n';
	SP = [ \t];
	
	protocol_value = graph* >SetMarker %EventProtocolValue;
	
	protocol_line = protocol_value (SP+ protocol_value)*;
	
	header_name = (alnum | '-' | '_' )+;
	
	header_value = print*;
	
	header = ( header_name  >SetMarker %EventHeaderName ) 
				SP* ':' SP* 
			 	   ( header_value >SetMarker %EventHeaderValue );
	
	envelope = (header LF)*;
	
	body = any* >SetMarker %EventBody;
	
	main :=  ( protocol_line LF envelope (LF body)? ) $CountToken;  
}%%
	
static inline void 
_event(Message *message, ParseEvent event)
{
	message->parser_state->event = event;
	message->parser_state->f_event_handler(message);
}

%% write data;

void
ragel_parse_message(Message *message)
{
	RagelParserState *state = message->parser_state;
	if (state->event == P_NEW)
		%% write init;

	%% write exec;
}
