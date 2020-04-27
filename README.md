# Cwebdemo
It's a web framework implementation in C.
Please note that developing web content in C is insecure and unstable, but your server will be very fast and lightweight. You should only do that from education purpose or if you have a good reason.
Live version: https://mprotect.hu
## Install
Install on linux:
- git clone https://github.com/tihamerasd/cwebdemo.git
- OPTIONAL: regenerate x509 cert for real world usage, but I give a default one too:
 ```sh 
openssl req -newkey rsa:2048 -nodes -keyout key.pem -x509 -days 365 -out certificate.pem
```
- Edit dev/config.h as you need.
- Compile:
-- epoll is an linux based nonblocking single thread server. 
--  make_shared is a platform and socket independent solution, actually built with a go based https socket.
```sh
./compile_http_epoll.sh
#or
# sudo pacman -S go
./make_shared.sh
```
## Install on *BSD and OSX:
- git clone https://github.com/tihamerasd/cwebdemo.git
- OPTIONAL openssl req -newkey rsa:2048 -nodes -keyout key.pem -x509 -days 365 -out certificate.pem
- Edit config.h
- Compile:
-- kqueue is an UNIX based nonblocking single thread server. 
--  make_shared is a platform and socket independent solution, actually built with a go based https socket.
```sh
./compile_http_kqueue.sh
#or
#pkg install go
./make_shared.sh
```
## Install on windows: 
- Haha! Forget it, never will work.

#### You still need database initializaton to run the demo. It's based on sqlite.
In dev/db there is a db_init.sh file, use that to create my demo schema. In real project you should do something similar. Use my insecure password generator (stores pw in bash history) to set up a password for the demo project, admin page is https://localhost:8000/richtext/index.html

## Directory structure:
- dev: The directory for developers. Implement your functions here.
- - db: database related things
- - fronted: What do you think? :)
- - routing: It contains your controller for dynamic query handling
- server: It's responsible for sockets and query serving
- - the core server which provides you the functions
- build: the directory where the bnaries are stored
- retired_servers: a lot of fun technique to handle sockets and https query serving, but actually these are outdated, you can't easily recompile them. Maybe try to rollback to a later version then they are in the project home directory
- management: This directory stores helper scripts for updating and clearing your repository. WARNING! store your code before you run, this maybe not stable
- - update.sh: I guess you understand...
- - unstable_update.sh: updates git projects included to my project
- - init_empty_project.sh : please make sure you understand the topology and compiler errors.

## Create route:
- Check the controllercall() function in dev/routing/ and you will see how to add new routes.
- After that, define the function which is shown by the function pointer.
- Implement your logic there. (See available helper functions later.)
- You need to return with malloc-ed sds which contains the heaeders and the html too.
- add demoroute to the header file
#### Example
```c
sds demoroute(void){
	sds response = setresponsecode(okcode); //means HTTP/1.1 200 OK
	addheader(&response, "Connection", "close");
	addheader(&response, "Content-Type", "text/html");
	addheadersdone(&response);
	response=sdscat(response,"<h1>This Works!</h1>");
	fclose(fp);
	return response;
	}

/*register the routes here*/
void controllercall(void){
	create_route("demo", &demoroute); //means: https://localhost:8000/demo
	}
```

## Rendering template engine:
Watch the dev/frontend/templates for some example.
supported template methods: if, loop, valueing, include 

It's really simple, based on: libflate, you can read more about in the libflate documentation here: https://github.com/ac000/libflate

## Sds - dynamic strings:
It's c and string handling often very uncomfortable, this repository is for make it easier, probably will save you some time if you don't have to debug overflows. Please read documentation here: https://github.com/antirez/sds

## Basics:
- Add headers: Don't forget to do this on every response. I won't automatize it, this give you controll.
- Give attention for cache handling, if your page is dynamic, like handling cookies, or time dependent, or changing the database between queries etc...
   DONT CACHE that. You can leak sensitive data with bad cache method, or just simply won't work...
- Cookie-handling: sds get_cookie_by_name(sds cookiename): use this function in your route callback function,
   note that this returns with "NOTFOUND" instead of null if value not found. After that, implement your logic with the value...
