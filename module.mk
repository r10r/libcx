# cache evaluation of path
L := $(LOCAL_DIR)

TESTS += $(L)/test_debug

# -- executables --
#_OBJS := 
#_FLAGS := 

# -- tests -- 
TEST_FLAGS := -Wall -w -g -I$(BASE_DIR)
TEST_OBJS := $(BASE_DIR)/libcx-base/unity.o

$(L)/test_debug_OBJS := $(TEST_OBJS) $(L)/test_debug.o
$(L)/test_debug_FLAGS := $(TEST_FLAGS)
