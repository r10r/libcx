#include <stdio.h>      /* stdin, fopen */
#include <string.h>     /* strdup */
#include <stdlib.h>     /* calloc */
#include <limits.h>
#include <stdbool.h>

#undef _CX_DEBUG_MEM
#include "../list/list.h"

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
#define LOG_FORMAT  "%*s %li %c %s %s %d %zu"
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
		XDBG("ERROR: Input available but no conversions assigned");
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
			XFDBG("ERROR: Invalid method_token token: %c", method_token);
			return ERROR;
		}
	}
	else
	{
		XFDBG("ERROR: Invalid number of conversions %d", result);
		return ERROR;
	}

	return OK;
}

static inline void
log_error(MemoryAllocation* alloc, const char* message)
{
	XFLOG("LEAK[0x%lx] %zu bytes %s line:%d (%s): %s",
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

static unsigned long
log_leaks(List* allocations)
{
	unsigned long num_leaks = allocations->length;

	if (num_leaks == 0)
	{
		XLOG("\nNo memory leaks.")
	}
	else
	{
		XFLOG("\n#%lu memory leaks\n"
		      "=======================", allocations->length);

		Node* head = allocations->first;
		Node* node;
		LIST_EACH(head, node)
		{
			MemoryAllocation* unfree = (MemoryAllocation*)node->data;

			log_error(unfree, "was not freed");
		}
	}
	XLOG("\n");
	return num_leaks;
}

static List* allocations;

static void
signal_log_leaks(int signum)
{
	UNUSED(signum);
	log_leaks(allocations);
}

static void
signal_clear_leaks(int signum)
{
	log_leaks(allocations);
	XFLOG("\nReceived signal %d. Clear list of unfreed memory allocations.\n", signum);
	List_free(allocations);
	allocations = List_new();
}

int
main()
{
	allocations = List_new();

	signal(SIGTRAP, signal_log_leaks);
	signal(SIGUSR1, signal_clear_leaks);

	setvbuf(stdin, NULL, _IOLBF, 0);

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
			{
				MemoryAllocation_delete_alloc(allocations, a);
			}
		}
		else if (result == FINISHED)
		{
			if (log_leaks(allocations) == 0)
				exit_code = 0;
			break;
		}
		else if (result == ERROR)
		{
			XFDBG("ERROR: processing line[%d]\n", count);
		}
		count++;
	}

	MemoryAllocation_free(a);
	List_free(allocations);
//	fclose(file);
	return exit_code;
}
