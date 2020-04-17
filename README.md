It's a web framework implementation in C.
The project supports everything you need for a basic webapp like dynamic content rendering, static file serving,
sql database handling, and WAF. You have to do all the things by hand, the framework don't do anything, unless 
you code it, for example no automatic response headers or no default GET/POST handling, but every info you need
is in the threalocalhrq variable (means thread-local-http-request) which is a thread-local object 
from http_request struct. Please note that file uploads will never work over the fixed size (thats in the config.h)
in this "server forced close" style implementation. I probabaly won't implement this because 
I prefer the run-speed not the perfect rfc implementation.

It's C... memory handling seems to be stable it's probably never will be perfect.

C version is at least -std=gnu11 which is the gcc default, because of the thread-local variables. 
Tested with gcc and clang too.

Install on linux:
1. git clone https://github.com/tihamerasd/cwebdemo.git
2. pacman -S sqlite3 / apt-get install sqlit3-dev 
   regenerate x509 cert for real world usage, but I give a default one too:
3. openssl req -newkey rsa:2048 -nodes -keyout key.pem -x509 -days 365 -out certificate.pem
4. ./compile_http_epoll.sh / ./make_shared.sh
5. Edit dev/config.h as you need.
6. Generate database, generate password if you need. check it on dev/db

Install on *BSD: (Probably on OSX too.)
1. pkg install sqlite3
2. openssl req -newkey rsa:2048 -nodes -keyout key.pem -x509 -days 365 -out certificate.pem
3. Edit config.h
4. ./make shared ./compile_kqueue_http.sh
5. Generate database, generate password if you need. check it on dev/db

Install on windows: 
Haha! Forget it, never will work.

In dev/db there is a db_init.sh file use that to create my demo schema. In real project you should do something similar.

dev: The directory for developers. <br>
---db: database related things <br>
---fronted: What do you think? :) <br>
---routing: It contains your controller for dynamic query handling <br>
server: it's responsible for sockets and query serving <br>
---more details later <br>
build: the directory where the bnaries are stored <br>
retired_servers: a lot of fun technique to handle sockets and https query serving, <br>
but actually these are outdated, you can't easily recompile them <br>

Create route:
1. Check the controllercall() function in dev/routing/ and you will see how to add new routes.
2. After that, define the function which is shown by the function pointer.
3. Implement your logic there.
4. You need to return with malloc-ed sds which contains the heaeders and the html.

Rendering template engine:
Watch the dev/frontend/templates for some example.
supported template methods: if, loop, valueing. It's really simple. 
It's based on: libflate, you can read more in the libflate docs about that. (link is below)

Basics:
1. Add headers: Don't forget to do this on every response. I won't automatize it, this give you controll.
2. Give attention for cache handling, if your page is dynamic, like handling cookies, or time dependent, or changing the database between queries etc...
   DONT CACHE that. You can leak sensitive data with bad cache method, or just simply won't work...
3. Cookie-handling: sds get_cookie_by_name(sds cookiename): use this function in your route callback function,
   note that this returns with "NOTFOUND" instead of null if value not found. After that, implement your logic with the value...
4. header search: just loop through threadlocalhrq.headers[i]
5. GET: threadlocalhrq.body[i]
6. POST not parsed use it as raw in threadlocalhrq.rawbody

WAF:
This waf is very primitive but do the job. Just some string grepping. I won't explain this...
Probably no problem if you turn it off.
Check in server/backend/webapplication_firewall.h, but edit the filtering in config.h

Outer Projects: ( dependencies updated at 2020.04.17.)
1. dynamic rendering template : https://github.com/ac000/libflate
2. http parsing: https://github.com/nodejs/http-parser
3. string parsing : https://github.com/antirez/sds //really offered to use this instead of char* 
4. crypto things: handeld by golangand openssl
5. sql and database: sqlite3 - amalgation build, check native api here: https://www.sqlite.org/index.html

TODOS:
0. clean script for empty project, updater method for projects.
1. BSD kqueue is broken on different ways.
2. Create Documentation with pictures and with more details
3. Clear the code and the comments
4. Implement a fuzzer, looking for memory leaks
5. Rewrite/Port the server to a linux kernel-module. (Yes I know it's a big one.)
6. The linux/tls.h maybe interesting alternative against userland crypto.
   https://github.com/torvalds/linux/blob/v4.13/Documentation/networking/tls.txt
7. There's already a demo websocket implementation in retired_servers, synchronize that with backend.
