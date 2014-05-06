#include "websocket.h"
#include "base/debug.h"
#include "string/string_buffer.h"
#include <inttypes.h> /* print types uint64_t */
#include "util.h"

/*
 * Header fields with fixed offset and length.
 *
 * - see [Data Notations - Big Endian](https://tools.ietf.org/html/rfc1700)
 * - see [Websockets - Base Framing Protocol](https://tools.ietf.org/html/rfc6455#section-5.2)
 *
 * Bit numbering must be reversed.
 */
const HeaderField WS_HDR_FIN  =  { .type = HDR_FIELD_BOOL, .offset = 0, .bitmask = BIT(7) };
const HeaderField WS_HDR_RSV1 = { .type = HDR_FIELD_BOOL, .offset = 0, .bitmask = BIT(6) };
const HeaderField WS_HDR_RSV2 = { .type = HDR_FIELD_BOOL, .offset = 0, .bitmask = BIT(5) };
const HeaderField WS_HDR_RSV3 = { .type = HDR_FIELD_BOOL, .offset = 0, .bitmask = BIT(4) };
const HeaderField WS_HDR_OPCODE = { .type = HDR_FIELD_OCTET, .offset = 0, .bitmask = 0xf };
const HeaderField WS_HDR_MASKED = { .type = HDR_FIELD_BOOL, .offset = 1, .bitmask = BIT(7) };
const HeaderField WS_HDR_PAYLOAD_LENGTH = { .type = HDR_FIELD_OCTET, .offset = 1, .bitmask = 0x7f };
const HeaderField WS_HDR_PAYLOAD_LENGTH_EXT = { .type = HDR_FIELD_INT16, .offset = 2, .bitmask = HDR_MASK_ALL };
const HeaderField WS_HDR_PAYLOAD_LENGTH_EXT_CONTINUED = { .type = HDR_FIELD_INT64, .offset = 2, .bitmask = HDR_MASK_ALL };
