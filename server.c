/*made by Tihamer Darai*/
#include "keyvalue.h"
#include "requester.h"
#include "responser.h"
#include "controller.c"



void test_responser(http_request hrq){
	
sds response_header= sdsempty();
response_header = build_response_header(hrq);
//char *accept_msg = "<!DOCTYPE html> <html><body><h1>asm everywhere<br></h1><h2>my c responder is awesome!</h2></body></html>\r\n";

sds response=sdsempty();
if (check_route(hrq.url)!=0) {response = do_route(&hrq);}
else{response = initdir_for_static_files(hrq.url);}
response_header = sdscatsds(response_header,response);
//sdsfree(response);

	register char* arg0 asm("rsi");
	register int   arg1 asm("rdx")=sdslen(response_header);
	arg0 = response_header;
    client_to_rdi();
  //  sdsfree(response_header);
//if (check_route(hrq.url)==0 && strcmp("It's place for 404!",accept_msg) != 0){

//	}
}

http_request test_requester(void){
	register char* raw_http_request asm("rsi");
	register int   raw_req_len asm("rdx");
	_read();
	http_request req = create_request(raw_http_request);
	print_http_req(req);
	return req;
	//TODO if length is over 1024 we miss the data
	// TODO _read() sets the values from asm, you should store it somewhere, or it will disappear on next syscall
	//printf("%s\n%d\n\n",raw_http_request,raw_req_len);
	}

int main(){
	extern int server_asm();
	initcode();
	_listen();

//init the routes in the controller.c
controllercall();

	while(1){
		_accept();
		http_request req = test_requester();
		test_responser(req);
		closesock();
	}

	return 0;
}
