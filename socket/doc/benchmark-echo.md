[ruben@rauscher test]$ du -s ~/Downloads/MacTeX.pkg
4618432	/Users/ruben/Downloads/MacTeX.pkg

2.2G

[ruben@rauscher test]$ time cat ~/Downloads/MacTeX.pkg > /dev/null

real	0m6.533s
user	0m0.014s
sys	0m1.070s


128 byte

[ruben@rauscher test]$ time cat ~/Downloads/MacTeX.pkg | nc -U /tmp/echo.sock > /dev/null

real	1m19.681s
user	0m2.208s
sys	0m52.757s

256 byte

[ruben@rauscher test]$ time cat ~/Downloads/MacTeX.pkg | nc -U /tmp/echo.sock > /dev/null

real	0m37.694s
user	0m2.132s
sys	0m33.331s


512 byte

[ruben@rauscher test]$ time cat ~/Downloads/MacTeX.pkg | nc -U /tmp/echo.sock > /dev/null

real	0m17.998s
user	0m1.495s
sys	0m21.683s


768

[ruben@rauscher test]$ time cat ~/Downloads/MacTeX.pkg | nc -U /tmp/echo.sock > /dev/null

real	0m15.272s
user	0m1.293s
sys	0m19.043s


1024

[ruben@rauscher test]$ time cat ~/Downloads/MacTeX.pkg | nc -U /tmp/echo.sock > /dev/null

real	0m14.900s
user	0m1.302s
sys	0m18.660s



## Interpretation

* Cat to slow ?
* nc to slow ?
* cpu to slow ?


