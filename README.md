# About

Multiple dynamic string implementations.

## String

* + very small overhead (8 bytes) / continous memory allocation
* single dereferencation
* - growing the string requires pointer reassignment
* backed by char array
* dynamic lifetime
* length information is the number of used bytes.

## StringPointer

* a point to a char array with length information
* collect references to existing char arrays for efficient processing
* e.g allocating one memory region for all collected arrays to join them ...

## StringBuffer

* grows / shrinks automatically
* points to (dynamic) String
* additionaly has information how much memory was allocated

## Resources

* [Char array vs Char Pointer in C](http://stackoverflow.com/questions/10186765/char-array-vs-char-pointer-in-c)
* [Arrays and Pointers](http://c-faq.com/aryptr/index.html)
* [Dynamic Strings in C](http://locklessinc.com/articles/dynamic_cstrings/)
* [Redis Hacked Strings](http://redis.io/topics/internals-sds)