# cache evaluation of path
L := $(LOCAL_DIR)

TESTS += $(L)/test_list

# -- executables --
#_OBJS := 
#_FLAGS := 

# -- tests -- 
TEST_FLAGS := -Wall -w -g -I$(BASE_DIR)
TEST_OBJS := $(BASE_DIR)/test/unity.o

$(L)/test_list_OBJS := $(TEST_OBJS) $(L)/test_list.o $(L)/list.o
$(L)/test_list_FLAGS := $(TEST_FLAGS)
