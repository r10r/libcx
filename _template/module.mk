# cache evaluation of path
L := $(LOCAL_DIR)

#OBJS += $(L)/
#PROGRAMS += $(L)/
#TESTS += $(L)/

# -- executables --
#_OBJS := 
#_CFLAGS := 

# -- tests -- 
TEST_CFLAGS := -Wall -w -g -I$(BASE_DIR) -DTEST -DTRACE
TEST_OBJS := $(BASE_DIR)/test/unity.o

#_OBJS := $(TEST_OBJS)
#_CFLAGS := $(TEST_CFLAGS)
