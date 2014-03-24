# cache evaluation of path
L := $(LOCAL_DIR)

TESTS += $(L)/test_debug $(L)/test_base

# -- executables --
#_OBJS := 
#_FLAGS := 

# -- tests -- 
$(L)/test_debug_OBJS := $(TEST_OBJS) $(L)/test_debug.o
$(L)/test_debug_FLAGS := $(TEST_FLAGS)

$(L)/test_base_OBJS := $(TEST_OBJS) $(L)/test_base.o
$(L)/test_base_FLAGS := $(TEST_FLAGS)