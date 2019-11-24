#ifndef requester_H
#define requester_H
#include "keyvalue.h"
#include "threads.h"

typedef struct http_request{
sds req_type; //GET POST HEAD OPTIONS TRACE PUT DELETE...
sds url;
sds http_version; // HTTP/1.1 for example
keyvaluepair req_headers[100];
int headercount;
keyvaluepair req_body[100];
int bodycount;
} http_request;

//TODO wtf? it's really important but cause segfault
//thread_local http_request threadlocalhrq;
//http_request threadlocalhrq;

extern thread_local http_request threadlocalhrq;


void create_request(sds);
void print_http_req(http_request);
void requestfree(void);
void urlparser(char*, size_t);

#endif // requester_H
