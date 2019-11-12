#ifndef requester_H
#define requester_H

#include "keyvalue.h"

typedef struct http_request{
char req_type[10]; //GET POST HEAD OPTIONS TRACE PUT DELETE...
char url[300];
char http_version[20]; // HTTP/1.1 for example
keyvaluepair req_headers[100];
keyvaluepair req_body[100];
int req_type_count, url_count, http_version_count, req_headers_count, req_body_count;
} http_request;

http_request create_request(char*);
void print_http_req(http_request);

#endif // requester_H
