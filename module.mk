# cache evaluation of path
L := $(LOCAL_DIR)

#OBJS += $(L)/
PROGRAMS += $(L)/echo-client $(L)/echo-server \
	$(L)/echo-server-threaded $(L)/echo-client-threaded
#TESTS += $(L)/

# -- executables --
$(L)/echo-client_OBJS := $(L)/echo-client.o $(L)/client.o 
$(L)/echo-client_FLAGS := -lev

$(L)/echo-server_OBJS := $(L)/echo-server.o $(L)/server.o 
$(L)/echo-server_FLAGS := -lev 

$(L)/echo-server-threaded_OBJS := $(L)/echo-server-threaded.o \
	$(L)/server.o $(BASE_DIR)/libcx-workqueue/pool.o $(BASE_DIR)/libcx-list/list.o 
$(L)/echo-server-threaded_FLAGS := -lev 

$(L)/echo-client-threaded_OBJS := $(L)/echo-client-threaded.o $(L)/client.o
#$(L)/echo-client-threaded_FLAGS := 

# -- tests -- 
TEST_FLAGS := -Wall -w -g -I$(BASE_DIR)
TEST_OBJS := $(BASE_DIR)/libcx-base/unity.o

#_OBJS := $(TEST_OBJS)
#_FLAGS := $(TEST_FLAGS)
