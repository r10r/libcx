# cache evaluation of path
L := $(LOCAL_DIR)

OBJS += $(L)/list.o
#PROGRAMS += $(L)/
TESTS += $(L)/test_list

# -- executables --
#_OBJS := 
#_FLAGS := 

# -- tests -- 
TEST_FLAGS := -Wall -w -g -I$(BASE_DIR)
TEST_OBJS := $(BASE_DIR)/test/unity.o

test_list_OBJS := $(TEST_OBJS) $(L)/test_list.o $(L)/list.o
test_list_FLAGS := $(TEST_FLAGS)
