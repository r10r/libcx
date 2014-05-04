# cache evaluation of path
L := $(LOCAL_DIR)

PROGRAMS += $(L)/jsrpc-example
TESTS += $(L)/test_jsonrpc

# overwrite CFLAGS per object
#$(L)/.o : CFLAGS += ...

# -- programs --
$(L)/jsrpc-example_FLAGS := -lyajl
$(L)/jsrpc-example_OBJS := $(L)/jsrpc-example.o \
	$(L)/jsrpc_yajl.o \
	$(L)/rpc.o \
	$(L)/hello_service.o \
	$(LIBCX_DIR)/string/string.o

# -- tests -- 

$(L)/test_jsonrpc_FLAGS := -lyajl
$(L)/test_jsonrpc_OBJS := $(TEST_OBJS) \
	$(L)/test_jsonrpc.o \
	$(L)/rpc.o \
	$(LIBCX_DIR)/string/string.o
