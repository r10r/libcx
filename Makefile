default: all

# [ settings ]
# ============
# Variables that might be overwritten by commandline or Settings.mk
BASE_DIR := $(realpath $(PWD))
UNCRUSTIFY_CONFIG := $(BASE_DIR)/uncrustify.cfg
LCOV_INFO_FILE := coverage.info
LCOV_DIR := coverage
GCOV_TOOL := gcov
MODULES := 
OS := $(shell uname -s)
OBJECT_DEPENDENCY_SCRIPT := $(BASE_DIR)/depend.sh

# A macro that evaluates to the local directory path of an included Makefile.
LOCAL_DIR = $(realpath $(patsubst %/,%,$(dir $(lastword $(MAKEFILE_LIST)))))

# Each module may append to the following variables
# All objects that should be generated
OBJS :=
# All sources used for object generation (sources / includes ...)
SRC :=
# Generated executables
PROGRAMS :=
# Generated test executables
TESTS :=

# Include module makefiles module.mk.
-include $(BASE_DIR)/module.mk
include $(patsubst %,%/module.mk,$(MODULES))

# look for include files in each of the modules
# use -isystem ?
CFLAGS += $(patsubst %,-I%,$(MODULES))

all: $(OBJS) $(PROGRAMS) $(TESTS)

# [ os ]
# ======
# Set platform specific compiler/linker flags.
ifeq ($(DEBUG), true)
ifeq ($(OS),Darwin)
CFLAGS += -gdwarf-2 -g -O0 -fno-inline
endif
ifeq ($(OS),Linux)
CFLAGS += -g -O0 -fno-inline
endif
endif


# [ templates ]
# =============
# Templates for programms and tests.
# You can set objects,CFLAGS,LDFLAGS per program/test.
define PROGRAM_template
$(1): $$($(2)_OBJS) $(1).c
	$$(CC) $$(CFLAGS) $$($(2)_CFLAGS) $$(LDFLAGS) $$($(2)_LDFLAGS) \
	-o $(1) $$($(2)_OBJS)

OBJS += $$($(2)_OBJS)
endef
$(foreach prog,$(PROGRAMS),$(eval $(call PROGRAM_template,$(prog),$(notdir $(prog)))))

# The test template executes the generated program afterwards (runs the test).
define TEST_template
$(1): $$($(2)_OBJS) $(1).c
	$$(CC) $$(CFLAGS) $$($(2)_CFLAGS) $$(LDFLAGS) $$($(2)_LDFLAGS) \
	-o $(1) $$($(2)_OBJS) && $(1)

OBJS += $$($(2)_OBJS)
endef
$(foreach prog,$(TESTS),$(eval $(call TEST_template,$(prog),$(notdir $(prog)))))


# [ dependency tracking ]
# =======================
# calculate C include dependencies
%.o.mk: %.c
	$(OBJECT_DEPENDENCY_SCRIPT) $*.c $(CFLAGS) $*.c > $@
	
# include a dependency file per object
# when it does not exist the rule above automatically
# creates the dependency file
include $(OBJS:=.mk)
	

# [ uncrustify ]
# ==============
.uncrustify: $(SRC)
	uncrustify --mtime -c $(UNCRUSTIFY_CONFIG) --replace $? &&\
	touch .uncrustify
	
uncrustify: .uncrustify;


# [ gcov/lcov ]
# =============
$(LCOV_DIR)/index.html: $(LCOV_INFO_FILE)
	genhtml --ignore-errors source $(LCOV_INFO_FILE) \
	--output-directory $(LCOV_DIR)

$(LCOV_INFO_FILE): $(TESTS)
	lcov --capture --directory $(BASE_DIR) \
	--no-external --output-file $(LCOV_INFO_FILE) \
	--gcov-tool $(GCOV_TOOL)
	
coverage: $(LCOV_DIR)/index.html;


# [ clean ]
# =========
# Should remove all generated artifacts
ARTIFACTS := $(OBJS) $(OBJS:=.mk) \
	$(OBJS:.o=.gcda) $(OBJS:.o=.gcno) \
	$(LCOV_DIR) $(LCOV_INFO_FILE)
	
clean: 
	rm -rf $(ARTIFACTS)
	
	
# [ ragel ]
# ===========
# object generation rules
.PRECIOUS: %.c
%.c: %.rl
	ragel -L -G2 -o $@ $<	

%.dot: %.rl
	ragel -V -p -o $@ $<

%.png: %.dot
	dot -Tpng $< -o $@
	
	
# Optionally append new rules or overwrite existing ones.
-include Rules.mk
