#include "errno.h"
#include "debug.h"
#include <assert.h>

// http://stackoverflow.com/questions/11699596/how-to-set-errno-value
// http://programmers.stackexchange.com/questions/209729/rationale-behind-c-library-functions-never-setting-errno-to-zero

//#define CX_ERR_MESSAGE_LENGTH 128

/* WARN never access this directly */
__thread int
CX_HIDDEN(err_code) = CX_ERR_OK;

//static __thread char _cx_err_message[CX_ERR_MESSAGE_LENGTH];

//const char*
//cx_err_message()
//{
//	return _cx_err_message;
//}

static uint32_t uid_counter = 0;
static pthread_mutex_t uid_counter_mutex[] = { PTHREAD_MUTEX_INITIALIZER };
static __thread char _cx_uid[CX_UID_LENGTH] = "";

uint32_t
cx_uid_counter()
{
	return uid_counter;
}

const char*
cx_uid()
{
	return _cx_uid;
}

const char*
cx_next_uid()
{
	uint32_t counter;

	pthread_mutex_lock(uid_counter_mutex);
	counter = uid_counter++;
	pthread_mutex_unlock(uid_counter_mutex);

	snprintf(_cx_uid, CX_UID_LENGTH, "%u-%u", (unsigned)time(NULL), counter);

	return _cx_uid;
}
