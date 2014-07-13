#include "util.h"

unsigned
str_cnt(const char* s, const char* token)
{
	if (!s || !token)
		return 0;

	unsigned count = 0;

	size_t token_len = strlen(token);
	const char* start = s;
	const char* end = s + strlen(s);

	while (start + token_len <= end)
	{
		if (strncmp(start, token, token_len) == 0)
		{
			start += token_len;
			count++;
		}
		else
			start++;
	}
	return count;
}

unsigned
str_split(char* s, const char* sep, const char*** ptr)
{
	if (s == NULL)
		return 0;

	unsigned num_sep = str_cnt(s, sep);

	unsigned num_tokens = num_sep + 1;
	unsigned num_elems = num_tokens + 1 /* NULL terminator */;
	*ptr = cx_alloc((size_t)(num_elems * sizeof(char*)));
	char** arr = (char**)*ptr;
	arr[num_tokens] = NULL;

	int index = 0;
	char* strtok_state;
	char* token = strtok_r(s, sep, &strtok_state);
	while (token)
	{
		arr[index++] = token;
		token = strtok_r(NULL, sep, &strtok_state);
	}

	return num_tokens;
}

/*
 * @param arr array of char arrays terminated with NULL
 */
unsigned
str_arr_intersect(const char** a, unsigned num_a, const char** b, unsigned num_b, const char*** c)
{
	unsigned max_num_elems = (num_a > num_b) ? num_a : num_b;

	*c = cx_alloc(max_num_elems * sizeof(char*));
	unsigned index_c = 0;

	int index_a = 0;
	const char* elem_a = NULL;
	while ((elem_a = a[index_a++]) != NULL)
	{
		int index_b = 0;
		const char* elem_b = NULL;
		while ((elem_b = b[index_b++]) != NULL)
		{
			if (strcmp(elem_a, elem_b) == 0)
			{
				(*c)[index_c++] = elem_a;
			}
		}
	}
	(*c)[index_c++] = NULL;
	return index_c;
}

/*
 * @param a source array of char arrays terminated with NULL
 * @param b array of char arrays that will be removed from a, terminated with NULL
 * @param c pointer to target array, terminated with NULL
 */
unsigned
str_arr_diff(const char** a, unsigned num_a, const char** b, const char*** c)
{
	unsigned max_num_elems = num_a;

	*c = cx_alloc(max_num_elems * sizeof(char*));
	unsigned index_c = 0;

	int index_a = 0;
	const char* elem_a = NULL;
	while ((elem_a = a[index_a++]) != NULL)
	{
		int index_b = 0;
		const char* elem_b = NULL;
		int matched = 0;
		while ((elem_b = b[index_b++]) != NULL)
		{
			if (strcmp(elem_a, elem_b) == 0)
			{
				matched = 1;
				break;
			}
		}
		if (!matched)
			(*c)[index_c++] = elem_a;
	}
	(*c)[index_c] = NULL;
	return index_c;
}

/*
 * @param arr array of char arrays terminated with NULL
 */
char*
str_arr_join(const char** arr, const char* sep)
{
	/* calculate string length */
	unsigned len = 0;

	const char* elem = NULL;
	int elem_idx = 0;

	while ((elem = arr[elem_idx]) != NULL)
	{
		len += strlen(elem);
		elem_idx++;
	}

	if (elem_idx == 0)
		return NULL;

	len += (unsigned)(elem_idx - 1) * strlen(sep);
	char* s = cx_alloc(len + 1 * sizeof(char));
	s[len] = '\0';

	/* copy value,separator ... */
	char* pos = s;
	elem_idx = 0;
	while ((elem = arr[elem_idx]) != NULL)
	{
		pos = strcat(pos, elem);
		if (arr[elem_idx + 1] != NULL)
			pos = strcat(pos, sep);
		elem_idx++;
	}

	return s;
}

/* calculates length of NULL terminated string array */
int
str_arr_len(const char** a)
{
	int len = 0;

	while (a[len++]);
	return len;
}
