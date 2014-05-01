# cache evaluation of path
L := $(LOCAL_DIR)

#OBJS += $(L)/
PROGRAMS += $(L)/echo-server \
	$(L)/echo-client-threaded
	
TESTS += $(L)/test_queue

# -- executables --

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
	$(L)/worker_unix.o \
	$(L)/request.o \
	$(LIBCX_DIR)/list/list.o \
	$(LIBCX_DIR)/string/string.o \
	$(LIBCX_DIR)/string/pair.o

# -- tests -- 
$(L)/test_queue_FLAGS := -lpthread
$(L)/test_queue_OBJS := $(TEST_OBJS) \
	$(L)/test_queue.o \
	$(L)/queue.o \
	$(LIBCX_DIR)/list/list.o
