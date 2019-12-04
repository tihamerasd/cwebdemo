It's a web framework implementation in c (at least gnu11 which is the gcc default, because of thread-local variables),
The only dependency is linux kernel for core files.

I also implemented a WAF with libyara, so you also need to do sg like "sudo pacman -S yara", or throw that library out.

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

YaraWaf: It's quite mad... Yara is absolutely not made for this. And it's totally bad, but yara is a very powerfool malware analysis tool,
but maybe slow to do things like this. All in all it's c, maybe the server will be fast enough to handle the problem.
See how to write yara rules here: https://github.com/Yara-Rules/rules
YARA is BROKEN in my code! It works, but there are problems with yara and thread handling, so with yara u can waf every user not just the one u want.

dynamic rendering template : https://github.com/ac000/libflate
http parsing: https://github.com/nodejs/http-parser
string parsing : https://github.com/antirez/sds //really offered to use this instead of char* 
https crypto : https://github.com/wolfssl
frontend template : https://www.os-templates.com/free-website-templates/drywest
Yara: https://github.com/VirusTotal/yara

TODOS:
Cookies not supported at the moment. I should parse them from headers. (so boring...)
Refactor the code, need to clean the messy things, change some values to pointers for speed etc... (more boring)
Optimize sds strings for less realloc.
Optimize flate, I want to load templates from memory instead of filesystem.
I have so many issues with nodejs parser, think about this alternative: https://github.com/h2o/picohttpparser
There's a demo websocket implementation, That works but buggy, fix that.
Fix yara, or remove.

Later:
Port the server to a linux kernel-module.
The linux/tls.h maybe interesting alternative against userland crypto.
Database support.
Implement a second lvl cache with database.
