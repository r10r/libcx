# cache evaluation of path
L := $(LOCAL_DIR)

#OBJS += $(L)/
PROGRAMS += $(L)/echo-client \
	$(L)/test-server \
	$(L)/echo-server \
	$(L)/echo-client-threaded
	
# $(L)/echo-server-threaded \
	
#TESTS += $(L)/

# -- executables --
$(L)/echo-client_OBJS := $(L)/echo-client.o $(L)/client.o 
$(L)/echo-client_FLAGS := -lev

$(L)/echo-server_OBJS := $(L)/echo-server.o \
	$(L)/server.o \
	$(L)/unix_server.o \
	$(L)/connection.o \
	$(L)/worker.o \
	$(L)/request.o \
	$(BASE_DIR)/libcx-list/list.o \
	$(BASE_DIR)/libcx-umtp/message.o \
	$(BASE_DIR)/libcx-umtp/message_fsm.o \
	$(BASE_DIR)/libcx-string/string.o \
	$(BASE_DIR)/libcx-string/pair.o
$(L)/echo-server_FLAGS := -lev 

#$(L)/echo-server-threaded_OBJS := $(L)/echo-server-threaded.o \
#	$(L)/server.o $(BASE_DIR)/libcx-workqueue/pool.o $(BASE_DIR)/libcx-list/list.o 
#$(L)/echo-server-threaded_FLAGS := -lev 

$(L)/echo-client-threaded_OBJS := $(L)/echo-client-threaded.o $(L)/client.o
#$(L)/echo-client-threaded_FLAGS := 

$(L)/test-server_OBJS := $(L)/test-server.o \
	$(L)/server.o \
	$(L)/unix_server.o \
	$(L)/worker.o \
	$(L)/request.o \
	$(L)/connection.o \
	$(BASE_DIR)/libcx-list/list.o \
	$(BASE_DIR)/libcx-list/queue.o \
	$(BASE_DIR)/libcx-string/string.o \
	$(BASE_DIR)/libcx-string/pair.o \
	$(BASE_DIR)/libcx-umtp/message.o \
	$(BASE_DIR)/libcx-umtp/message_fsm.o
	
	 
$(L)/test-server_FLAGS := -lev 

# -- tests -- 
TEST_FLAGS := -Wall -w -g -I$(BASE_DIR)
TEST_OBJS := $(BASE_DIR)/libcx-base/unity.o

#_OBJS := $(TEST_OBJS)
#_FLAGS := $(TEST_FLAGS)