- header search: just loop through threadlocalhrq.headers[i]
- GET: threadlocalhrq.body[i]
- POST not parsed use it as raw in threadlocalhrq.rawbody
#### Example request object structure from the server:
```c
typedef struct http_request{
    sds req_type; //GET POST HEAD OPTIONS TRACE PUT DELETE...
    sds url;
    sds http_version; // HTTP/1.1 for example
    keyvaluepair req_headers[MAX_LIST_LENGTH];
    int headercount;
    keyvaluepair req_body[MAX_LIST_LENGTH]; //means get parameters, a little bit confusing name
    int bodycount;
    sds rawbody;
    sds rawurl;
} http_request;
```
## Web Application Firewall:
This waf is very primitive but do the job. Just some string grepping. I won't explain this...
Feel free to turn it off.
Edit the filtering in config.h
#### Example
it's important to bann templates, without this, an attacker can guess your template files.
```c
//WEB APPLICATION FIREWALL SETTINGS
#define BANNLEN 3 //number of banned words
//array to bann
#define BANNTHIS "../","..%2F","templates/"
```
## Response helper functions
```c
//get a cookie from request, you need to free the return value
sds get_cookie_by_name(sds); 

//it's for static file serving, this will guess the file type. Probably you will never need this.
sds build_response_header(void);

// add a new header to the response object (response object is just a buffer with length)
void addheader(sds*, char*, char*);

// default minimal requirements for headers, if you don't know what you are doing, try this
sds adddefaultheaders(void);

//Closing the header part of the response
void addheadersdone(sds *resp);

//set response code, try to guess the response based on responses.h, if it's fail do it by hand
sds setresponsecode(char*);

//flate algorithm for compressed content serving, examples later.
void compress_content(char*, int, char*, int*);
```
## How to use sql
- In dev/db edit the sql_queries file
- you need an existing db file, which you can define in the config file.
- The return value is stored in a global object
- sqlite is able to build with threadsafe options, but i'm not sure this happens in amalgamation build, while it's single thread probably ok, need some more information for later usage...
- dont forget to add your functions to header file
#### example
```c
/*Select the articles by category*/
void select_by_category_en(char* category){
	char *sql = sqlite3_mprintf("SELECT title_EN,category,content_EN,created_at from posts WHERE category='%q'", category);   
    int rc = sqlite3_exec(db, sql, callback, 0, &mysql_err_msg);
    if (rc != SQLITE_OK ) {  
        fprintf(stderr, "Failed to select data\n");
        fprintf(stderr, "SQL error: %s\n", mysql_err_msg);
        sqlite3_free(mysql_err_msg);
        sqlite3_free(sql);
        return;
    }
```
sql respponse data is stored in: (kvp means key-value-pair)
```c
extern thread_local keyvaluepair kvp_array_sqldata[MAXPOSTSSHOWN];
```
You can read them with this:
title_EN, category, content_EN, created_at is 4 parameter in SELECT so I need to follow it in my loop with i+=4, because c is very lowlevel and the return is just bytes in the memory.
```c
for (int i=0; i<MAXPOSTSSHOWN; i+=4){
		if (kvp_array_sqldata[i].key!=NULL && kvp_array_sqldata[i].value!=NULL){
			sds sdsurl= sdsnew("/onepost?tittle=");
			sdsurl =sdscatsds(sdsurl, kvp_array_sqldata[i].value);

			flateSetVar(f, "LINK2",sdsurl);
			flateSetVar(f, "TITTLE2", kvp_array_sqldata[i].value);
			flateSetVar(f, "createdat", kvp_array_sqldata[i+3].value);
			flateDumpTableLine(f, "listsecurity");
			sdsfree(sdsurl);
		}
```

##Debug advices:
Actually projects are configured for production environment (Full RELRO, PIE, NX-enabled, -fstack-protector-strong), remove at least "-s" flag for function names in the binary and a usefull backtrack.

#### Known issues:
- In HTTP servers there is only one read, not a "loop while not end". Yes, it's for security reason, and not compatible with rfc. I won't change it. Add a new socket handler if it's not ok for you.
- uncomfortable post saving in demo

## Remote projects used:
- dynamic rendering template : https://github.com/ac000/libflate
- http parsing: https://github.com/nodejs/http-parser
- string parsing : https://github.com/antirez/sds //really offered to use this instead of char* 
- Crypto things: handeld by golang and openssl (Easy to remove if not needed)
- sql and database: sqlite3 - amalgation build, check native api here: https://www.sqlite.org/index.html

## dependencies updated at 2020.04.17.
