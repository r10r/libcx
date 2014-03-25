# TODO split up into a module-dev.mk / module-release.mk

LIBCX_DIR := $(LOCAL_DIR)

CC := clang
CFLAGS += -Weverything -Werror -Wall -pedantic \
	-Wno-error=incompatible-pointer-types-discards-qualifiers # requires downcasting to void * pointer

# profile release
#CFLAGS += -Os

# disable debug statements
#CFLAGS +=  -DNDEBUG

# profile development
CFLAGS += -gdwarf-2 -g -O0 -fno-inline -DTRACE -DPROFILE --coverage

ifeq ($(OS),Darwin)
# only when clang is installed through homebrew
LDFLAGS += -L/usr/local/lib/llvm-3.4/usr/lib
endif

MODULES := libcx-base \
	libcx-sandbox \
	libcx-string \
	libcx-list \
	libcx-umtp \
	libcx-socket-unix

# to explicitly ignore unused parameters use a macro
# #define UNUSED(x) (void)(x)

CFLAGS += -I$(LIBCX_DIR) \
	-Wno-error=unused-parameter \
	-Wno-error=unused-function \
	-Wno-error=unused-variable \
	-Wno-error=unused-value \
	-Wno-error=padded \
	-Wno-error=cast-align

TEST_OBJS := $(LIBCX_DIR)/libcx-base/unity.o \
	$(LIBCX_DIR)/libcx-base/xmalloc.o 

# ignore unity errors
UNITY_FLAGS += \
 	-Wno-unused-macros \
	-Wno-sign-conversion \
	-Wno-float-equal \
	-Wno-missing-field-initializers

$(LIBCX_DIR)/libcx-base/unity.o: CFLAGS += $(UNITY_FLAGS)