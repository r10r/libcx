# cache evaluation of path
L := $(LOCAL_DIR)

TESTS += $(L)/test_string \
	$(L)/test_stringbuffer \
	$(L)/test_string_pair

# -- executables --
#_OBJS := 
#_FLAGS := 

# -- tests -- 
$(L)/test_string_OBJS := $(TEST_OBJS) \
	 $(L)/test_string.o \
	 $(L)/string.o
	 
$(L)/test_stringbuffer_OBJS := $(TEST_OBJS) \
	 $(L)/test_stringbuffer.o \
	 $(L)/string.o

$(L)/test_string_pair_OBJS := $(TEST_OBJS) \
	 $(L)/test_string_pair.o \
	 $(L)/string.o \
	 $(L)/pair.o