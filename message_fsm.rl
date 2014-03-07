#include "parser.h"

%%{
	machine message_fsm;
	
	action Print {
		XFLOG("Token[%ld] %c -> %p\n", state->token_count, *(state->p), state->p);
	}
	
	action Mark {
		XFLOG("Mark[%ld] %c -> %p\n", state->token_count, *(state->p), state->p);
		state->mark = state->p;
		state->mark_offset = 0;
	}
	
	action Count {
		state->token_count++;
		state->mark_offset++;
	}
	
	action ProtocolValue {
		if (state->cb) state->cb(state, message, P_PROTOCOL_VALUE);
	}
	
	action HeaderName {
		if (state->cb) state->cb(state, message, P_HEADER_NAME);
	}
	
	action HeaderValue {
		if (state->cb) state->cb(state, message, P_HEADER_VALUE);
	}
	
	action Body {
		if (state->cb) state->cb(state, message, P_BODY);
	}
	
	# keep machine state in a struct
	access state->;
	variable p state->p;
	variable pe state->pe;
	variable eof state->eof;
	
	LF = '\r'? '\n';
	SP = [ \t];
	
	protocol_value = graph* >Mark %ProtocolValue;
	protocol_line = protocol_value (SP+ protocol_value)*;
	header_name = (alnum | '-' | '_' )+;
	header_value = print*;
	header = (header_name  >Mark %HeaderName) SP* ':' SP* header_value >Mark %HeaderValue;
	envelope = (header LF)*;
	body = any* >Mark %Body;
	
	main :=  ( protocol_line LF envelope (LF body)? ) $Count;  
}%%


int 
ragel_parse_message(MachineState *state, Message *message, bool eof)
{
	%% write data;
	%% write init;
	state->p = &(message->data)[0];
	state->pe = &(message->data)[message->data_size];
	if (eof) state->eof = state->pe;
	
	%% write exec;
	
	if (state->p != state->pe) {
		printf("Only consumed %lu of %lu tokens: %p -> [%c]\n", 
			state->token_count, message->data_size, state->p, *(state->p));
	}
	return state->token_count;
}
