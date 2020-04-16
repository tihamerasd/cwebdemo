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
deflateInit(&defstream, COMPRESSRATE);
deflate(&defstream, Z_FINISH);
deflateEnd(&defstream);
//printf("Deflated size is: %lu\n", (char*)defstream.next_out - output);
*output_len =(int)((char*)defstream.next_out - output);
output[*output_len]='\0';
}

//TODO url controlled by user, and 404 caching too... What if \0 makes confuse in sdscmp?
//checking the route, if it's returns 1 means route is in routingtable
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

/*checking for created route in controller.c*/
sds do_route(){
	FUNC_PTR fv;
	for (int i=0; i<table.route_count; i++)
		if (sdscmp(threadlocalhrq.url,table.routes[i].url) == 0) {
			fv=table.routes[i].funcref;
			return fv();
			}
	return sdsnew("HTTP/1.1 301 Moved Permanently\r\n"
						  "Location: /\r\n"
		                  "NOTFOUND_URL: TRUE\r\n"
		                  "Cache-Control: no-cache, no-store, must-revalidate\r\n"
		                  "Pragma: no-cache\r\n"
		                  "Expires: 0\r\n\r\n");
}

/*Initialize cache*/
void globalinit_cache(void){
	for (int i=0; i<CACHESIZE; i++){
	cache.cachedpages[i]=create_keyvaluepair();
	}
}

/*free cache, only executed in exit*/
void globalfree_cache(void){
for (int i=0; i<CACHESIZE; i++){
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

/*plain string parsing adding "\r\n" bytes to the response string.*/
void addheadersdone(sds *resp){
	*resp=sdscat(*resp,"\r\n");
	}

void init_file_extension(void){
	int i=0;
	extensions.file_extension[i++].key = sdsnew("png"); extensions.file_extension[0].value = sdsnew("image/png");
	extensions.file_extension[i++].key = sdsnew("css"); extensions.file_extension[1].value = sdsnew("text/css");
	extensions.file_extension[i++].key = sdsnew("js");  extensions.file_extension[2].value = sdsnew("text/javascript");
	extensions.file_extension[i++].key = sdsnew("html"); extensions.file_extension[3].value = sdsnew("text/html");
	extensions.file_extension[i++].key = sdsnew("woff2"); extensions.file_extension[4].value = sdsnew("application/font-woff2");
	extensions.file_extension[i++].key = sdsnew("ico"); extensions.file_extension[5].value = sdsnew("image/x-icon");
	extensions.file_extension[i++].key = sdsnew("gif"); extensions.file_extension[6].value = sdsnew("image/gif");
	extensions.file_extension[i++].key = sdsnew("jpg"); extensions.file_extension[7].value = sdsnew("image/jpeg");
	extensions.file_extension[i++].key = sdsnew("svg"); extensions.file_extension[8].value = sdsnew("image/svg+xml");

	for (i; i<MAX_FILE_EXTENSIONS; i++){
	extensions.file_extension[i].key=sdsempty();
	extensions.file_extension[i].value=sdsempty();
	}

	}

void clear_file_extension(void){
	for (int i=0; i<MAX_FILE_EXTENSIONS; i++){
		sdsfree(extensions.file_extension[i].key);
		sdsfree(extensions.file_extension[i].value);
		}
	}

sds search_file_extension(sds ext){
	for (int i=0; i<MAX_FILE_EXTENSIONS; i++){
		if(sdscmp(ext,extensions.file_extension[i].key)==0) return extensions.file_extension[i].value;
		}
	return NULL;
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
	builder = sdscat(builder,"X-Content-Type-Options: nosniff\r\n");
	builder = sdscat(builder,"X-XSS-Protection: 1; mode=block\r\n");
	builder = sdscat(builder,"X-Frame-Options: deny\r\n");
	builder = sdscat(builder,"Strict-Transport-Security: max-age=31536000; includeSubDomains\r\n");
	builder = sdscat(builder,"Content-Encoding: deflate\r\n");
	builder = sdscat(builder,"Content-Type: ");

	sds type= search_file_extension(extension);
	//printf("TYPE: %s\n",type);
	if (type==NULL) builder = sdscat(builder,"application/octet-stream");
	else builder = sdscatsds(builder, type);
	
	
    builder = sdscat(builder,"\r\n");
	builder = sdscat(builder,"Connection: close\r\n\r\n");
	sdsfree(extension);
	return builder;
	}
