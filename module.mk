# cache evaluation of path
L := $(LOCAL_DIR)

#OBJS += $(L)/
PROGRAMS += $(L)/echo-client $(L)/echo-server
#TESTS += $(L)/

# -- executables --
$(L)/echo-client_OBJS := $(L)/echo-client.o $(L)/client.o 
$(L)/echo-client_FLAGS := -lev 

$(L)/echo-server_OBJS := $(L)/echo-server.o $(L)/server.o 
$(L)/echo-server_FLAGS := -lev 

# -- tests -- 
TEST_FLAGS := -Wall -w -g -I$(BASE_DIR)
TEST_OBJS := $(BASE_DIR)/test/unity.o

#_OBJS := $(TEST_OBJS)
#_FLAGS := $(TEST_FLAGS)
