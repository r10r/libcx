## About

Small (static) C library.

## Code Conventions

* Typedef all functions that are used as parameters / struct members (better IDE support)
* Typedef all structs

Structs: typedef my_type_t MyType

### Functions

Function typedefs: F_MyFunction
Function typedefs parameters: f_my_function
Function typedefs members: f_my_function

### TODO

* Protect internal API in header files (for proper encapsulation).
* Make each module self contained (using a module makefile **module.mk**)
* Do not generate dependency files when running **clean**

### Share

* use CCAN http://ccodearchive.net/ ?

## Performance / Testing

### > 1024 File selectors

* set limit with ulimit -n 1024

* http://pod.tst.eu/http://cvs.schmorp.de/libev/ev.pod#OS_X_AND_DARWIN_BUGS
* Backend on OSX must be either use EVBACKEND_POLL / EVBACKEND_KQUEUE
* _DARWIN_UNLIMITED_SELECT doesn't work on OSX 10.9.x ?

* build libev with debugging enabled
	
		export CFLAGS="-gdwarf-2 -g -O0 -fno-inline"
		./configure
		make 
		make install


### Benchmark 
* http://stackoverflow.com/questions/2332741/what-is-the-theoretical-maximum-number-of-open-tcp-connections-that-a-modern-lin
* http://danielmendel.github.io/blog/2013/04/07/benchmarkers-beware-the-ephemeral-port-limit/
* http://www.metabrew.com/article/a-million-user-comet-application-with-mochiweb-part-1


### Linux 


#### Arch Linux

    install base-devel ragel clang libev4 libmpdclient yajl uncrustify

### Aliasing Issues


* Use `-fno-strict-aliasing` to force the compiler not to make any aliasing assumptions
  to avoid aliasing issues.
  
* http://lists.schmorp.de/pipermail/libev/2010q1/000943.html
* http://blog.worldofcoding.com/2010/02/solving-gcc-44-strict-aliasing-problems.html
* http://dbp-consulting.com/tutorials/StrictAliasing.html

## Benchmark

* http://danielmendel.github.io/blog/2013/04/07/benchmarkers-beware-the-ephemeral-port-limit/
* look at http://www.wangafu.net/~nickm/libevent-book/Ref6_bufferevent.html

## Resources

* http://frank.harvard.edu/~coldwell/toolchain/bigendian.html
* http://www.greenend.org.uk/rjk/tech/inline.html

## TODO

### Building 

* build server on mips (with ulibc)

### Safety / Memory Management

    grep -Rn free $(find . -name *.c)  | grep -v _free
    grep -Rn malloc $(find . -name *.c)
    grep -Rn calloc $(find . -name *.c)

* print error when memory allocation faild (XFERROR)

* http://stackoverflow.com/questions/9071566/is-it-safe-to-use-realloc
* http://www.iso-9899.info/wiki/Why_not_realloc
*  http://www.iso-9899.info/wiki/SecString

* ensure the StringBuffer_value / string->value is never assigned to a variable 
  this variable will point to garbage when the string is reallocated.

### API

* Extract API from server implementations
* Pluggable connectors (raw TCP, Websockets, ...)

### Build / Makefile

* only run coverage if --coverage is in CFLAGS ;)
* generate coverage script for GCC
* use macro to mark parameters as unused 

#### Dependency Management

* library dependencies (better over package management ?)
* compiler flags / compiler settings

### Strings

* make it platform specific ?

* http://nadeausoftware.com/articles/2012/10/c_c_tip_how_detect_compiler_name_and_version_using_compiler_predefined_macros
* http://stackoverflow.com/questions/9071566/is-it-safe-to-use-realloc

### RPC

* expose libev RPC to lua ?
* routing in lua ?