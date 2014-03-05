# cache evaluation of path
L := $(LOCAL_DIR)

#OBJS += $(L)/
#PROGRAMS += $(L)/
#TESTS += $(L)/

# -- executables --
#_OBJS := $(L)/
#_FLAGS := $(L)/

# -- tests -- 
TEST_FLAGS := -Wall -w -g -I$(BASE_DIR)
TEST_OBJS := $(BASE_DIR)/test/unity.o

#_OBJS := $(TEST_OBJS)
#_FLAGS := $(TEST_FLAGS)
