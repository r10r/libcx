# cache evaluation of path
L := $(LOCAL_DIR)

#CFLAGS += -D_LIST_DISABLE_LOCKING

TESTS += $(L)/test_buffer

# -- executables --
#_OBJS := 
#_FLAGS := 

# -- tests -- 
TEST_FLAGS := -Wall -w -g -I$(BASE_DIR)
TEST_OBJS := $(BASE_DIR)/libcx-base/unity.o

$(L)/test_buffer_OBJS := $(TEST_OBJS) $(L)/test_buffer.o $(L)/buffer.o
$(L)/test_buffer_FLAGS := $(TEST_FLAGS)
