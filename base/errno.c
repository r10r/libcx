#include "errno.h"

// http://stackoverflow.com/questions/11699596/how-to-set-errno-value
// http://programmers.stackexchange.com/questions/209729/rationale-behind-c-library-functions-never-setting-errno-to-zero

/* WARN never access this directly */
__thread int
CX_HIDDEN(err_code) = CX_ERR_OK;
