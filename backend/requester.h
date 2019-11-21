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

http_request create_request(sds);
void print_http_req(http_request);
void requestfree(http_request);

#endif // requester_H
