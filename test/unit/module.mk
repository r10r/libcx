# cache evaluation of path
L := $(LOCAL_DIR)

TEST_CFLAGS := -Wall -w -g -I$(L)/unity \
	-DTEST -DTRACE \
	-DTEST_BIO_CERT_PATH="\"$(TOPDIR)/test/cert.pem\"" \
	-DTEST_VERIFY_CERT_PATH="\"$(TOPDIR)/test/cert.pem\""

TESTS += $(L)/test_list

test_list_OBJS := $(L)/unity/unity.o $(L)/test_list.o \
	$(BASE_DIR)/modules/libcx-list/list.o

test_list_CFLAGS := $(TEST_CFLAGS)
