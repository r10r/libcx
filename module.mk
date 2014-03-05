MODULES := libcx-list libcx-workqueue libcx-socket-unix

CFLAGS += --coverage -I$(BASE_DIR) -DTRACE \
	-Wall -Wextra -Werror -pedantic \
	-Wno-error=unused-parameter -Wno-error=unused-function
	
LDFLAGS += --coverage