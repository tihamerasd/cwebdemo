#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include "http_parser.h"
#include <string.h>
#include <assert.h>
#include "sds.h"

typedef struct keyvaluepair {
  sds key;
  sds value;
} keyvaluepair;

typedef struct http_request{
sds req_type; //GET POST HEAD OPTIONS TRACE PUT DELETE...
sds url;
sds http_version; // HTTP/1.1 for example
keyvaluepair req_headers[100];
int headercount;
keyvaluepair req_body[100];
int bodycount;
} http_request;
http_request hrq;

void urlparser(const char* url, size_t len, http_request *hrq){
	int i=0;
	while( i<len && url[i]!='?'){i++;}
	hrq->url = sdsnewlen(url,i-1);

	i--;
	while(i<len){
	while( i<len && url[i]!='='){i++;}
	char* separator=&(url[i-1]);
	hrq->req_body[hrq->bodycount].key = sdsnewlen(separator,i-1);

	if( !(i<len) ){hrq->req_body[hrq->bodycount++].value=sdsempty();} //handle malformed query-s, need to malloc both key and value, or nothing.
	i--;
	while( i<len && url[i]!='&'){i++;}
	separator=&(url[i-1]);
	hrq->req_body[hrq->bodycount++].value = sdsnewlen(separator,i-1);
	}
	
}

void freeall(void){
	sdsfree(hrq.req_type);
	printf("req_type: %s\n", hrq.req_type);
	sdsfree(hrq.url);
	printf("URL: %s\n", hrq.url);
	for(int i=0; i<hrq.headercount; i++){
	printf("HEADER:%s: %s\n", hrq.req_headers[i].key, hrq.req_headers[i].value);
	sdsfree(hrq.req_headers[i].key);
	sdsfree(hrq.req_headers[i].value);
	}

	hrq.headercount=0;
	for(int i=0; i<hrq.bodycount; i++){
	printf("BODY:%s: %s\n", hrq.req_body[i].key, hrq.req_body[i].value);
	sdsfree(hrq.req_body[i].key);
	sdsfree(hrq.req_body[i].value);
	}
	hrq.bodycount=0;
}

int on_header_field (http_parser *_, const char *at, size_t len){
	hrq.req_headers[hrq.headercount].key =sdsnewlen(at, len);    
	return 0;
}

int on_header_value (http_parser *_, const char *at, size_t len){
 	hrq.req_headers[hrq.headercount].value =sdsnewlen(at, len);    
	hrq.headercount++;
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

	if (_->method==0) hrq.req_type=sdsnew("DELETE");
 	if (_->method==1) hrq.req_type=sdsnew("GET");
 	if (_->method==2) hrq.req_type=sdsnew("HEAD");
 	if (_->method==3) hrq.req_type=sdsnew("POST");
 	if (_->method==4) hrq.req_type=sdsnew("PUT");
 
	return 0;
}

int my_url_callback(http_parser *_, const char *at, size_t len){
	struct http_parser_url *ipandport=malloc(sizeof(struct http_parser_url));
	http_parser_parse_url(at, len, 0, ipandport);
	sds url = sdsnewlen(at, len);
	urlparser( at, len, &hrq);
	sdsfree(url);
	free(ipandport);
	return 0;
}

int on_body(http_parser *_, const char *at, size_t len){
	urlparser(at, len, &hrq);
return 0;
}

int main() {
    
  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  struct sockaddr_in server;
  server.sin_family = AF_INET;
  server.sin_port = htons(8081);
  server.sin_addr.s_addr = htonl(INADDR_ANY);
  int opt = 1;
 if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt))<0) {perror("setsockopt");exit(EXIT_FAILURE);}if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, (char *)&opt, sizeof(opt))<0)   {
           perror("setsockopt");exit(EXIT_FAILURE);}
  bind(server_fd, (struct sockaddr*) &server, sizeof(server));
  listen(server_fd, 128);

	http_parser_settings settings;
	http_parser_settings_init( &settings);
	settings.on_url = my_url_callback;
	settings.on_header_field = on_header_field;
	settings.on_header_value = on_header_value;
	settings.on_headers_complete = on_headers_complete;
	settings.on_body = on_body;

for (;;) {
    int client_fd = accept(server_fd, NULL, NULL);

	http_parser *parser = malloc(sizeof(http_parser));
	http_parser_init(parser, HTTP_REQUEST);
	parser->data=NULL;

	size_t len = 80*1024;
	char buf[len];
	ssize_t recved;
	memset(buf,0,len);
	recved = recv(client_fd, buf, len, 0);

	if (recved < 0) {
		/* Handle error. */
	}

/* Start up / continue the parser.
 * Note we pass recved==0 to signal that EOF has been received.
 */
	http_parser_execute(parser, &settings, buf, recved);
	//printf("method: %d\n",parser->method);
    char response[] = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\nConnection: close\r\n\r\nHello, world!";
   send(client_fd, response, sizeof(response), 0);
	freeall();
	free(parser);
    close(client_fd);
  }

  return 0;
}
