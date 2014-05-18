# cache evaluation of path
L := $(LOCAL_DIR)

#TESTS += 	$(L)/test_rpc \
#	$(L)/test_jansson \
#	$(L)/test_jsonrpc

TESTS += $(L)/test_rpc_example_service \
	$(L)/test_rpc_json_jansson \
	$(L)/test_jansson
	
#PROGRAMS += 

# overwrite CFLAGS per object
#$(L)/.o : CFLAGS += ...

# -- programs --

# -- tests -- 
$(L)/test_rpc_example_service_FLAGS := -ljansson
$(L)/test_rpc_example_service_OBJS := $(TEST_OBJS) \
	$(L)/test_rpc_example_service.o \
	$(L)/rpc.o \
	$(L)/rpc_example_service.o \
	$(LIBCX_DIR)/base/errno.o

$(L)/test_jansson_FLAGS := -ljansson
$(L)/test_jansson_OBJS := $(TEST_OBJS) \
	$(L)/test_jansson.o \
	$(LIBCX_DIR)/string/string.o

$(L)/test_rpc_json_jansson_FLAGS := -ljansson
$(L)/test_rpc_json_jansson_OBJS := $(TEST_OBJS) \
	$(L)/test_rpc_json_jansson.o \
	$(L)/rpc.o \
	$(L)/rpc_json_jansson.o \
	$(L)/rpc_example_service.o \
	$(LIBCX_DIR)/base/errno.o
