#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "./backend/keyvalue.h"
#include "./backend/requester.h"
#include "./backend/responser.h"
#include "./backend/controller.h"
#include "./backend/sql/sqlthings.h"
#include "backend/webapplication_firewall/simple_waf.h"
#include "../dev/config.h"

void portable_requester(char* rawreq, int len){
	sds s = sdsempty();
	init_threadlocalhrq();
	s=sdscatlen(s,rawreq,len);
	create_request(s);
	sdsfree(s);
}

sds portable_responser(void){
	sds response;
	if(threadlocalhrq.url==NULL) {
		threadlocalhrq.url=sdsnew("/");    //go to webroot
		threadlocalhrq.rawurl=sdsnew("/"); //go to webroot
		puts("ERROR! Somehow url is null\n");
	}
	if (check_route()!=0) {
		response = do_route();
	}
	else{
		sds response_body = initdir_for_static_files();
		if(response_body == NULL){
				response=sdsnew("HTTP/1.1 301 Moved Permanently\r\n"
								"Location: /\r\n"
								"NOTFOUND_URL: TRUE\r\n"
								"Cache-Control: no-cache, no-store, must-revalidate\r\n"
								"Pragma: no-cache\r\n"
								"Expires: 0\r\n\r\n");
			}
		else{
			response = adddefaultheaders();
			response = sdscatsds(response,response_body);
			sdsfree(response_body);
		}
	}
	return response;
}

void  INThandler(int sig){
	puts("OK I'm exit...");
	for(int i=0; i<table.route_count; i++) sdsfree(table.routes[i].url);
	globalfree_cache();
	sqlite_close_function();
	clear_file_extension();
    exit (0);
}

void my_sighandler(){
	signal(SIGINT, INThandler);
	signal(SIGPIPE, SIG_IGN);
	}

sds packed_function(char* in_str)
{
	sds response=NULL;
	if (strlen(in_str)<0) return NULL; //error handling
	int match = simple_waf(in_str, strlen(in_str));
	if( match == 1){
		char* simpleblock="HTTP/1.1 301 Moved Permanently\r\n"
						  "Location: /\r\n"
		                  "Set-Cookie: Hacker=TRUE\r\n"
		                  "Cache-Control: no-cache, no-store, must-revalidate\r\n"
		                  "Pragma: no-cache\r\n"
		                  "Expires: 0\r\n\r\n";
		response=sdsnew(simpleblock);
				}
	else{
		portable_requester(in_str, strlen(in_str));
		response=portable_responser();
		requestfree();
	}
    return ((char*)response);
}


char* extern_port_to_go(void){
	char *str =malloc(7);
	memset(str,0,7);
	str[0]=':';
	printf("in C returned: %s\n",str);
	sprintf(str+1, "%d", PORT);
	return str;
	}
char* extern_cert_to_go(void){return CERTFILE;}
char* extern_key_to_go(void){return KEYFILE;}
