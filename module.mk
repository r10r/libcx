# cache evaluation of path
L := $(LOCAL_DIR)

#CFLAGS += -D_LIST_DISABLE_LOCKING

TESTS += $(L)/test_sizeof

# -- executables --
#_OBJS := 
#_FLAGS := 

# -- tests -- 
$(L)/test_sizeof_OBJS := $(TEST_OBJS) $(L)/test_sizeof.o
$(L)/test_sizeof_FLAGS := $(TEST_FLAGS)