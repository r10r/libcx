#include <stdio.h>      /* stdin, fopen */
//#include <string.h> /* memset */
#include <stdlib.h>     /* calloc */
#include <limits.h>
#include <stdbool.h>

#include "list/list.h"

/* FIXME does not detect */

typedef enum cx_alloc_method_t
{
	ALLOC,
	FREE
} Method;

#define PATH_LENGTH_MAX (1024 + 1)      /* OSX HFS+ */
#define FUNC_LENGTH_MAX (63 + 1)        /*  C standard, section 5.2.4.1 */

typedef struct cx_allocation_t MemoryAllocation;

struct cx_allocation_t
{
	char* file;
	char* func;
	Method method;
	long address;
	int line;
	size_t size;
};

#define MIN_CONVERSIONS 5
#define MAX_CONVERSIONS 6
#define LOG_FORMAT  "%*s %li %c %s %s %d %zu\n"
#define ALLOC_TOKEN '+'
#define FREE_TOKEN '-'

#define OK 1
#define ERROR 0
#define FINISHED -1

static inline MemoryAllocation*
MemoryAllocation_dup(MemoryAllocation* a)
{
	MemoryAllocation* dup = calloc(1, sizeof(MemoryAllocation));

	memcpy(dup, a, sizeof(MemoryAllocation));
	dup->file = strdup(a->file);
	dup->func = strdup(a->func);
	return dup;
}

static inline MemoryAllocation*
MemoryAllocation_new()
{
	MemoryAllocation* a = calloc(1, sizeof(MemoryAllocation));

	a->file = calloc(PATH_LENGTH_MAX, sizeof(char));
	a->func = calloc(FUNC_LENGTH_MAX, sizeof(char));
	return a;
}

static inline void
MemoryAllocation_free(MemoryAllocation* a)
{
	free(a->file);
	free(a->func);
	free(a);
}

static inline int
read_line(MemoryAllocation* allocation, FILE* stream)
{
	char method_token;

	int result = fscanf(stream, LOG_FORMAT,
			    &allocation->address,
			    &method_token,
			    allocation->file,
			    allocation->func,
			    &allocation->line,
			    &allocation->size);

	if (result == 0) /* error: input available but no conversions assigned */
	{
		fprintf(stderr, "ERROR: Input available but no conversions assigned\n");
		return ERROR;
	}

	if (result == EOF)
		return FINISHED;

	if (result >= MIN_CONVERSIONS && result <= MAX_CONVERSIONS)
	{
		if (method_token == ALLOC_TOKEN)
			allocation->method = ALLOC;
		else if (method_token == FREE_TOKEN)
			allocation->method = FREE;
		else
		{
			fprintf(stderr, "ERROR: Invalid method_token token: %c\n", method_token);
			return ERROR;
		}
	}
	else
	{
		fprintf(stderr, "ERROR: Invalid number of conversions %d\n", result);
		return ERROR;
	}

	return OK;
}

static inline void
log_error(MemoryAllocation* alloc, const char* message)
{
	fprintf(stderr, "LEAK[0x%lx] %zu bytes %s line:%d (%s): %s\n",
		alloc->address,
		alloc->size,
		alloc->file,
		alloc->line,
		alloc->func,
		message);
}

static inline void
MemoryAllocation_delete_alloc(List* allocations, MemoryAllocation* alloc_free)
{
	Node* head = allocations->first;
	Node* current = NULL;

	int delete = 0;
	unsigned int index = 0;

	LIST_EACH_WITH_INDEX(head, current, index)
	{
		MemoryAllocation* alloc = (MemoryAllocation*)current->data;

		if (alloc->address == alloc_free->address)
		{
			delete = 1;
			break;
		}
	}

	if (delete)
		List_delete(allocations, index);
}

static inline void
free_memory_allocation(void* value)
{
	MemoryAllocation_free((MemoryAllocation*)value);
}

int
main()
{
	List* allocations = List_new();

	allocations->f_node_data_free = free_memory_allocation;
	MemoryAllocation* a = MemoryAllocation_new();

	int result = 0;
	int count = 0;
	int exit_code = 1;

//	FILE* file = fopen(argv[1], "r");

	while (true)
	{
//		result = read_line(a, file);
		result = read_line(a, stdin);
		if (result == OK)
		{
			if (a->method == ALLOC)
			{
				MemoryAllocation* dup = MemoryAllocation_dup(a);
				List_push(allocations, dup);
			}
			else if (a->method == FREE)
				MemoryAllocation_delete_alloc(allocations, a);
		}
		else if (result == FINISHED)
		{
			if (allocations->length == 0)
				exit_code = 0;
			else
			{
				fprintf(stderr, "detected %lu memory leaks\n"
					"================\n", allocations->length);

				Node* head = allocations->first;
				Node* node;
				LIST_EACH(head, node)
				{
					MemoryAllocation* unfree = (MemoryAllocation*)node->data;

					log_error(unfree, "was not freed");
				}
			}

			break;
		}
		else if (result == ERROR)
		{
			fprintf(stderr, "ERROR: processing line[%d]\n", count);
			break;
		}
		count++;
	}

	MemoryAllocation_free(a);
	List_free(allocations);
//	fclose(file);
	return exit_code;
}