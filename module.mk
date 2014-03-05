# cache evaluation of path
L := $(LOCAL_DIR)

#OBJS += $(L)/
PROGRAMS += $(L)/echo-client $(L)/echo-server
#TESTS += $(L)/

# -- executables --
echo-client_OBJS := $(L)/echo-client.o $(L)/client.o 
echo-client_FLAGS := -lev 

echo-server_OBJS := $(L)/echo-server.o $(L)/server.o 
echo-server_FLAGS := -lev 

# -- tests -- 
TEST_FLAGS := -Wall -w -g -I$(BASE_DIR)
TEST_OBJS := $(BASE_DIR)/test/unity.o

#_OBJS := $(TEST_OBJS)
#_FLAGS := $(TEST_FLAGS)
