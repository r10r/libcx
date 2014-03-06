default: all

# [ settings ]
# ============
# Variables that might be overwritten by commandline or module.mk
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

$(info Using dependency script: $(OBJECT_DEPENDENCY_SCRIPT))

# look for include files in each of the modules
# use -isystem ?
#CFLAGS += $(patsubst %,-I%,$(MODULES))

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
# Templates for programs and tests.
# You can set objects,CFLAGS,LDFLAGS per program/test.
define PROGRAM_template
$(1): $$($(1)_OBJS) $(1).c
	$$(CC) $$(CFLAGS) $$(LDFLAGS) $$($(1)_FLAGS) \
	-o $(1) $$($(1)_OBJS)

OBJS += $$($(1)_OBJS)
endef
$(foreach prog,$(PROGRAMS),$(eval $(call PROGRAM_template,$(prog))))

# The test template executes the generated program afterwards (runs the test).
define TEST_template
$(1): $$($(1)_OBJS) $(1).c
	$$(CC) $$(CFLAGS) $$(LDFLAGS) $$($(1)_FLAGS) \
	-o $(1) $$($(1)_OBJS) && $(1)

OBJS += $$($(1)_OBJS)
endef
$(foreach prog,$(TESTS),$(eval $(call TEST_template,$(prog))))


# [ dependency tracking ]
# =======================
# Calculate dependencies for object.
# Regenerate dependency makefile when object is updated.
%.o.mk:
	$(OBJECT_DEPENDENCY_SCRIPT) $*.c $(CFLAGS) $*.c > $@
	
# Include a dependency file per object.
# The dependency file is created automatically by the rule above.
include $(OBJS:=.mk)
	

# [ uncrustify ]
# ==============
# Keep your code nice and shiny ;)
.uncrustify: $(SRC)
	uncrustify --mtime -c $(UNCRUSTIFY_CONFIG) --replace $? &&\
	touch .uncrustify
	
uncrustify: .uncrustify;


# [ lcov ]
# ========
$(LCOV_DIR)/index.html: $(LCOV_INFO_FILE)
	genhtml --ignore-errors source $(LCOV_INFO_FILE) \
	--output-directory $(LCOV_DIR)

$(LCOV_INFO_FILE): $(TESTS)
	lcov --capture --directory $(BASE_DIR) \
	--no-external --output-file $(LCOV_INFO_FILE) \
	--gcov-tool $(GCOV_TOOL)
	
lcov: $(LCOV_DIR)/index.html;

# Memchecking is essential but unfortunately slow.
# TODO Create a file for each test executable to
# indicate the last successful valgrind run 
# Maybe custom memory profiling by overwriting 
# malloc/free with xmalloc/xfree is a feasible option.
# [ valgrind ]
# ============
#.PHONY
#valgrind:
#	valgrind --error-exitcode=1 --leak-check=full ./libcx-list/test_list


# [ clean ]
# =========
# Should remove all generated artifacts
ARTIFACTS := $(OBJS) $(OBJS:=.mk) \
	$(PROGRAMS) $(TESTS) \
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
