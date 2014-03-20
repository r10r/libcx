# cache evaluation of path
L := $(LOCAL_DIR)

#OBJS += $(L)/
PROGRAMS += $(L)/message-parser
TESTS += $(L)/test_message

# -- executables --
$(L)/message-parser_OBJS := $(L)/message-parser.o \
	$(L)/message.o \
	$(L)/message_fsm.o \
	$(BASE_DIR)/libcx-string/string.o \
	$(BASE_DIR)/libcx-string/pair.o \
	$(BASE_DIR)/libcx-list/list.o
#$(L)/message-parser_FLAGS := $(L)/

# -- tests -- 
TEST_FLAGS := -Wall -w -g -I$(BASE_DIR)
TEST_OBJS := $(BASE_DIR)/libcx-base/unity.o

$(L)/test_message_OBJS := $(TEST_OBJS) \
	$(L)/test_message.o \
	$(L)/message.o \
	$(L)/message_fsm.o \
	$(BASE_DIR)/libcx-string/string.o \
	$(BASE_DIR)/libcx-string/pair.o \
	$(BASE_DIR)/libcx-list/list.o
#$(L)/test_message_FLAGS := $(TEST_FLAGS)
