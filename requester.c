#include "requester.h"
#include "keyvalue.h"



void print_http_req(http_request hr){
	printf("req_header:%s %s %s\n", hr.req_type, hr.url, hr.http_version);
	}
http_request create_request(sds raw_req){

	
	http_request hrq;
	sds *tokens, *firstlinetokens;
	int count, j, count2;
	sdstrim(raw_req,"\r");

	sds firstline =sdssplitnth(raw_req, sdslen(raw_req), "\n", 1, &count, 0);
	while (count<3) firstline = sdscat(firstline," badhacker"); 
	
	//printf("%s\n",firstline);
	//tokens = sdssplitlen(raw_req,sdslen(raw_req),"\n",1,&count);
	//sds firstline=sdsdup(tokens[0]);


	//firstlinetokens = sdssplitlen(firstline,sdslen(firstline)," ",1,&count2);
	//printf("returnlen:%d\nurlfail:%s\n", count2,raw_req);
	hrq.req_type = sdssplitnth(firstline, sdslen(firstline), " ", 1, &count2, 0);
	hrq.url = sdssplitnth(firstline, sdslen(firstline), " ", 1, &count2, 1);
	hrq.http_version=sdssplitnth(firstline, sdslen(firstline), " ", 1, &count2, 2);

	
		//sds otherline=sdsempty();
    // TODO HEADERS tokens[j]);

	//for (j = 3; j < count2; j++)sdsfree(firstlinetokens[j]);
	//free(firstlinetokens);
	//sdsfreesplitres(tokens,count);
	sdsfree(firstline);
	//sdsfreesplitres(firstlinetokens,count2);
	return hrq;
	}
