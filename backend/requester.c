#include "requester.h"
#include "keyvalue.h"
#include "http_parser/http_parser.h"

thread_local http_request threadlocalhrq;

sds get_cookie_by_name(sds cookiename){
for(int i=0; i<threadlocalhrq.headercount; i++){
		if(strcmp(threadlocalhrq.req_headers[i].key,"Cookie")==0) {
			char* startptr = strstr(threadlocalhrq.req_headers[i].value, cookiename);
			if (startptr==NULL) return sdsempty();
			//TODO check length if startptr is in scope
			startptr=startptr+sdslen(cookiename); //ptr show after the name;
			if (*startptr == '='){		//valid format is "cookiename=blablabla;"
				char* endptr = threadlocalhrq.req_headers[i].value +
							   sdslen(threadlocalhrq.req_headers[i].value); 
				int newlen=0;
				//TODO wtf man? delete this...
				//Can I broke more programming rules in one line, bitch?
				//what if ";" is part of cookie? I drop the end, Is it legal?
				for(char* looper = startptr+1; (*looper != ';' && looper < endptr); looper++, newlen++);
				sds ret = sdsnewlen(startptr+1, newlen);
				printf("Cookie value: %s\n",ret);
				return ret;
			}
		}
	}
puts("Cookie is NULL");
return sdsempty();
}
/*parsing the url, getting the path and get variables.*/
void urlparser(char* url, size_t len){
	int i=1;
	while( i<len && url[i]!='?'){
		if (url[i] == ' ') {
			threadlocalhrq.url = sdsnewlen(url+1,i-1);
			return;}
		else i++;
		}
	threadlocalhrq.url = sdsnewlen(url+1,i-1);
	printf("threadlocalhrq.url: %s\n", threadlocalhrq.url);
	
	while(i<len){
	i++;
	char* separator=&(url[i]);
	int actuallen=i;

	while( i<len && url[i]!='='){
		if (url[i] == ' ') return;
		else i++;
		}

	if(threadlocalhrq.bodycount>=MAX_LIST_LENGTH-2) {puts("segfault here solved!\n"); return; }

	threadlocalhrq.req_body[threadlocalhrq.bodycount].value = sdsempty();
	threadlocalhrq.req_body[threadlocalhrq.bodycount].key = sdsempty();
	threadlocalhrq.req_body[threadlocalhrq.bodycount].key = sdscatlen(
	threadlocalhrq.req_body[threadlocalhrq.bodycount].key, separator, i-actuallen);
	

	i++;
	separator=&(url[i]);
	actuallen=i;
	while( i<len && url[i]!='&'){
		if( url[i] == ' ') {
				threadlocalhrq.req_body[threadlocalhrq.bodycount].value = sdscatlen(
															threadlocalhrq.req_body[threadlocalhrq.bodycount].value,
															separator, i-actuallen);
				threadlocalhrq.bodycount++;
			return;
			}
		else i++;
		}
	threadlocalhrq.req_body[threadlocalhrq.bodycount].value = sdscatlen(
															threadlocalhrq.req_body[threadlocalhrq.bodycount].value,
															separator, i-actuallen);
	threadlocalhrq.bodycount++;
	//printf("get_counter: %d\n", threadlocalhrq.bodycount);
	}
}

void print_http_req(http_request hr){
	//printf("req_header:%s %s %s\n", hr.req_type, hr.url, hr.http_version);
	}

/*free all allocated memory in request object*/
void requestfree(void){
	//printf("req_type: %s\n", threadlocalhrq.req_type);
	sdsfree(threadlocalhrq.req_type);
	//printf("URL: %s\n", threadlocalhrq.url);
	sdsfree(threadlocalhrq.url);
	for(int i=0; i<threadlocalhrq.headercount; i++){
	//printf("HEADER:%s: %s\n", threadlocalhrq.req_headers[i].key, threadlocalhrq.req_headers[i].value);
	sdsfree(threadlocalhrq.req_headers[i].key);
	sdsfree(threadlocalhrq.req_headers[i].value);
	}

	threadlocalhrq.headercount=0;
	for(int i=0; i<threadlocalhrq.bodycount; i++){
	//printf("BODY:%s: %s\n", threadlocalhrq.req_body[i].key, threadlocalhrq.req_body[i].value);
	sdsfree(threadlocalhrq.req_body[i].key);
	sdsfree(threadlocalhrq.req_body[i].value);
	}
	threadlocalhrq.bodycount=0;

	sdsfree(threadlocalhrq.rawurl);
	sdsfree(threadlocalhrq.rawbody);
	}

