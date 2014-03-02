BASE_DIR := $(realpath $(patsubst %/,%,$(dir $(lastword $(MAKEFILE_LIST)))))

# [ modules ]
# ===========
MODULES_DIR = $(BASE_DIR)/modules
MODULES = libcx-list libcx-threadpool

# add module includes to compiler system lookup path
CFLAGS += $(foreach m,$(MODULES),-isystem$(MODULES_DIR)/$(m)/include)

# builds the module the first time 
# an artifact of the module is referenced from a Makefile
$(MODULES_DIR)/%:
	$(MAKE) -C $(MODULES_DIR)/$(firstword $(subst /, ,$*))
