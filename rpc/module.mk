# cache evaluation of path
L := $(LOCAL_DIR)

#PROGRAMS += $(L)/jsrpc-example \
#	$(L)/echo-server
	
TESTS += $(L)/test_batch
#	$(L)/test_rpc \
#	$(L)/test_jsonrpc \
	

# overwrite CFLAGS per object
#$(L)/.o : CFLAGS += ...

# -- programs --
$(L)/jsrpc-example_FLAGS := -lyajl
$(L)/jsrpc-example_OBJS := $(L)/jsrpc-example.o \
	$(L)/rpc.o \
	$(L)/jsrpc.o \
	$(L)/jsrpc_yajl.o \
	$(L)/hello_service.o \
	$(LIBCX_DIR)/string/string.o

# -- tests -- 

$(L)/test_rpc_OBJS := $(TEST_OBJS) \
	$(L)/test_rpc.o \
	$(L)/rpc.o \
	$(LIBCX_DIR)/string/string.o
	
$(L)/test_jsonrpc_FLAGS := -lyajl
$(L)/test_jsonrpc_OBJS := $(TEST_OBJS) \
	$(L)/test_jsonrpc.o \
	$(L)/rpc.o \
	$(LIBCX_DIR)/string/string.o
	
$(L)/test_batch_FLAGS := -lyajl -lmpdclient
$(L)/test_batch_OBJS := $(TEST_OBJS) \
	$(L)/test_batch.o \
	$(L)/rpc.o \
	$(L)/jsrpc.o \
	$(L)/jsrpc_yajl.o \
	$(L)/mpd/mpd_service.o \
	$(LIBCX_DIR)/string/string.o

$(L)/echo-server_FLAGS := -I$(L) -lyajl -lev -lpthread
$(L)/echo-server_OBJS := $(L)/echo-server.o \
	$(L)/echo_service.o \
	$(L)/rpc.o \
	$(L)/jsrpc.o \
	$(L)/jsrpc_yajl.o \
	$(LIBCX_DIR)/socket/server.o \
	$(LIBCX_DIR)/socket/server_unix.o \
	$(LIBCX_DIR)/socket/server_tcp.o \
	$(LIBCX_DIR)/socket/socket.o \
	$(LIBCX_DIR)/socket/socket_unix.o \
	$(LIBCX_DIR)/socket/socket_tcp.o \
	$(LIBCX_DIR)/socket/connection.o \
	$(LIBCX_DIR)/socket/worker.o \
	$(LIBCX_DIR)/socket/worker_unix.o \
	$(LIBCX_DIR)/socket/request.o \
	$(LIBCX_DIR)/list/list.o \
	$(LIBCX_DIR)/string/string.o \
	$(LIBCX_DIR)/string/pair.o
