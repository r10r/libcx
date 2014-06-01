# cache evaluation of path
L := $(LOCAL_DIR)

PROGRAMS += $(L)/echo-server
#TESTS += $(L)/test_jsonrpc

# overwrite CFLAGS per object
#$(L)/.o : CFLAGS += ...

# -- programs --
$(L)/echo-server_FLAGS := -I$(L) -lev -lpthread
$(L)/echo-server_OBJS := $(L)/echo-server.o \
	$(L)/base64_enc.o \
	$(L)/sha1.o \
	$(L)/websocket.o \
	$(L)/ws_connection.o \
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
