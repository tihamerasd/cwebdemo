It's a web framework implementation in c (c99), the only dependency is linux kernel.
But there's a lot of TODO and FIXME. Don't use it in production.

create route:
go to the controllercall() function and u will see how to add (take a new route there, and create a function with your logic).
after that, define the function whixh is shown by the function pointer. You need to return with malloc-ed char* or sds.

rendering template:
Watch the temp.html file and the /admin route, it's basd on: libflate, u can read the docs about.

dynamic rendering template : https://github.com/ac000/libflate
string parsing : https://github.com/antirez/sds
https crypto : https://github.com/wolfssl
frontend template : https://www.os-templates.com/free-website-templates/drywest

TODOS:

Pull this to the library and refactor the http-parsing (now that's full trash) : https://github.com/h2o/picohttpparser
Cookies not supported at the moment. with a new parser this would work.
Use valgrind a lot to check heap allocation bugs.
Refactor the code, need to clean the messy things, change some values to pointers for speed.
Optimize sds strings
