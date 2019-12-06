It's a web framework implementation in c (at least gnu11 which is the gcc default, because of thread-local variables).
Dependencies:
sqlite3 (or sqlite3-dev on treat like debian systems)
wolfssl (see below)
linux kernel (over 4.12) (Once I probably port to BSD)

There's some TODO and FIXME. The project is not ready, but almost prepared for the first tries. Don't use it in production.

Generete new x509 certs for security ofc. with openssl... lel

create route:
1: Go to the controllercall() function and u will see how to add new routes.
2: After that, define the function which is shown by the function pointer.
3 Implement your logic there.
4: You need to return with malloc-ed sds which contains the heaeders and the html.

rendering template engine:
Watch the temp.html file and the /admin route ( adminroute() function ) as an example.
It's basd on: libflate, u can read the docs about.

Add headers: Don't forget to do this on every response. I won't automatize it, this give you controll.

WAF: This waf is very primitive but do the job. I won't explain this... 

dynamic rendering template : https://github.com/ac000/libflate
http parsing: https://github.com/nodejs/http-parser
string parsing : https://github.com/antirez/sds //really offered to use this instead of char* 
crypto things: https://github.com/wolfssl
sql and database: sqlite3, check native api here: https://www.sqlite.org/index.html

TODOS:
Cookies and POST parameter parsing not supported at the moment. I should parse them from headers. Ofc. it's useable with own string handler (so boring...)
Refactor the code, need to clean the messy things, change some values to pointers for speed etc... (more boring)
Optimize sds strings for less realloc.
Optimize flate, I want to load templates from memory instead of filesystem.
Nodejs parser is not my favourite, think about this alternative: https://github.com/h2o/picohttpparser
There's a demo websocket implementation, synchronize that with backend.

Later:
Port the server to a linux kernel-module.
The linux/tls.h maybe interesting alternative against userland crypto.
Implement a cache for database.
