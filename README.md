## About

Template for new c projects.


### Create modules

### Use modules

[GIT submodules](http://git-scm.com/book/de/Git-Tools-Submodule)

Include module as submodule

	git submodule add git@github.com:r10r/libcx-list.git modules/cx-list

Include module in header

	#include <cx/list.h>

### TODO

* protect internal API / proper encapsulation