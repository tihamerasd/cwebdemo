#include "responser.h"
#include "keyvalue.h"
#include "requester.h"

//TODO should care about special chars, urlencode and many more...
int check_route(char* url){
	for (int i=0; i<table.route_count; i++) if (strcmp(url, table.routes[i].url) == 0) return 1;
	return 0;
	}

void create_route(char* url, FUNC_PTR action){
	route newroute;
	strcpy(newroute.url,url);
	newroute.funcref=action;
	
	table.routes[table.route_count]=newroute;
	table.route_count++;
	}

sds do_route(http_request *hrq){
	FUNC_PTR fv;
	for (int i=0; i<table.route_count; i++)
	if (strcmp(hrq->url,table.routes[i].url) == 0) {fv=table.routes[i].funcref;
	return sdsnew(fv(hrq));
	}
		sds notfound = sdsnew("404 NOT FOUND");
		return notfound;
	}

sds build_response_header(http_request req){
	sds *tokens, *tokens2, backsplit;
	int count, j, count2;

	sds line = sdsnew(req.url);
	tokens = sdssplitlen(line,sdslen(line),".",1,&count);
	backsplit=sdsdup(tokens[count-1]);
	tokens2 = sdssplitlen(backsplit,sdslen(backsplit),"?",1,&count2);
	printf("%s\n", tokens2[0]);
	
	//TODO the browser create version like path/to/something.js?v=mainversion.subversion this dot is fucked up...
	sds builder=sdsempty();
	builder=sdscat(builder,"HTTP/1.1 200 OK\x0d\x0a");
	builder = sdscat(builder,"Server: asm_server\r\n");
	builder = sdscat(builder,"Content-Type:");
	if(strcmp(tokens[count-1],"png")==0) sdscat(builder,"image/png");
	if(strcmp(tokens[count-1],"html")==0) sdscat(builder,"text/html");
	if(strcmp(tokens[count-1],"css")==0) sdscat(builder,"text/css");
	if(strcmp(tokens[count-1],"js")==0) sdscat(builder,"text/javascript");
	if(strcmp(tokens[count-1],"woff")==0) sdscat(builder,"application/x-font-woff");
	builder = sdscat(builder,"\r\n");
	builder = sdscat(builder,"Connection: Closed\r\n\r\n");
	//printf("%s\n",builder);
	sdsfreesplitres(tokens,count);
	sdsfreesplitres(tokens2,count2);
	return builder;
	}
