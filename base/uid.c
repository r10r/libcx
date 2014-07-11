#include "uid.h"

static uint16_t uid_counter = 0;
static pthread_mutex_t uid_counter_mutex[] = { PTHREAD_MUTEX_INITIALIZER };

void
cx_uid_next(char* uid_ptr)
{
	uint16_t counter;

	pthread_mutex_lock(uid_counter_mutex);
	counter = uid_counter++;
	pthread_mutex_unlock(uid_counter_mutex);

	snprintf(uid_ptr, CX_UID_LENGTH, "%010u%05u", (unsigned)time(NULL), counter);
}
