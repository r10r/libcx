# cache evaluation of path
L := $(LOCAL_DIR)

#CFLAGS += -D_LIST_DISABLE_LOCKING

TESTS += $(L)/test_string

# -- executables --
#_OBJS := 
#_FLAGS := 

# -- tests -- 
TEST_FLAGS := -Wall -w -g -I$(BASE_DIR)
TEST_OBJS := $(BASE_DIR)/libcx-base/unity.o

$(L)/test_string_OBJS := $(TEST_OBJS) \
	 $(BASE_DIR)/libcx-base/xmalloc.o $(L)/string.o $(L)/test_string.o
$(L)/test_string_FLAGS := $(TEST_FLAGS)