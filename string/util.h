#include <string.h>
#include <libcx/base/base.h>

unsigned
str_cnt(const char* s, const char* sep);

unsigned
str_split(char* s, const char* sep, const char*** arr);

unsigned
str_arr_intersect(const char** a, unsigned num_a, const char** b, unsigned num_b, const char*** c);

unsigned
str_arr_diff(const char** a, unsigned num_a, const char** b, const char*** c);

char*
str_arr_join(const char** a, const char* sep);

int
str_arr_len(const char** a);
