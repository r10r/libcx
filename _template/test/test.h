#ifndef _TEST_H
#define _TEST_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include "unity.h"
#include "trace.h"

#define NOSETUP \
	void setUp(){}; \
	void tearDown(){};

#define RUN(func) \
	RUN_TEST(func, __LINE__);

#define TEST_BEGIN \
	UnityBegin();

#define TEST_END \
	return UnityEnd();

/* @see http://stackoverflow.com/questions/5459868/c-preprocessor-concatenate-int-to-string */
#define STR_HELPER(x) #x
#define S(x) STR_HELPER(x)

#endif
