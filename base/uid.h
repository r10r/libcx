#ifndef _CX_UID_H
#define _CX_UID_H

#include <pthread.h>
#include <time.h>
#include <stdint.h>
#include <stdio.h>      /* snprintf */
#include <stddef.h>     /* NULL */

/* very simple and dump thread-safe UID implementation */

/* currently limits the number of UIDs to about 65k per second */
#define CX_UID_LENGTH   (10 + 5 + 1) /* timestamp (uint32_t) + counter (uint16_t) + \0 */

/*
 * Increments the UID counter.
 *
 * @return the current UID
 */
void
cx_uid_next(char* uid_ptr);

#endif
