MODULES := libcx-base libcx-list libcx-socket-unix \
	libcx-workqueue libcx-buffer libcx-umtp

CFLAGS += --coverage -I$(BASE_DIR) -DTRACE \
	-Wall -Wextra -Werror -pedantic \
	-Wno-error=unused-parameter -Wno-error=unused-function \
	-Wno-error=unused-variable \
	-gdwarf-2 -g -O0 -fno-inline
	
LDFLAGS += --coverage