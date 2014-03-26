# cache evaluation of path
L := $(LOCAL_DIR)

# 	$(L)/tcp-echo-server \

#OBJS += $(L)/
PROGRAMS += $(L)/echo-client \
	$(L)/echo-server \
	$(L)/echo-client-threaded
	
# $(L)/echo-server-threaded \
	
TESTS += $(L)/test_queue

# -- executables --
$(L)/echo-client_OBJS := $(L)/echo-client.o \
	$(L)/client.o 

$(L)/echo-client-threaded_OBJS := $(L)/echo-client-threaded.o \
	$(L)/client.o
$(L)/echo-client-threaded_FLAGS := -lpthread

$(L)/echo-server_FLAGS := -lev  -lpthread
$(L)/echo-server_OBJS := $(L)/echo-server.o \
	$(L)/server.o \
	$(L)/unix_server.o \
	$(L)/connection.o \
	$(L)/worker.o \
	$(L)/request.o \
	$(LIBCX_DIR)/libcx-list/list.o \
	$(LIBCX_DIR)/libcx-umtp/message.o \
	$(LIBCX_DIR)/libcx-umtp/parser.o \
	$(LIBCX_DIR)/libcx-umtp/message_fsm.o \
	$(LIBCX_DIR)/libcx-string/string.o \
	$(LIBCX_DIR)/libcx-string/pair.o
	
$(L)/tcp-echo-server_FLAGS := -lev  -lpthread
$(L)/tcp-echo-server_OBJS := $(L)/tcp-echo-server.o \
	$(L)/server.o \
	$(L)/tcp_server.o \
	$(L)/connection.o \
	$(L)/worker.o \
	$(L)/request.o \
	$(LIBCX_DIR)/libcx-list/list.o \
	$(LIBCX_DIR)/libcx-umtp/message.o \
	$(LIBCX_DIR)/libcx-umtp/parser.o \
	$(LIBCX_DIR)/libcx-umtp/message_fsm.o \
	$(LIBCX_DIR)/libcx-string/string.o \
	$(LIBCX_DIR)/libcx-string/pair.o

# -- tests -- 
$(L)/test_queue_FLAGS := -lpthread
$(L)/test_queue_OBJS := $(TEST_OBJS) \
	$(L)/test_queue.o \
	$(L)/queue.o \
	$(LIBCX_DIR)/libcx-list/list.o
