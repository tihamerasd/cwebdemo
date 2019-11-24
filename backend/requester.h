#ifndef requester_H
#define requester_H

#include "keyvalue.h"

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
//static __thread http_request threadlocalhrq;
http_request threadlocalhrq;

void create_request(sds);
void print_http_req(http_request);
void requestfree(void);

#endif // requester_H
