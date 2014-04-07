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

### Tests

* Test includes should be relative to BASE_DIR ?
* Include test includes / sources in every module ?

### Debugging

	$(warning $(D))

* use `--debug` parameter

### Resources
* http://mad-scientist.net/make/
