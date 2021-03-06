# cache evaluation of path
L := $(LOCAL_DIR)

TESTS += $(L)/test_debug \
	$(L)/test_base \
	$(L)/test_pthread

PROGRAMS += $(L)/mleaks

# -- executables --
#_OBJS := 
#_FLAGS := 

# -- tests -- 
$(L)/test_debug_OBJS := $(TEST_OBJS) \
	$(L)/test_debug.o

$(L)/test_base_OBJS := $(TEST_OBJS) \
	$(L)/test_base.o \
	$(L)/uid.o
	
$(L)/test_pthread_FLAGS := -lpthread
$(L)/test_pthread.o: CFLAGS += -Wno-error=unused-macros
$(L)/test_pthread_OBJS := $(TEST_OBJS) \
	$(L)/test_pthread.o
	
$(L)/mleaks_OBJS := $(L)/mleaks.o \
	$(LIBCX_DIR)/list/list.o