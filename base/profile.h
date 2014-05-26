/*
 * Simple header for tracing time.
 * Include and define TRACE to enable.
 * @notice Not Thread Safe
 */
#ifndef _CX_PROFILE_H
#define _CX_PROFILE_H

#ifndef _CX_PROFILE

#define PROFILE_INIT
#define PROFILE_BEGIN(message) UNUSED(message)
#define PROFILE_BEGIN_FMT(format, ...) UNUSED(format);
#define PROFILE_END

#else

#include <time.h>       /* clock_t, CLOCKS_PER_SEC */
#include <sys/time.h>   /* gettimeofday */
#include <stdio.h>

#ifndef CLOCKS_PER_SEC
/* (on linux CLOCKS_PER_SEC is undefined, see man 3 clock */
#define CLOCKS_PER_SEC 1000000
#endif

static struct timeval _PROFILE_TIME_START;
static struct timeval _PROFILE_TIME_END;
static double _PROFILE_TIME_DIFF;
static clock_t _PROFILE_CLOCK_START;
static clock_t _PROFILE_CLOCK_END;
static double _PROFILE_CLOCK_DIFF;

/*
 * Leaving the variadic args off only works with LLVM/GCC
 * see http://gcc.gnu.org/onlinedocs/cpp/Variadic-Macros.html
 *
 */
#define PROFILE_BEGIN_FMT(fmt, ...) \
	gettimeofday(&_PROFILE_TIME_START, NULL); \
	_PROFILE_CLOCK_START = clock(); \
	puts("~~> PROFILE BEGIN"); \
	printf("~~? " fmt, __VA_ARGS__);

#define PROFILE_BEGIN(message) \
	gettimeofday(&_PROFILE_TIME_START, NULL); \
	_PROFILE_CLOCK_START = clock(); \
	puts("~~> PROFILE BEGIN"); \
	puts("~~? " message);

#define PROFILE_END \
	gettimeofday(&_PROFILE_TIME_END, NULL); \
	_PROFILE_TIME_DIFF = timeval_diff(&_PROFILE_TIME_START, &_PROFILE_TIME_END); \
	_PROFILE_CLOCK_END = clock(); \
	_PROFILE_CLOCK_DIFF = (double)(_PROFILE_CLOCK_END - _PROFILE_CLOCK_START) / CLOCKS_PER_SEC * 1000.0; \
	printf("<~~ PROFILE END (real: %.4f ms, cpu: %.4f ms)\n", _PROFILE_TIME_DIFF, _PROFILE_CLOCK_DIFF);

#endif

#endif
