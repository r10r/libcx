# cache evaluation of path
L := $(LOCAL_DIR)

#OBJS += $(L)/
PROGRAMS += $(L)/message-parser
TESTS += $(L)/test_message \
	$(L)/test_body_parser

# -- executables --
$(L)/message-parser_OBJS := $(L)/message-parser.o \
	$(L)/message.o \
	$(L)/message_parser.o \
	$(L)/message_fsm.o \
	$(L)/parser.o \
	$(LIBCX_DIR)/libcx-string/string.o \
	$(LIBCX_DIR)/libcx-string/pair.o \
	$(LIBCX_DIR)/libcx-list/list.o

# -- tests -- 

$(L)/test_message_OBJS := $(TEST_OBJS) \
	$(L)/test_message.o \
	$(L)/parser.o \
	$(L)/message_parser.o \
	$(L)/message_fsm.o \
	$(L)/message.o \
	$(LIBCX_DIR)/libcx-string/string.o \
	$(LIBCX_DIR)/libcx-string/pair.o \
	$(LIBCX_DIR)/libcx-list/list.o

$(L)/test_body_parser_OBJS := $(TEST_OBJS) \
	$(L)/test_body_parser.o \
	$(L)/parser.o \
	$(L)/body_fsm.o \
	$(L)/message_parser.o \
	$(L)/message_fsm.o \
	$(L)/message.o \
	$(LIBCX_DIR)/libcx-string/string.o \
	$(LIBCX_DIR)/libcx-string/pair.o \
	$(LIBCX_DIR)/libcx-list/list.o