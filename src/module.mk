# alias to current dir (without defered evaluation)
L := $(LOCAL_DIR)

# OBJS +=
PROGRAMS += $(L)/server

# rules for each program
server_OBJS := $(L)/server.o \
	$(BASE_DIR)/modules/libcx-list/list.o
	
#server_CFLAGS = -g -Isystem$(D)
server_LDFLAGS := -lev
