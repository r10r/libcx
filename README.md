## About

Template for new c projects.


### Create modules

### Use modules

[GIT submodules](http://git-scm.com/book/de/Git-Tools-Submodule)

Include module as submodule

	git submodule add git@github.com:r10r/libcx-list.git modules/cx-list

Include module in header

	#include <cx/list.h>
	
## Code Conventions

* Typedef all functions that are used as parameters / struct members (IDE support)
* Typedef all structs


Structs: typedef my_type_t MyType


### Functions

Function typedefs: F_MyFunction
Function typedefs parameters: f_my_function
Function typedefs members: f_my_function

### Questions

### TODO

* Protect internal API in header files (for proper encapsulation).
* Make each module self contained (using a module makefile **module.mk**)
* Do not generate dependency files when running **clean**

## Makefile

* [Recursive Make Considered Harmful](http://miller.emu.id.au/pmiller/books/rmch/)

* Use immediate evaluation `:=` unless you knowingly want deferred evaluation.
* Record results and include them in the Makefile.
* Automatic include dependency tracking to rely on interface changes in *.h files.
* Complete DAG right
* Use includes, as opening files is relatively inexpensive
* A whole project make only needs to interpret a single Makefile (build the DAG once)
* Do full project builds to spot errors/dependencies early

### Tests

* Test includes should be relative to BASE_DIR ?
* Include test includes / sources in every module ?

### Debugging

	$(warning $(D))

* use `--debug` parameter

### Resources
* http://mad-scientist.net/make/
