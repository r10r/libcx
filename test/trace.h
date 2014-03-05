/*
 * Simple header for tracing time.
 * Include and define TRACE to enable.
 * @notice Not Thread Safe
 */
#ifndef _TRACE_H
#define _TRACE_H

#ifndef TRACE

#define TRACE_INIT
#define TRACE_BEGIN
#define TRACE_BEGIN_FMT
#define TRACE_END

#else

#include <sys/time.h> /* clock, gettimeofday, CLOCKS_PER_SEC */
#include <stdio.h>

#ifndef CLOCKS_PER_SEC
/* (on linux CLOCKS_PER_SEC is undefined, see man 3 clock */
#define CLOCKS_PER_SEC 1000000
#endif

#define TRACE_INIT \
	struct timeval _TRACE_TIME_START; \
	struct timeval _TRACE_TIME_END; \
	double _TRACE_TIME_DIFF; \
	clock_t _TRACE_CLOCK_START; \
	clock_t _TRACE_CLOCK_END; \
	double _TRACE_CLOCK_DIFF;
/*
 * Leaving the variadic args off only works with LLVM/GCC
 * see http://gcc.gnu.org/onlinedocs/cpp/Variadic-Macros.html
 *
 */
#define TRACE_BEGIN_FMT(fmt, ...) \
	gettimeofday(&_TRACE_TIME_START, NULL); \
	_TRACE_CLOCK_START = clock(); \
	puts("~~> TRACE BEGIN"); \
	printf("~~? " fmt, __VA_ARGS__);

#define TRACE_BEGIN(message) \
	gettimeofday(&_TRACE_TIME_START, NULL); \
	_TRACE_CLOCK_START = clock(); \
	puts("~~> TRACE BEGIN"); \
	puts("~~? " message);

#define TRACE_END \
	gettimeofday(&_TRACE_TIME_END, NULL); \
	_TRACE_TIME_DIFF = (_TRACE_TIME_END.tv_sec - _TRACE_TIME_START.tv_sec) * 1000.0; \
	_TRACE_TIME_DIFF += (_TRACE_TIME_END.tv_usec - _TRACE_TIME_START.tv_usec) / 1000.0; \
	_TRACE_CLOCK_END = clock(); \
	_TRACE_CLOCK_DIFF = (double)(_TRACE_CLOCK_END - _TRACE_CLOCK_START) / CLOCKS_PER_SEC * 1000.0; \
	printf("<~~ TRACE END (real: %.4f ms, cpu: %.4f ms)\n", _TRACE_TIME_DIFF, _TRACE_CLOCK_DIFF);


#endif

#endif
