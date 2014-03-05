/*
 * Simple header for tracing time.
 * Include and define TRACE to enable.
 * @notice Not Thread Safe
 */
#ifndef _TRACE_H
#define _TRACE_H

#ifdef TRACE

#include <sys/time.h> /* clock, gettimeofday, CLOCKS_PER_SEC */
#include <stdio.h>

#ifndef CLOCKS_PER_SEC
/* (on linux CLOCKS_PER_SEC is undefined, see man 3 clock */
#define CLOCKS_PER_SEC 1000000
#endif

static struct timeval _TRACE_TIME_START;
static struct timeval _TRACE_TIME_END;
static double _TRACE_TIME_DIFF;

static clock_t _TRACE_CLOCK_START;
static clock_t _TRACE_CLOCK_END;
static double _TRACE_CLOCK_DIFF;

static const char *_TRACE_BEGIN_FMT =
	"~~> TRACE BEGIN";
static const char *_TRACE_END_FMT =
	"<~~ TRACE END (real: %.4f ms, cpu: %.4f ms)\n";

/*
 * Leaving the variadic args off only works with LLVM/GCC
 * see http://gcc.gnu.org/onlinedocs/cpp/Variadic-Macros.html
 *
 */
#define TRACE_BEGIN_FMT(fmt, ...) \
	gettimeofday(&_TRACE_TIME_START, NULL); \
	_TRACE_CLOCK_START = clock(); \
	puts(_TRACE_BEGIN_FMT); \
	printf("~~? " fmt, __VA_ARGS__);

#define TRACE_BEGIN(message) \
	gettimeofday(&_TRACE_TIME_START, NULL); \
	_TRACE_CLOCK_START = clock(); \
	puts(_TRACE_BEGIN_FMT); \
	puts("~~? " message);

#define TRACE_END \
	gettimeofday(&_TRACE_TIME_END, NULL); \
	_TRACE_TIME_DIFF = (_TRACE_TIME_END.tv_sec - _TRACE_TIME_START.tv_sec) * 1000.0; \
	_TRACE_TIME_DIFF += (_TRACE_TIME_END.tv_usec - _TRACE_TIME_START.tv_usec) / 1000.0; \
	_TRACE_CLOCK_END = clock(); \
	_TRACE_CLOCK_DIFF = (double)(_TRACE_CLOCK_END - _TRACE_CLOCK_START) / CLOCKS_PER_SEC * 1000.0; \
	printf(_TRACE_END_FMT, _TRACE_TIME_DIFF, _TRACE_CLOCK_DIFF);
#else
#define TRACE_BEGIN
#define TRACE_END
#endif

#endif
