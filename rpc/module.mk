# cache evaluation of path
L := $(LOCAL_DIR)

TESTS += 	$(L)/test_rpc \
	$(L)/test_jansson \
	$(L)/test_jsonrpc 

#PROGRAMS += 

# overwrite CFLAGS per object
#$(L)/.o : CFLAGS += ...

# -- programs --

# -- tests -- 
$(L)/test_rpc_FLAGS := -ljansson -lyajl
$(L)/test_rpc_OBJS := $(TEST_OBJS) \
	$(L)/test_rpc.o \
	$(L)/rpc.o \
	$(L)/echo_service.o \
	$(L)/jsrpc.o \
	$(L)/jsrpc_yajl.o \
	$(LIBCX_DIR)/string/string.o \
	$(LIBCX_DIR)/socket/request.o \
	$(LIBCX_DIR)/socket/response.o

$(L)/test_jansson_FLAGS := -ljansson
$(L)/test_jansson_OBJS := $(TEST_OBJS) \
	$(L)/test_jansson.o \
	$(LIBCX_DIR)/string/string.o
	
$(L)/test_jsonrpc_FLAGS := -lyajl
$(L)/test_jsonrpc_OBJS := $(TEST_OBJS) \
	$(L)/test_jsonrpc.o \
	$(L)/rpc.o \
	$(L)/jsrpc.o \
	$(L)/jsrpc_yajl.o \
	$(L)/echo_service.o \
	$(LIBCX_DIR)/socket/request.o \
	$(LIBCX_DIR)/socket/response.o \
	$(LIBCX_DIR)/string/string.o \
	$(LIBCX_DIR)/list/list.o
