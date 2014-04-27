# cache evaluation of path
L := $(LOCAL_DIR)

PROGRAMS += $(L)/mpd-server
#TESTS += $(L)/test_jsonrpc

# overwrite CFLAGS per object
#$(L)/.o : CFLAGS += ...

# -- programs --
$(L)/mpd-server_FLAGS := -I$(L) -lyajl -lmpdclient -lev -lpthread
$(L)/mpd-server_OBJS := $(L)/mpd-server.o \
	$(LIBCX_DIR)/rpc/rpc.o \
	$(LIBCX_DIR)/rpc/jsrpc.o \
	$(LIBCX_DIR)/rpc/jsrpc_yajl.o \
	$(L)/mpd_service.o \
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
