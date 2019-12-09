#include "responser.h"
#include "keyvalue.h"
#include "requester.h"

pthread_mutex_t cache_locker_mutex = PTHREAD_MUTEX_INITIALIZER;

void compress_content(char* input, int input_len, char* output, int* output_len){
z_stream defstream;
defstream.zalloc = Z_NULL;
defstream.zfree = Z_NULL;
defstream.opaque = Z_NULL;
defstream.avail_in = (uInt)input_len; // size of input, string + terminator
defstream.next_in = (Bytef *)input; // input char array
defstream.avail_out = (uInt)input_len; // size of output It's not sure thats enough. malloced outer with same size
defstream.next_out = (Bytef *)output; // output char array
deflateInit(&defstream, 9);
deflate(&defstream, Z_FINISH);
deflateEnd(&defstream);
printf("Deflated size is: %lu\n", (char*)defstream.next_out - output);
*output_len =(int)((char*)defstream.next_out - output);
output[*output_len]='\0';
}

//TODO url controlled by user, and 404 caching too... What if \0 makes confuse in sdscmp?
//checking the route, if it's returns 1 means route is in cache
//0 means route is not in cache, you need to check file system.
int check_route(void){
	for (int i=0; i<table.route_count; i++) {
		if (sdscmp(threadlocalhrq.url, table.routes[i].url) == 0) return 1;
		//printf("dyn-url: %s ,%d\n",threadlocalhrq.url, sdslen(threadlocalhrq.url));
		//printf("route-table: %s. %d\n",table.routes[i].url, sdslen(table.routes[i].url));
	}
	return 0;
}

/*callback function for registering route
 * here will be executed your functions registered inn controller.c*/
void create_route(sds url, FUNC_PTR action){
	route newroute;
	newroute.url = sdsdup(url);
	newroute.funcref=action;
	
	table.routes[table.route_count]=newroute;
	table.route_count++;
}

/*checking for created route in controller.c
 * TDOD why return 404 if not found?*/
sds do_route(){
	FUNC_PTR fv;
	for (int i=0; i<table.route_count; i++)
		if (sdscmp(threadlocalhrq.url,table.routes[i].url) == 0) {
			fv=table.routes[i].funcref;
			return fv();
			}
	return sdsnew("HTTP/1.1 200 OK\r\n"
				  "Server: asm_server\r\n"
				  "Content-Type:text/html\r\n"
			      "Connection: Closed\r\n\r\n"
			      "404 NOT FOUND");
}

/*Initialize cache*/
void globalinit_cache(void){
	for (int i=0; i<100; i++){
	cache.cachedpages[i]=create_keyvaluepair();
	}
}

/*free cache, only executed in exit*/
void globalfree_cache(void){
for (int i=0; i<100; i++){
	sdsfree(cache.cachedpages[i].key);
	sdsfree(cache.cachedpages[i].value);
	}
}


/*Add basic headers for html files, no response object, it's just plain string parsing*/
sds adddefaultheaders(void){
return build_response_header();
	}

/*setting response code, still plain string parsing, watch statuscodes.h for values.*/
sds setresponsecode(char* top){
	sds s=sdsnew(top);
	return s;
	}

/*plain string parsing adding a new header, it's just parse a header into the end of the response string.*/
void addheader(sds *resp, char *key2, char* value2){
	sds key=sdsnew(key2);
	sds value=sdsnew(value2);
	*resp=sdscatsds(*resp,key);
	*resp=sdscatlen(*resp, ": " ,2);
	*resp=sdscatsds(*resp,value);
	*resp=sdscatlen(*resp, "\r\n" ,2);

	sdsfree(key);
	sdsfree(value);
	
	}

/*return  a header, dynamically builds response type*/
sds build_response_header(void){
	sds extension=sdsempty();

	//getting the file type	
	for (int j=sdslen(threadlocalhrq.url)-1; j>=0; j--){
		if( threadlocalhrq.url[j] == '.' ) {
			extension= sdscat(extension, &(threadlocalhrq.url[j+1]));
			break;
			}
		}

	//printf("extension: %s\n",extension);
	
	sds builder=sdsempty();
	builder=sdscat(builder,"HTTP/1.1 200 OK\x0d\x0a");
	builder = sdscat(builder,"Server: asm_server\r\n");
	builder = sdscat(builder,"Content-Encoding: deflate\r\n");
	builder = sdscat(builder,"Content-Type:");
	sds png =sdsnew("png");
	sds css =sdsnew("css");
	sds js =sdsnew("js");
	sds html =sdsnew("html");
	sds woff =sdsnew("woff");

	int notfoundtype=1;
	if(sdscmp(extension,png)  == 0) { builder = sdscat(builder,"image/png");               notfoundtype=0; }
	if(sdscmp(extension,html) == 0) { builder = sdscat(builder,"text/html");               notfoundtype=0; }
	if(sdscmp(extension,css)  == 0) { builder = sdscat(builder,"text/css");                notfoundtype=0; }
	if(sdscmp(extension,js)   == 0) { builder = sdscat(builder,"text/javascript");         notfoundtype=0; }
	if(sdscmp(extension,woff) == 0) { builder = sdscat(builder,"application/x-font-woff"); notfoundtype=0; }
	if( notfoundtype          == 1)   builder = sdscat(builder,"text/html");
	builder = sdscat(builder,"\r\n");
	builder = sdscat(builder,"Connection: Closed\r\n\r\n");

	sdsfree(extension);
	sdsfree(png);
	sdsfree(html);
	sdsfree(css);
	sdsfree(woff);
	sdsfree(js);
	return builder;
	}
