#include "requester.h"
#include "keyvalue.h"



void print_http_req(http_request hr){
	printf("%s %s %s\n", hr.req_type, hr.url, hr.http_version);
	//for (int i=0; i<=hr.req_headers_count; i++){
	//	printf("%s: %s\n", hr.req_headers[i].key, hr.req_headers[i].value);
	//	}
	}
http_request create_request(char* raw_req){

	http_request hrq;
	char delim[] = " \r";
	char *ptr = strtok(raw_req, delim);

	if(strlen(ptr)<10) {strcpy(hrq.req_type, ptr); hrq.req_type_count=strlen(ptr);}
	else {strcpy(hrq.req_type,"OVERFLOW"); hrq.req_type_count=8;}
	ptr = strtok(NULL, delim);

	if(strlen(ptr)<300) {strcpy(hrq.url, ptr); hrq.url_count=strlen(ptr);}
	else {strcpy(hrq.url,"OVERFLOW"); hrq.req_type_count=8;}
	strcpy(hrq.url, ptr);
	ptr = strtok(NULL, delim);

	if(strlen(ptr)<20) {strcpy(hrq.http_version, ptr); hrq.http_version_count=strlen(ptr);}
	else {strcpy(hrq.url,"OVERFLOW"); hrq.req_type_count=8;}
	strcpy(hrq.http_version, ptr);
	ptr = strtok(NULL, delim);

	return hrq;
	/*
	delim[0] = "\r";

	while (strlen(ptr) != 1 && ptr != NULL)
	{
		keyvaluepair kvp = create_keyvalue_from_header(ptr);
		hrq.req_headers[hrq.req_headers_count++]= kvp;
		ptr = strtok(NULL, delim);
	}

	//TODO http body missing from here, it's enough to test

	return hrq;
	*/
	}
