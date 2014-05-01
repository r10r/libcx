#include "util.h"
/*
 * host order is LE (unless we are working on a mainframe ;), network byte order of TCP BE
 * http://stackoverflow.com/questions/2100331/c-macro-definition-to-determine-big-endian-or-little-endian-machine
 *
 */

uint64_t
htobe64(uint64_t x)
{
	return (x >> 56) |
  	((x << 40) & 0x00ff000000000000LL) |
  	((x << 24) & 0x0000ff0000000000LL) |
  	((x << 8) & 0x000000ff00000000LL) |
  	((x >> 8) & 0x00000000ff000000LL) |
  	((x >> 24) & 0x0000000000ff0000LL) |
  	((x >> 40) & 0x000000000000ff00LL) |
  	(x << 56);
}

uint64_t
be64toh(uint64_t x)
{
	return (x >> 56) |
		((x << 40) & 0x00ff000000000000LL) |
		((x << 24) & 0x0000ff0000000000LL) |
		((x << 8) & 0x000000ff00000000LL) |
		((x >> 8) & 0x00000000ff000000LL) |
		((x >> 24) & 0x0000000000ff0000LL) |
		((x >> 40) & 0x000000000000ff00LL) |
		(x << 56);
}

struct websocket_frame_t
{

};


size_t
get_payload_offset(uint64_t payload_length)
{

}

size_t
getPayloadLength(uint8_t* input_buf, size_t buf_length)
{
	uint8_t payload_length = input_buf[1] & CONTINUED_EXTENDED_PAYLOAD_LENGTH;
	uint64_t extended_payload_length;

	/* TODO check for incomplete frames !!! and payload length > CONTINUED_EXTENDED_PAYLOAD_LENGTH  */

	if (payload_length < EXTENDED_PAYLOAD_LENGTH)
		extended_payload_length = payload_length;
	if (payload_length == EXTENDED_PAYLOAD_LENGTH)
		extended_payload_length = ntohs((uint16_t)input_buf[2]);
	else if (payload_length == CONTINUED_EXTENDED_PAYLOAD_LENGTH)
		extended_payload_length = be64toh((uint64_t)input_buf[2]);

	XFDBG("Payload length:  %zu\n", extended_payload_length);

	return extended_payload_length;
}
