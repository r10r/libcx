# TODO split up into a module-dev.mk / module-release.mk

LIBCX_DIR := $(LOCAL_DIR)

CC := clang
CFLAGS += -Weverything -Werror -Wall -pedantic

ifeq ($(profile),release)
CFLAGS += -Os -DNDEBUG -DNTRACE
else
CFLAGS += -gdwarf-2 -g -O0 -fno-inline --coverage
endif

ifeq ($(OS),Darwin)
# only when clang is installed through homebrew
LDFLAGS += -L/usr/local/lib/llvm-3.4/usr/lib
endif

MODULES := base \
	sandbox \
	string \
	list \
	umtp \
	socket

# to explicitly ignore unused parameters use a macro
# #define UNUSED(x) (void)(x)

CFLAGS += -Wno-error=unused-parameter \
	-Wno-error=unused-function \
	-Wno-error=unused-variable \
	-Wno-error=unused-value \
	-Wno-error=padded \
	-Wno-error=cast-align \
	-Wno-error=incompatible-pointer-types-discards-qualifiers
	
# curl uses recursive macro expansion magic to match parameters
# linux declares stdout/stdin/stderr recursive
CFLAGS += -Wno-error=disabled-macro-expansion 

TEST_OBJS := $(LIBCX_DIR)/libcx-base/unity.o \
	$(LIBCX_DIR)/libcx-base/xmalloc.o 

# ignore unity errors
UNITY_FLAGS += \
 	-Wno-unused-macros \
	-Wno-sign-conversion \
	-Wno-float-equal \
	-Wno-missing-field-initializers \
	-Wno-missing-braces

$(LIBCX_DIR)/libcx-base/unity.o: CFLAGS += $(UNITY_FLAGS)