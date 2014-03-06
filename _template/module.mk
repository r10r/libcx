# cache evaluation of path
L := $(LOCAL_DIR)

#OBJS += $(L)/
#PROGRAMS += $(L)/
#TESTS += $(L)/

# -- executables --
#$(L)/_OBJS := $(L)/
#$(L)/_FLAGS := $(L)/

# -- tests -- 
TEST_FLAGS := -Wall -w -g -I$(BASE_DIR)
TEST_OBJS := $(BASE_DIR)/libcx-base/unity.o

#$(L)/_OBJS := $(TEST_OBJS)
#$(L)/_FLAGS := $(TEST_FLAGS)
