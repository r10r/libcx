# cache evaluation of path
L := $(LOCAL_DIR)

#CFLAGS += -D_LIST_DISABLE_LOCKING

TESTS += $(L)/test_list \
	$(L)/test_concurrent_list \
	$(L)/test_queue \
	$(L)/test_queue2


# -- executables --
#_OBJS := 
#_FLAGS := 

# -- tests -- 
# disable warnings for passing 'const char[x]' to 'void*' parameter
$(L)/test_list_FLAGS := -Wno-incompatible-pointer-types-discards-qualifiers
$(L)/test_list_OBJS := $(TEST_OBJS) \
	$(L)/test_list.o \
	$(L)/list.o

$(L)/test_concurrent_list_FLAGS := -Wno-incompatible-pointer-types-discards-qualifiers
$(L)/test_concurrent_list_OBJS := $(TEST_OBJS) \
	$(L)/test_concurrent_list.o \
	$(L)/list.o

$(L)/test_queue_FLAGS := -lpthread
$(L)/test_queue_OBJS := $(TEST_OBJS) \
	$(L)/test_queue.o \
	$(L)/queue.o \
	$(LIBCX_DIR)/list/list.o
	
$(L)/test_queue2_FLAGS := -lpthread
$(L)/test_queue2_OBJS := $(TEST_OBJS) \
	$(L)/test_queue2.o \
	$(L)/queue.o \
	$(LIBCX_DIR)/list/list.o