#include "base/test.h"
#include "string/string_buffer.h"
#include "rpc/mpd/mpd_service.h"

#define TESTFILE "/Users/ruben/Code/libcx/rpc/playlist.rpc.json"


static void
test_write_object()
{
	Pipeline* pipeline = RPC_Pipeline_new();

	RPC_Method mpd_methods[] = { RPC_methods(MusicPlayerDaemon), RPC_Method_none };

	// read request
	FILE* f = fopen(TESTFILE, "r");

	StringBuffer_fload(pipeline->request_buffer, f, 128);
	fclose(f);

	RPC_Pipeline_process(pipeline, mpd_methods);
	RPC_Pipeline_free(pipeline);
}

int
main()
{
	TEST_BEGIN

	RUN(test_write_object);

	TEST_END
}
