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