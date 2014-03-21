# TODO split up into a module-dev.mk / module-release.mk

COVERAGE_TOOL := $(BASE_DIR)/coverage.sh
COVERAGE_REPORT_TOOL := $(BASE_DIR)/decover.sh
TEST_RUNNER := $(BASE_DIR)/valgrind-testrunner.sh

CC := clang-3.4
CFLAGS += -Weverything \
	-Wno-error=incompatible-pointer-types-discards-qualifiers # requires downcasting to void * pointer

LDFLAGS += -L/usr/local/lib/llvm-3.4/usr/lib

MODULES := libcx-base libcx-sandbox \
	libcx-string libcx-list libcx-umtp libcx-socket-unix

CFLAGS += -I$(BASE_DIR) \
	-Werror -Wall -pedantic \
	-Wno-error=unused-parameter \
	-Wno-error=unused-function \
	-Wno-error=unused-variable \
	-Wno-error=unused-value \
	-Wno-error=padded \

# ignore unity errors
CFLAGS += \
	-Wno-error=unused-macros \
	-Wno-error=sign-conversion \
	-Wno-error=cast-align \
	-Wno-error=float-equal


# profile release
#CFLAGS += -Os

# disable debug statements
#CFLAGS +=  -DNDEBUG

# profile development
CFLAGS += -gdwarf-2 -g -O0 -fno-inline -DTRACE -DPROFILE --coverage
