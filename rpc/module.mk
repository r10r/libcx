# cache evaluation of path
L := $(LOCAL_DIR)

TESTS += $(L)/test_params \
 $(L)/test_jsonrpc \
 $(L)/test_rpc

# overwrite CFLAGS per object
#$(L)/.o : CFLAGS += ...

# -- programs --

# -- tests -- 

$(L)/test_rpc_OBJS := $(TEST_OBJS) \
	$(L)/test_rpc.o \
	$(L)/rpc.o \
	$(LIBCX_DIR)/string/string.o
	
$(L)/test_jsonrpc_FLAGS := -lyajl
$(L)/test_jsonrpc_OBJS := $(TEST_OBJS) \
	$(L)/test_jsonrpc.o \
	$(L)/rpc.o \
	$(L)/jsrpc.o \
	$(L)/jsrpc_yajl.o \
	$(L)/echo_service.o \
	$(LIBCX_DIR)/string/string.o

$(L)/test_params_FLAGS := -lyajl
$(L)/test_params_OBJS := $(TEST_OBJS) \
	$(L)/test_params.o