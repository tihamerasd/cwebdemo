#include "requester.h"
#include "keyvalue.h"



void print_http_req(http_request hr){
	//printf("req_header:%s %s %s\n", hr.req_type, hr.url, hr.http_version);
	}

void requestfree(http_request hrq){
	sdsfree(hrq.url);
	sdsfree(hrq.http_version);
	sdsfree(hrq.req_type);
	//for (int i=0; i<hrq.headercount; i++){
	//	free(hrq.req_headers[i].key);
	//	free(hrq.req_headers[i].value);
	//	}
	for (int i=0; i<hrq.bodycount-1; i++){
		free(hrq.req_body[i].key);
		free(hrq.req_body[i].value);
		}
	}

	
//TODO this fv is memleaking and really slow... but can't drop, the code need a big rework.
http_request create_request(sds raw_req){

	
	http_request hrq;
	hrq.bodycount=0;
	hrq.headercount=0;
	int count=0, j, count2=0;
	raw_req = sdstrim(raw_req,"\r");
	int lastline=0;

	sds firstline =sdssplitnth(raw_req, sdslen(raw_req), "\n", 1, &count, 0);
	lastline=count-1;
	int postnum=0;
	sds postparams= sdssplitnth(raw_req, sdslen(raw_req), "\n", 1, &postnum, lastline);
	
	hrq.req_type = sdssplitnth(firstline, sdslen(firstline), " ", 1, &count2, 0);
	while (count2<3){
		firstline = sdscat(firstline," badhacker");
		count2++;
		} 
	sds urlandget = sdssplitnth(firstline, sdslen(firstline), " ", 1, &count2, 1);
	hrq.url = sdssplitnth(urlandget,sdslen(urlandget),"?",1,&count2,0);
	hrq.http_version=sdssplitnth(firstline, sdslen(firstline), " ", 1, &count2, 2);

	//TODO separate get and post params
	sds getparams;
	getparams = sdssplitnth(urlandget,sdslen(urlandget),"?",1,&count2,1);
	if(sdslen(postparams)!=0 && sdslen(getparams)!=0)getparams=sdscat(getparams, "&");
	getparams = sdscatsds(getparams,postparams);
	sdsfree(postparams);
	printf("%s\n", getparams);
	int count3=0, count4=0, count5=0;
	for(j=0; j<count2; j++){
		hrq.req_body[j].key=sdsempty();
		hrq.req_body[j].value=sdsempty();
		sds tmp = sdssplitnth(getparams,sdslen(getparams), "&",1, &count4,j);
		sds tmp2 = sdssplitnth(tmp, sdslen(tmp), "=", 1, &count3, 0);
		hrq.req_body[j].key = sdscatsds(hrq.req_body[j].key, tmp2);
		sds tmp3 = sdssplitnth(tmp, sdslen(tmp), "=", 1, &count5, 1);
		if(sdslen(tmp2)>0 && sdslen(tmp3)>0){
		hrq.req_body[j].value = sdscatsds(hrq.req_body[j].value, tmp3);
		hrq.bodycount++;
		}
		count3=0;
		sdsfree(tmp);
		sdsfree(tmp2);
		sdsfree(tmp3);
		//printf("%d\n", hrq.bodycount);
		}
		
	sdsfree(getparams);
	sdsfree(urlandget);
	sdsfree(firstline);
	
	return hrq;
	}
