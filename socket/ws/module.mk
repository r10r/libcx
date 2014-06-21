# cache evaluation of path
L := $(LOCAL_DIR)

PROGRAMS += $(L)/echo-server
TESTS += $(L)/test_handshake

# overwrite CFLAGS per object
#$(L)/.o : CFLAGS += ...

# -- programs --
$(L)/echo-server_FLAGS := -I$(L) -lev -lpthread
$(L)/echo-server_OBJS := $(L)/echo-server.o \
	$(L)/base64_enc.o \
	$(L)/sha1.o \
	$(L)/handshake.o \
	$(L)/frame.o \
	$(L)/util.o \
	$(L)/ws_connection.o \
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
	$(LIBCX_DIR)/socket/worker.o \
	$(LIBCX_DIR)/socket/connection_worker.o \
	$(LIBCX_DIR)/socket/request.o \
	$(LIBCX_DIR)/list/list.o \
	$(LIBCX_DIR)/string/string.o \
	$(LIBCX_DIR)/string/pair.o

#$(L)/test_handshake_FLAGS := -I$(L) -lev -lpthread
$(L)/test_handshake_OBJS := $(TEST_OBJS) \
	$(L)/test_handshake.o \
	$(L)/handshake.o \
	$(L)/sha1.o \
	$(L)/util.o \
	$(L)/base64_enc.o \
	$(LIBCX_DIR)/umtp/parser.o \
	$(LIBCX_DIR)/umtp/message_parser.o \
	$(LIBCX_DIR)/umtp/message_fsm.o \
	$(LIBCX_DIR)/umtp/message.o \
	$(LIBCX_DIR)/string/string.o \
	$(LIBCX_DIR)/string/pair.o \
	$(LIBCX_DIR)/list/list.o

