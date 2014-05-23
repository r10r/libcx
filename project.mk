LIBCX_DIR := $(LOCAL_DIR)
TEST_RUNNER := $(SMAKE_DIR)/bin/mleaks-testrunner.sh

# FIXME 
# * do not run decover.sh / coverage script  when coverage is disabled
# * create coverage script for gcc

# set a compiler
compiler ?= clang
CC_GCC := gcc
CC_CLANG := clang

MODULES := base \
	sandbox \
	string \
	list \
	umtp \
	socket

# TODO ignore parameter/functions/values/variables with a macro
# #define UNUSED(x) (void)(x)

CFLAGS += -Werror -Wall -pedantic -std=c99 -D_POSIX_C_SOURCE -D_C99_SOURCE\
	-Wno-error=padded \
	-Wno-error=cast-align \
	-Wno-error=switch-enum
	
# [ profile specific CFLAGS ] 
# ===========================
# TODO split up into separate included files with
# arch/system/compiler specific settings

# default profile
profile ?= debug

# TODO Check whether autoconf is of use here ?

ifeq ($(profile),release)
	CFLAGS += -Os
else ifeq ($(profile),debug)
	CFLAGS += -O0 -g -fno-inline --coverage -D_CX_DEBUG -D_CX_TRACE -D_CX_DEBUG_MEM
	ifeq ($(OS),Darwin)
		CFLAGS += -gdwarf-2
		# when clang is installed through homebrew
		LDFLAGS += -L/usr/local/lib/llvm-3.4/usr/lib
	endif
	ifeq ($(OS),Linux)
		ifeq ($(ARCH),armv6l)
			ifeq ($(compiler),clang)
				# no coverage support for clang on armv6l
				CFLAGS := $(filter-out --coverage,$(CFLAGS))
			endif
		endif
	endif
endif

ifeq ($(OS),Darwin)
	CFLAGS += -D_DARWIN_C_SOURCE
endif

# [ compiler specific settings ]

ifeq ($(compiler),gcc)
# flags only supported by gcc
	CC := $(CC_GCC)
else ifeq ($(compiler),clang)
	CC := $(CC_CLANG)
# flags only supported by clang
# disabled-macro-expansion: 
# * libcurl uses recursive macro expansion magic to match parameters
# * linux declares stdout/stdin/stderr recursive
	CFLAGS += -Weverything \
		-Wno-error=disabled-macro-expansion \
		-Wno-error=incompatible-pointer-types-discards-qualifiers

endif

# [ os / arch specific settings ]

ifeq ($(OS),Linux)
	ifeq ($(compiler),gcc)
		CFLAGS += -D_GNU_SOURCE=1
	endif
	
	ifeq ($(ARCH),armv6l)
		# reduce maximum string size to 100Mb
		CFLAGS += -DSTRING_MAX_LENGTH=104857600  
	endif
endif

TEST_OBJS := $(LIBCX_DIR)/base/unity.o

# ignore unity errors
UNITY_FLAGS += \
 	-Wno-unused-macros \
	-Wno-sign-conversion \
	-Wno-float-equal \
	-Wno-missing-field-initializers \
	-Wno-missing-braces \
	-Wno-unused-variable \
	-Wno-cast-align

$(LIBCX_DIR)/base/unity.o: CFLAGS += $(UNITY_FLAGS)
