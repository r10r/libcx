LIBCX_DIR := $(LOCAL_DIR)/libcx

CC := clang-3.4
CFLAGS += -Weverything \
	-I$(LIBCX_DIR) \
	-I$(BASE_DIR)

# profile release
#CFLAGS += -Os

# disable debug statements
#CFLAGS +=  -DNDEBUG

# profile development
CFLAGS += -gdwarf-2 -g -O0 -fno-inline -DTRACE -DPROFILE --coverage

LDFLAGS += -L/usr/local/lib/llvm-3.4/usr/lib \
	-lpcap

MODULES := src

# to explicitly ignore unused parameters use a macro
# #define UNUSED(x) (void)(x)
CFLAGS +=  -Werror -Wall -pedantic \
	-Wno-error=unused-parameter \
	-Wno-error=unused-function \
	-Wno-error=unused-variable \
	-Wno-error=unused-value \
	-Wno-error=padded \
	-Wno-error=incompatible-pointer-types-discards-qualifiers
	
CFLAGS += -Wno-error=disabled-macro-expansion # curl recursive macro expansion magic 
	
# ignore unity errors
CFLAGS += \
	-Wno-error=unused-macros \
	-Wno-error=sign-conversion \
	-Wno-error=cast-align \
	-Wno-error=float-equal
