MODULES := libcx-base libcx-list libcx-socket-unix libcx-workqueue libcx-buffer

CFLAGS += --coverage -I$(BASE_DIR) -DTRACE \
	-Wall -Wextra -Werror -pedantic \
	-Wno-error=unused-parameter -Wno-error=unused-function \
	-gdwarf-2 -g -O0 -fno-inline
	
LDFLAGS += --coverage