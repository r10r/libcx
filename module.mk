# TODO split up into a module-dev.mk / module-release.mk

COVERAGE_TOOL := $(BASE_DIR)/coverage.sh
COVERAGE_REPORT_TOOL := $(BASE_DIR)/decover.sh
TEST_RUNNER := $(BASE_DIR)/valgrind-testrunner.sh

CC := clang-3.4
CFLAGS += -Weverything \
	-Wno-error=incompatible-pointer-types-discards-qualifiers
LDFLAGS += -L/usr/local/lib/llvm-3.4/usr/lib

MODULES := libcx-base libcx-sandbox libcx-list libcx-socket-unix \
	libcx-workqueue libcx-string #libcx-umtp

CFLAGS += -I$(BASE_DIR) -DTRACE \
	-Wall -pedantic  -coverage \
	-Wno-error=unused-parameter -Wno-error=unused-function \
	-Wno-error=unused-variable -Wno-error=padded \
	-gdwarf-2 -g -O0 -fno-inline  -DPROFILE -DNDEBUG \
	-Wno-error=sign-conversion -Wno-error=cast-align -Wno-error=float-equal# unity only
