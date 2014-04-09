# cache evaluation of path
L := $(LOCAL_DIR)

PROGRAMS += $(L)/jsrpc-example
TESTS += $(L)/test_jsonrpc

# overwrite CFLAGS per object
#$(L)/.o : CFLAGS += ...

# -- programs --
$(L)/jsrpc-example_FLAGS := -lyajl
$(L)/jsrpc-example_OBJS := $(L)/jsrpc-example.o \
	$(LIBCX_DIR)/list/list.o \
	$(L)/jsrpc_yajl.o

# -- tests -- 

$(L)/test_jsonrpc_FLAGS := -lyajl
$(L)/test_jsonrpc_OBJS := $(TEST_OBJS) \
	$(L)/test_jsonrpc.o
