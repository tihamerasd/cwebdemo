/*made by Tihamer Darai*/
#include "keyvalue.h"
#include "requester.h"
#include "responser.h"
#include "controller.c"
#include <signal.h>

void  INThandler(int sig)
{
     char  c;

     signal(sig, SIG_IGN);
     printf("OUCH, did you hit Ctrl-C?\n"
            "Do you really want to quit? [y/n] ");
     c = getchar();
     if (c == 'y' || c == 'Y'){
		for(int i=0; i<table.route_count; i++) sdsfree(table.routes[i].url);
          _exit(0);
          }
     else
          signal(SIGINT, INThandler);
     getchar(); // Get new line character
}
void test_responser(http_request hrq){
	
sds response_header= build_response_header(hrq);

sds response;
if (check_route(hrq.url)!=0) {response = do_route(&hrq);}
else{response = initdir_for_static_files(hrq.url);}
response_header = sdscatsds(response_header,response);

	register char* arg0 asm("rsi");
	register int   arg1 asm("rdx")=sdslen(response_header);
	arg0 = response_header;
    client_to_rdi();
    
     sdsfree(response_header);
	 sdsfree(response);
}

http_request test_requester(void){
	sds req_str=sdsempty();
	register char* raw_http_request asm("rsi");
	register int   raw_req_len asm("rdx");
	_read();
	req_str=sdscatlen(req_str,raw_http_request, raw_req_len);

	http_request req = create_request(req_str);
	sdsfree(req_str);
	return req;
	//TODO if length is over than we got the result is DOS
		}

int main(){

	signal(SIGINT, INThandler);
	while (1) {
	
	extern int server_asm();
	initcode();
	_listen();

//init the routes from the controller.c
controllercall();
	while(1){
		_accept();
		http_request req = test_requester();
		test_responser(req);
		sdsfree(req.req_type);
		sdsfree(req.url);
		sdsfree(req.http_version);
		closesock();
	}
}
	return 0;
}
