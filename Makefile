default: all

# [ settings ]
# ============
# Variables that might be overwritten by commandline or module.mk
BASE_DIR := $(realpath $(PWD))
UNCRUSTIFY_CONFIG := $(BASE_DIR)/uncrustify.cfg
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
# FIXME running 'clean' triggers module makefile creation
-include $(BASE_DIR)/module.mk
include $(patsubst %,%/module.mk,$(MODULES))

$(info Using dependency script: $(OBJECT_DEPENDENCY_SCRIPT))

# look for include files in each of the modules
# use -isystem ?
#CFLAGS += $(patsubst %,-I%,$(MODULES))


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

PROGRAMS += $(TESTS)
$(foreach prog,$(PROGRAMS),$(eval $(call PROGRAM_template,$(prog))))

# [ dependency tracking ]
# =======================
# Calculate dependencies for object.
# Regenerate dependency makefile when object is updated.
%.o.mk:
	$(OBJECT_DEPENDENCY_SCRIPT) $*.c $(CFLAGS) $*.c > $@
	
# Include a dependency file per object.
# The dependency file is created automatically by the rule above.
include $(OBJS:=.mk)

# after all module/dependency makefiles have been included
# it's time to remove duplicate objects/sources
OBJS := $(sort $(OBJS))
SRC := $(sort $(SRC))


all: format $(OBJS) $(PROGRAMS) test decover


# [ format ]
# ==========
# Keeps your code nice and shiny ;)
%.unc-backup~: %
	uncrustify -c $(UNCRUSTIFY_CONFIG) --replace $*
	
format: $(SRC:=.unc-backup~);


# [ tests ]
# =========
%.testresult: % 
	$(TEST_RUNNER) $*

test: $(TESTS:=.testresult);


# [ gcov ]
# ========
# Tests must execute to generate the GCOV files.
# All objects not linked to any test must be build 
# to include include them in the coverage report.
# TODO create textmate scheme for GCOV file format 

.PRECIOUS: %.gcno
%.gcno: $(OBJS) $(TESTS);

.PRECIOUS: %.cov
%.cov: %.gcno
	$(COVERAGE_TOOL) $*.o

# generate all coverage files
cov: $(OBJS:.o=.cov);

# generate a simple coverage report summary
decover: cov
	$(COVERAGE_REPORT_TOOL)


# [ clean ]
# =========
# Should remove all generated artifacts
ARTIFACTS := $(OBJS) $(OBJS:=.mk) \
	$(PROGRAMS) \
	$(wildcard $(MODULES:=/*.testresult)) \
	$(wildcard $(MODULES:=/*.testlog)) \
	$(wildcard $(MODULES:=/*.gcda)) \
	$(wildcard $(MODULES:=/*.gcno)) \
	$(wildcard $(MODULES:=/*.o.mk)) \
	$(wildcard $(MODULES:=/*.cov)) \
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
