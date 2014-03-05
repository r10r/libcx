# cache evaluation of path
L := $(LOCAL_DIR)

#OBJS += $(L)/
PROGRAMS += $(L)/echo-client $(L)/echo-server
#TESTS += $(L)/

# -- executables --
echo-client_OBJS := $(L)/echo-client.o $(L)/client.o 
echo-client_LDFLAGS := -lev 

echo-server_OBJS := $(L)/echo-server.o $(L)/server.o 
echo-server_LDFLAGS := -lev 

# -- tests -- 
TEST_CFLAGS := -Wall -w -g -I$(BASE_DIR) -DTEST -DTRACE
TEST_OBJS := $(BASE_DIR)/test/unity.o

#_OBJS := $(TEST_OBJS)
#_CFLAGS := $(TEST_CFLAGS)
