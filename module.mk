MODULES := libcx-list libcx-workqueue libcx-socket-unix

CFLAGS += --coverage -I$(BASE_DIR) -DTRACE
LDFLAGS += --coverage