#include "errno.h"
// http://stackoverflow.com/questions/11699596/how-to-set-errno-value
// http://programmers.stackexchange.com/questions/209729/rationale-behind-c-library-functions-never-setting-errno-to-zero

__thread int cx_errno = 0;

void
dbg_set_cx_errno(int err, const char* file, int line, const char* func)
{
	if (err != 0)
	{
		flockfile(stderr);
		fprintf(stderr, "CX_ERRNO %d - %s %d (%s)\n", err, file, line, func);
	}
	cx_errno = err;
}
