# cache evaluation of path
L := $(LOCAL_DIR)

#CFLAGS += -D_LIST_DISABLE_LOCKING

TESTS += $(L)/test_list \
	$(L)/test_array \


# -- executables --
#_OBJS := 
#_FLAGS := 

# -- tests -- 
# disable warnings for passing 'const char[x]' to 'void*' parameter
$(L)/test_list_FLAGS := -Wno-incompatible-pointer-types-discards-qualifiers
$(L)/test_list_OBJS := $(TEST_OBJS) \
	$(L)/test_list.o \
	$(L)/list.o

$(L)/test_array_FLAGS := -Wno-incompatible-pointer-types-discards-qualifiers
$(L)/test_array_OBJS := $(TEST_OBJS) \
	$(L)/test_array.o \
	$(L)/array.o