/*parser callback for headers first parameter*/
int on_header_field (http_parser *_, const char *at, size_t len){
	if(threadlocalhrq.headercount>=MAX_LIST_LENGTH-2) return 0;
	threadlocalhrq.req_headers[threadlocalhrq.headercount].key =sdsnewlen(at, len);    
	threadlocalhrq.req_headers[threadlocalhrq.headercount].value =sdsempty();
	threadlocalhrq.headercount++;  
	return 0;
}

/*parser callback for headers second parameter*/
int on_header_value (http_parser *_, const char *at, size_t len){
	if(threadlocalhrq.headercount>MAX_LIST_LENGTH) return 0;
 	threadlocalhrq.req_headers[threadlocalhrq.headercount-1].value =sdscatlen(threadlocalhrq.req_headers[threadlocalhrq.headercount-1].value,at, len);    
	
	return 0;
}

/*parser callback for headers done*/
int on_headers_complete (http_parser *_, const char *at, size_t len){
	/* Request Methods 
	#define HTTP_METHOD_MAP(XX)         \
	XX(0,  DELETE,      DELETE)       \
	XX(1,  GET,         GET)          \
	XX(2,  HEAD,        HEAD)         \
	XX(3,  POST,        POST)         \
	XX(4,  PUT,         PUT)          \*/
 
	return 0;
}
/*parser callback when pointer is on url*/
int my_url_callback(http_parser *_, const char *at, size_t len){
	//printf("at: %s\n", at);
	//printf("len: %ld\n", len);
	//struct http_parser_url *ipandport=malloc(sizeof(struct http_parser_url));
	//sds tmp = sdsnewlen(at, len);
	//http_parser_parse_url(tmp, sdslen(tmp), 0, ipandport);
	//printf("URL:%s\n", at);
	sds s = sdsnewlen(at, len);	//compiler hack, bypass the constant in *at, if clean the code, throw this out.
	urlparser( s, sdslen(s)); 			//" HTTP/1.1" is 9 length with no ending \0 !!!!!
	threadlocalhrq.rawurl = sdsdup(s);
	sdsfree(s);
	//free(ipandport);
	//sdsfree(tmp);
	return 0;
}

/*callback for body parsing*/
int on_body(http_parser *_, const char *at, size_t len){
	sds s = sdsnewlen(at,len);
	//urlparser(s, len); //TODO value parser almost the same like get param parsing without the '?' sign handler
						 //POST is relatively rare, parse it on
	threadlocalhrq.rawbody=s;
return 0;
}

void init_threadlocalhrq(void){
threadlocalhrq.req_type=NULL;
threadlocalhrq.url=NULL;
threadlocalhrq.http_version=NULL;
threadlocalhrq.headercount=0;
threadlocalhrq.bodycount=0;

for (int i=0; i<MAX_LIST_LENGTH; i++){
	threadlocalhrq.req_headers[i].key=NULL;
	threadlocalhrq.req_headers[i].value=NULL;
	threadlocalhrq.req_body[i].key=NULL;
	threadlocalhrq.req_body[i].value=NULL;
	}

threadlocalhrq.rawurl = NULL;
threadlocalhrq.rawbody= NULL;
	}
	
/*create the request object, threadlocalhrq is the request object, which is thread local*/
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
	int defined=1;
	if (parser->method == 0){ threadlocalhrq.req_type=sdsnew("DELETE"); defined=0;}
 	if (parser->method == 1){ threadlocalhrq.req_type=sdsnew("GET"); defined=0;}
 	if (parser->method == 2){ threadlocalhrq.req_type=sdsnew("HEAD"); defined=0;}
 	if (parser->method == 3){ threadlocalhrq.req_type=sdsnew("POST"); defined=0;}
 	if (parser->method == 4){ threadlocalhrq.req_type=sdsnew("PUT"); defined=0;}
 	if (parser->method == 6){ threadlocalhrq.req_type=sdsnew("OPTIONS"); defined=0;}
 	if (defined == 1) {threadlocalhrq.req_type=sdsnew("UNKNOWN");}
	free(parser);
	}
