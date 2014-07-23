# cache evaluation of path
L := $(LOCAL_DIR)

#OBJS += $(L)/
PROGRAMS += $(L)/echo-server \
	$(L)/echo-client-threaded \
	$(L)/echo-server-send-threaded \
	$(L)/server-simple-accept \
	$(L)/server-fork-accept \
	
#TESTS += $(L)/test_

# -- executables --

# don't fail with GCC-4.4 (on linux ?)
# see [libev - gcc aliasing warnings](http://lists.schmorp.de/pipermail/libev/2010q1/000943.html)
ifeq ($(compiler),gcc)
$(L)/server.o : CFLAGS += -Wno-error=strict-aliasing
$(L)/connection.o : CFLAGS += -Wno-error=strict-aliasing
$(L)/connection_worker.o : CFLAGS += -Wno-error=strict-aliasing
endif

$(L)/echo-client-threaded_FLAGS := -lpthread
$(L)/echo-client-threaded_OBJS := $(L)/echo-client-threaded.o \
	$(L)/client.o \
	$(L)/socket.o \
	$(L)/socket_tcp.o \
	$(L)/socket_unix.o \
	$(LIBCX_DIR)/string/string.o

$(L)/echo-server_FLAGS := -lev  -lpthread
$(L)/echo-server_OBJS := $(L)/echo-server.o \
	$(L)/server.o \
	$(L)/server_unix.o \
	$(L)/server_tcp.o \
	$(L)/socket.o \
	$(L)/socket_unix.o \
	$(L)/socket_tcp.o \
	$(L)/connection.o \
	$(L)/worker.o \
	$(L)/connection_worker.o \
	$(L)/request.o \
	$(L)/response.o \
	$(L)/echo_connection.o \
	$(LIBCX_DIR)/base/uid.o \
	$(LIBCX_DIR)/list/list.o \
	$(LIBCX_DIR)/list/queue.o \
	$(LIBCX_DIR)/string/string.o \
	$(LIBCX_DIR)/string/pair.o
	
$(L)/echo-server-send-threaded_FLAGS := -lev  -lpthread
$(L)/echo-server-send-threaded_OBJS := $(L)/echo-server-send-threaded.o \
	$(L)/server.o \
	$(L)/server_unix.o \
	$(L)/server_tcp.o \
	$(L)/socket.o \
	$(L)/socket_unix.o \
	$(L)/socket_tcp.o \
	$(L)/connection.o \
	$(L)/worker.o \
	$(L)/connection_worker.o \
	$(L)/request.o \
	$(L)/response.o \
	$(L)/echo_connection.o \
	$(LIBCX_DIR)/base/uid.o \
	$(LIBCX_DIR)/list/list.o \
	$(LIBCX_DIR)/list/queue.o \
	$(LIBCX_DIR)/string/string.o \
	$(LIBCX_DIR)/string/pair.o

$(L)/server-simple-accept_FLAGS := -lpthread
$(L)/server-simple-accept_OBJS := $(L)/server-simple-accept.o \
	$(L)/socket.o \
	$(L)/socket_unix.o
	
$(L)/server-fork-accept_FLAGS := -lpthread
$(L)/server-fork-accept_OBJS := $(L)/server-fork-accept.o \
	$(L)/socket.o \
	$(L)/socket_unix.o \
	$(L)/socket_tcp.o

# -- tests -- 

