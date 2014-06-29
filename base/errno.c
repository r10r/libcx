#include "errno.h"
// http://stackoverflow.com/questions/11699596/how-to-set-errno-value
// http://programmers.stackexchange.com/questions/209729/rationale-behind-c-library-functions-never-setting-errno-to-zero

__thread int cx_errno = 0;
