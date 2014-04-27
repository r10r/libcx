# cache evaluation of path
L := $(LOCAL_DIR)
	
TESTS += $(L)/test_rpc \
	$(L)/test_jsonrpc

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
