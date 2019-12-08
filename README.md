It's a web framework implementation in the good old C.
The project supports everything you need for a basic webapp like dynamic content rendering, static file serving,
sql database handling, and WAF. You have to do all the things by hand, the framework don't do anything, unless 
you code it, for example no automatic response headers or no default GET/POST handling, but every info you need
is in the threalocalhrq variable (means thread-local-http-request) which is a thread-local object 
from http_request struct. Please note that file uploads will never work over the fixed size (thats in the #define in 
every *server.c) in this "server forced close" style implementation. I probabaly won't implement this because 
I prefer the run-speed not the perfect rfc implementation.

At the last tests, in the http version there was served over 1000 query/s. (intel i5-3th gen.)
Debugged with Burp nikto openVAS and valgrind, memory handling is nearly stable.

C version is at least -std=gnu11 which is the gcc default, because of the thread-local variables. 
Tested with gcc and clang too.

Install on linux:
1. git clone https://github.com/tihamerasd/cwebdemo.git
2. pacman -S sqlite3 / apt-get install sqlit3-dev 
3. pacmam -S wolfssl / apt-get install wolfssl
   regenerate x509 cert, edit the path of them in the code:
4. openssl req -newkey rsa:2048 -nodes -keyout key.pem -x509 -days 365 -out certificate.pem
5. remove my hardcoded paths from Makefile like -L/home/tihi/cweb/wolfssl/wolfssl, 
   (Because I don't installed just compiled wolfssl and link from the build path)
6. make http_multi or make http_single or asmengine_server or make ws
7. chmod +x binary_name (depends on what you build)
8. ./binary_name

Install on *BSD: (Probably on OSX too.)
Only kqueue is for BSD, use the compile_*.sh file instead of Makefile
1. pkg install sqlite3
2. openssl req -newkey rsa:2048 -nodes -keyout key.pem -x509 -days 365 -out certificate.pem
3. ./compile_kqueue_http.sh
4 ./kqueue_http (Sorry HTTPS is broken yet.)

Install on windows: 
Haha! Forget it, never will work.

create route:
1. Check the controllercall() function in backend/controller.c and you will see how to add new routes.
2. After that, define the function which is shown by the function pointer.
3. Implement your logic there.
4. You need to return with malloc-ed sds which contains the heaeders and the html.

rendering template engine:
Watch the temp.html file in frontend/templates and the /admin route ( adminroute() function in controller.c ) as an example.
supported template methods: if, loop, valueing. It's really simple. 
It's based on: libflate, you can read the docs about that.

Add headers: Don't forget to do this on every response. I won't automatize it, this give you controll.

WAF: This waf is very primitive but do the job. Just some string grepping. I won't explain this... 
Check in backend/webapplication_firewall.

Outer Projects:
1. dynamic rendering template : https://github.com/ac000/libflate
2. http parsing: https://github.com/nodejs/http-parser
3. string parsing : https://github.com/antirez/sds //really offered to use this instead of char* 
4. crypto things: https://github.com/wolfssl
5. sql and database: sqlite3, check native api here: https://www.sqlite.org/index.html

TODOS:
1. Cookies and POST parameter parsing not supported at the moment. I should parse them from headers.
   Ofc. it's useable with your own string handler.
2. BSD HTTPS support is broken. Fix that.
4. Refactor the code, need to clean some messy things. Add more comments and a better documentation.
5. Optimize sds strings and libtemplate (Seems to be useless, probably accept syscall is the bottleneck.)
6. Implement a cache layer for database.

Later ideas:
2. Nodejs parser is not my favourite, poking around this alternative: https://github.com/h2o/picohttpparser
3. Port the server to a linux kernel-module.
4. The linux/tls.h maybe interesting alternative against userland crypto.
   https://github.com/torvalds/linux/blob/v4.13/Documentation/networking/tls.txt?fbclid=IwAR0bJmhrp6pdLvl8XXr4s7_kkPHShuw_ewzoQBnozWI6eY0vh10Ca8BZSfg
5. There's already a demo websocket implementation, synchronize that with backend.
6. This power thanks to plain c would be fun in Webrtc and streaming. Implement rtun/stun servers, and dtls.
