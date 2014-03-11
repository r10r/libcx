# cache evaluation of path
L := $(LOCAL_DIR)

#CFLAGS += -D_LIST_DISABLE_LOCKING

TESTS += $(L)/test_list $(L)/test_array

# -- executables --
#_OBJS := 
#_FLAGS := 

# -- tests -- 
TEST_FLAGS := -Wall -w -g -I$(BASE_DIR)
TEST_OBJS := $(BASE_DIR)/libcx-base/unity.o

$(L)/test_list_OBJS := $(TEST_OBJS) $(L)/test_list.o $(L)/list.o
$(L)/test_list_FLAGS := $(TEST_FLAGS)

$(L)/test_array_OBJS := $(TEST_OBJS) $(L)/test_array.o $(L)/array.o
$(L)/test_array_FLAGS := $(TEST_FLAGS)