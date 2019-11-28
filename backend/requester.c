#include "requester.h"
#include "keyvalue.h"
#include "http_parser/http_parser.h"

thread_local http_request threadlocalhrq;

void urlparser(char* url, size_t len){
	int i=0;
	while( i<len && url[i]!='?'){
		if (url[i] == ' ') {
			threadlocalhrq.url = sdsnewlen(url,i);
			return;}
		else i++;
		}
	threadlocalhrq.url = sdsnewlen(url,i);

	while(i<len){
	i++;
	char* separator=&(url[i]);
	int actuallen=i;

	while( i<len && url[i]!='='){
		if (url[i] == ' ') return;
		else i++;
		}
	
	threadlocalhrq.req_body[threadlocalhrq.bodycount].key = sdsnewlen(separator, i-actuallen);

	i++;
	separator=&(url[i]);
	actuallen=i;
	while( i<len && url[i]!='&'){
		if( url[i] == ' ') {
				threadlocalhrq.req_body[threadlocalhrq.bodycount].value = sdsnewlen(separator, i-actuallen);
				threadlocalhrq.bodycount++;
			return;
			}
		else i++;
		}
	threadlocalhrq.req_body[threadlocalhrq.bodycount].value = sdsnewlen(separator, i-actuallen);
	threadlocalhrq.bodycount++;
	}
}

void print_http_req(http_request hr){
	//printf("req_header:%s %s %s\n", hr.req_type, hr.url, hr.http_version);
	}

void requestfree(void){
	sdsfree(threadlocalhrq.req_type);
	//printf("req_type: %s\n", threadlocalhrq.req_type);
	sdsfree(threadlocalhrq.url);
	//printf("URL: %s\n", threadlocalhrq.url);
	for(int i=0; i<threadlocalhrq.headercount; i++){
	printf("HEADER:%s: %s\n", threadlocalhrq.req_headers[i].key, threadlocalhrq.req_headers[i].value);
	sdsfree(threadlocalhrq.req_headers[i].key);
	sdsfree(threadlocalhrq.req_headers[i].value);
	}

	threadlocalhrq.headercount=0;
	for(int i=0; i<threadlocalhrq.bodycount; i++){
	printf("BODY:%s: %s\n", threadlocalhrq.req_body[i].key, threadlocalhrq.req_body[i].value);
	sdsfree(threadlocalhrq.req_body[i].key);
	sdsfree(threadlocalhrq.req_body[i].value);
	}
	threadlocalhrq.bodycount=0;
	}


int on_header_field (http_parser *_, const char *at, size_t len){
	threadlocalhrq.req_headers[threadlocalhrq.headercount].key =sdsnewlen(at, len);    
	return 0;
}

int on_header_value (http_parser *_, const char *at, size_t len){
	if(threadlocalhrq.headercount>MAX_LIST_LENGTH) return 0;
 	threadlocalhrq.req_headers[threadlocalhrq.headercount].value =sdsnewlen(at, len);    
	threadlocalhrq.headercount++;
	return 0;
}

int on_headers_complete (http_parser *_, const char *at, size_t len){
	/* Request Methods 
	#define HTTP_METHOD_MAP(XX)         \
	XX(0,  DELETE,      DELETE)       \
	XX(1,  GET,         GET)          \
	XX(2,  HEAD,        HEAD)         \
	XX(3,  POST,        POST)         \
	XX(4,  PUT,         PUT)          \*/

	if (_->method==0) threadlocalhrq.req_type=sdsnew("DELETE");
 	if (_->method==1) threadlocalhrq.req_type=sdsnew("GET");
 	if (_->method==2) threadlocalhrq.req_type=sdsnew("HEAD");
 	if (_->method==3) threadlocalhrq.req_type=sdsnew("POST");
 	if (_->method==4) threadlocalhrq.req_type=sdsnew("PUT");
 
	return 0;
}

int my_url_callback(http_parser *_, const char *at, size_t len){
	struct http_parser_url *ipandport=malloc(sizeof(struct http_parser_url));
	http_parser_parse_url(at, len, 0, ipandport);
	printf("URL:%s\n", at);
	sds s = sdsnewlen(at, len);	//compiler hack, bypass the constant in *at, if clean the code, throw this out.
	urlparser( s, len); 			//" HTTP/1.1" is 9 length with no ending \0 !!!!!
	sdsfree(s);
	free(ipandport);
	return 0;
}

int on_body(http_parser *_, const char *at, size_t len){
	sds s = sdsnewlen(at,len);
	urlparser(s, len);
	sdsfree(s);
return 0;
}
	
//TODO this fv is memleaking and really slow... but can't drop, the code need a big rework.
void create_request(sds raw_req){

	http_parser_settings settings;
	http_parser_settings_init( &settings);
	settings.on_url = my_url_callback;
	settings.on_header_field = on_header_field;
	settings.on_header_value = on_header_value;
	settings.on_headers_complete = on_headers_complete;
	//settings.on_body = on_body;

	http_parser *parser = malloc(sizeof(http_parser));
	http_parser_init(parser, HTTP_REQUEST);
	parser->data=NULL;

	http_parser_execute(parser, &settings, raw_req, (size_t)sdslen(raw_req));
	free(parser);
	}
