
extern static const char* MODULE_ERROR_STR;

#ifdef PTHREADS

#else

static int module_errno;

#define Error_message \
	MODULE_ERROR_STR[module_errno]

#define Error_clear() \
	module_errno = 0

// TODO warn if error will be overriden
#define Error_set(code) \
	module_errno = code

#define Error_get() \
	module_errno

#endif
