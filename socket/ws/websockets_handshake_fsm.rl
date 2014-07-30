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
	

		# 	    GET /chat HTTP/1.1
		# 		Host: server.example.com
		# S     Upgrade: websocket
		# S     Connection: Upgrade
		#       Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==
		#       Origin: http://example.com
		#       Sec-WebSocket-Protocol: chat, superchat
		# S     Sec-WebSocket-Version: 13

    resource = (any -- SP)*;
	
	# static required header lines 
	upgrade_line = 'Upgrade: websocket' LF;
	connection_line = 'Connection: Upgrade' LF;
	version_line = 'Sec-WebSocket-Version: 13' LF;
	
	static_required_headers = (upgrade_line | connection_line | version_line);
	
	
	# body is optional and always separated by a linefeed
	main := (
		'GET' SP (resource >SetMarker $Resource) SP 'HTTP/1.1' LF
		header* LF
		(upgrade_line | connection_line | version_line);
		
		
		(LF >CountToken >SetMarker >BodyStart)? )$CountToken;
}%%

%% write data;

void
message_fsm_parse(RagelParser* parser)
{
	if (RagelParser_firstrun(parser))
		%% write init;

	RagelParser_update(parser);

	%% write exec;
	
	parser->iterations++;
}
