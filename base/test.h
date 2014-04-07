#ifndef _CX_TEST_H
#define _CX_TEST_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include "unity.h"
#include "debug.h"

extern void
setUp(void);
extern void
tearDown(void);

void
setUp()
{
}

void
tearDown()
{
}

#define RUN(func) \
	RUN_TEST(func, __LINE__);

#define TEST_BEGIN \
	UnityBegin();

/* @cast unity has a unsigned int counter for test failures,
 * but it's unlikely that the number of test failures is > INT_MAX */
#define TEST_END \
	return (int)UnityEnd();

/* @see http://stackoverflow.com/questions/5459868/c-preprocessor-concatenate-int-to-string */
#define STR_HELPER(x) #x
#define S(x) STR_HELPER(x)

#endif
