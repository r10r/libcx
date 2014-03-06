MODULES := libcx-base libcx-list libcx-socket-unix libcx-workqueue 

CFLAGS += --coverage -I$(BASE_DIR) -DTRACE \
	-Wall -Wextra -Werror -pedantic \
	-Wno-error=unused-parameter -Wno-error=unused-function
	
LDFLAGS += --coverage