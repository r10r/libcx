# cache evaluation of path
L := $(LOCAL_DIR)

#TESTS += 	$(L)/test_rpc \
#	$(L)/test_jansson \
#	$(L)/test_jsonrpc

TESTS += $(L)/test_rpc_example_service \
	$(L)/test_rpc_json_jansson \
	$(L)/test_jansson
	
PROGRAMS += $(L)/example-server

# overwrite CFLAGS per object
#$(L)/.o : CFLAGS += ...

# -- programs --

# -- tests -- 
$(L)/test_rpc_example_service_FLAGS := -ljansson
$(L)/test_rpc_example_service_OBJS := $(TEST_OBJS) \
	$(L)/test_rpc_example_service.o \
	$(L)/rpc.o \
	$(L)/rpc_example_service.o

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
	$(LIBCX_DIR)/socket/request.o \
	$(LIBCX_DIR)/base/uid.o

$(L)/example-server_FLAGS := -I$(L) -lev -lpthread -ljansson
$(L)/example-server_OBJS := $(L)/example-server.o \
	$(L)/rpc.o \
	$(L)/rpc_json_jansson.o \
	$(L)/rpc_example_service.o \
	$(LIBCX_DIR)/base/errno.o \
	$(LIBCX_DIR)/umtp/parser.o \
	$(LIBCX_DIR)/umtp/message_parser.o \
	$(LIBCX_DIR)/umtp/message_fsm.o \
	$(LIBCX_DIR)/umtp/message.o \
	$(LIBCX_DIR)/socket/server.o \
	$(LIBCX_DIR)/socket/server_unix.o \
	$(LIBCX_DIR)/socket/server_tcp.o \
	$(LIBCX_DIR)/socket/socket.o \
	$(LIBCX_DIR)/socket/socket_unix.o \
	$(LIBCX_DIR)/socket/socket_tcp.o \
	$(LIBCX_DIR)/socket/connection.o \
	$(LIBCX_DIR)/socket/echo_connection.o \
	$(LIBCX_DIR)/socket/worker.o \
	$(LIBCX_DIR)/socket/connection_worker.o \
	$(LIBCX_DIR)/socket/request.o \
	$(LIBCX_DIR)/socket/response.o \
	$(LIBCX_DIR)/list/list.o \
	$(LIBCX_DIR)/list/queue.o \
	$(LIBCX_DIR)/string/string.o \
	$(LIBCX_DIR)/string/pair.o \
	$(LIBCX_DIR)/base/uid.o
