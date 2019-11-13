#include "responser.h"
#include "keyvalue.h"
#include "requester.h"

//TODO should care about special chars, urlencode and many more...
int check_route(sds url){
	for (int i=0; i<table.route_count; i++) if (sdscmp(url, table.routes[i].url) == 0) return 1;
	return 0;
	}

void create_route(sds url, FUNC_PTR action){
	route newroute;
	newroute.url = sdsdup(url);
	newroute.funcref=action;
	
	table.routes[table.route_count]=newroute;
	table.route_count++;
	}

sds do_route(http_request *hrq){
	FUNC_PTR fv;
	for (int i=0; i<table.route_count; i++)
		if (sdscmp(hrq->url,table.routes[i].url) == 0) {
			fv=table.routes[i].funcref;
			return fv(hrq);
			}
	return sdsnew("404 NOT FOUND");
}

sds build_response_header(http_request req){
	sds *tokens, *tokens2, backsplit;
	int count, j, count2;

	sds line = sdsnew(req.url);
	tokens = sdssplitlen(line,sdslen(line),"?",1,&count);
	backsplit=sdsdup(tokens[0]);
	tokens2 = sdssplitlen(backsplit,sdslen(backsplit),".",1,&count2);
	//printf("filetype: %s\n", tokens2[1]);
	
	//TODO the browser create version like path/to/something.js?v=mainversion.subversion this dot is fucked up...
	sds builder=sdsempty();
	builder=sdscat(builder,"HTTP/1.1 200 OK\x0d\x0a");
	builder = sdscat(builder,"Server: asm_server\r\n");
	builder = sdscat(builder,"Content-Type:");
	sds png =sdsnew("png");
	sds css =sdsnew("css");
	sds js =sdsnew("js");
	sds html =sdsnew("html");
	sds woff =sdsnew("woff");
	if(sdscmp(tokens2[count2-1],png)==0) builder = sdscat(builder,"image/png");
	if(sdscmp(tokens2[count2-1],html)==0) builder = sdscat(builder,"text/html");
	if(sdscmp(tokens2[count2-1],css)==0) builder = sdscat(builder,"text/css");
	if(sdscmp(tokens2[count2-1],js)==0) builder = sdscat(builder,"text/javascript");
	if(sdscmp(tokens2[count2-1],woff)==0) builder = sdscat(builder,"application/x-font-woff");
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
