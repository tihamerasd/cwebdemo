#include "responser.h"
#include "keyvalue.h"
#include "requester.h"



//TODO should care about special chars, urlencode and many more...
int check_route(void){
	for (int i=0; i<table.route_count; i++) if (sdscmp(threadlocalhrq.url, table.routes[i].url) == 0) return 1;
	return 0;
	}


void create_route(sds url, FUNC_PTR action){
	route newroute;
	newroute.url = sdsdup(url);
	newroute.funcref=action;
	
	table.routes[table.route_count]=newroute;
	table.route_count++;
	}

sds do_route(){
	FUNC_PTR fv;
	for (int i=0; i<table.route_count; i++)
		if (sdscmp(threadlocalhrq.url,table.routes[i].url) == 0) {
			fv=table.routes[i].funcref;
			return fv();
			}
	return sdsnew("404 NOT FOUND");
}

void globalinit_cache(void){
	for (int i=0; i<100; i++){
	cache.cachedpages[i]=create_keyvaluepair();
	}
}

void globalfree_cache(void){
for (int i=0; i<100; i++){
	sdsfree(cache.cachedpages[i].key);
	sdsfree(cache.cachedpages[i].value);
	}
}



sds adddefaultheaders(void){
return build_response_header();
	}

sds setresponsecode(char* top){
	sds s=sdsnew(top);
	return s;
	}

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

sds build_response_header(void){
	sds *tokens, *tokens2, backsplit;
	int count=0, count2=0;

	//getting the content type
	sds line = sdsdup(threadlocalhrq.url);
	tokens = sdssplitlen(line,sdslen(line),"?",1,&count);
	backsplit=sdsdup(tokens[0]);
	tokens2 = sdssplitlen(backsplit,sdslen(backsplit),".",1,&count2);
	//printf("filetype: %s\n", tokens2[1]);
	
	sds builder=sdsempty();
	builder=sdscat(builder,"HTTP/1.1 200 OK\x0d\x0a");
	builder = sdscat(builder,"Server: asm_server\r\n");
	builder = sdscat(builder,"Content-Type:");
	sds png =sdsnew("png");
	sds css =sdsnew("css");
	sds js =sdsnew("js");
	sds html =sdsnew("html");
	sds woff =sdsnew("woff");
	int notfoundtype=1;
	if(sdscmp(tokens2[count2-1],png)==0){ builder = sdscat(builder,"image/png"); notfoundtype=0;}
	if(sdscmp(tokens2[count2-1],html)==0){ builder = sdscat(builder,"text/html");notfoundtype=0;}
	if(sdscmp(tokens2[count2-1],css)==0){ builder = sdscat(builder,"text/css");notfoundtype=0;}
	if(sdscmp(tokens2[count2-1],js)==0){ builder = sdscat(builder,"text/javascript");notfoundtype=0;}
	if(sdscmp(tokens2[count2-1],woff)==0){ builder = sdscat(builder,"application/x-font-woff");notfoundtype=0;}
	if(notfoundtype == 1) sdscat(builder,"text/html");;
	builder = sdscat(builder,"\r\n");
	builder = sdscat(builder,"Connection: Closed\r\n\r\n");
	//printf("%s\n",builder);
	sdsfreesplitres(tokens,count);
	sdsfreesplitres(tokens2, count2);
	sdsfree(backsplit);
	sdsfree(line);
	sdsfree(png);
	sdsfree(html);
	sdsfree(css);
	sdsfree(woff);
	sdsfree(js);
	return builder;
	}
