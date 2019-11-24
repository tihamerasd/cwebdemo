#include "stdlib.h"
#include "stdio.h"
#include "sds.h"
#include "string.h"

typedef struct keyvaluepair{
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

http_request threadlocalhrq;

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

int main(){

	//char* url="/index.html?asd=1&b=s&ca=7&verylongvaluejustalittlebitlonger=chaos HTTP/1.1";
	//char* url="/index.html?asd=1&a&c&d==k&s=&b=s&ca=7&verylongvaluejustalittlebitlonger=chaos&omg=fail";
	char* url="/asd HHTP/1.1";
	size_t len= strlen(url);
	threadlocalhrq.bodycount=0;

	urlparser(url, len);

	printf("startpoint: %s\n\n", url);
	printf("url_created: %s\n", threadlocalhrq.url);
	sdsfree(threadlocalhrq.url);
	for(int i=0; i<threadlocalhrq.bodycount; i++){
		printf("key: %s, val: %s\n",threadlocalhrq.req_body[i].key,threadlocalhrq.req_body[i].value);
		sdsfree(threadlocalhrq.req_body[i].key);
		sdsfree(threadlocalhrq.req_body[i].value);
		}
	

return 0;
}
