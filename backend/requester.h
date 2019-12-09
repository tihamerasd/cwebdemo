#ifndef requester_H
#define requester_H

#include "keyvalue.h"
#include "threads.h"
#include <pthread.h>

#define MAX_LIST_LENGTH 100

typedef struct http_request{
sds req_type; //GET POST HEAD OPTIONS TRACE PUT DELETE...
sds url;
sds http_version; // HTTP/1.1 for example
keyvaluepair req_headers[MAX_LIST_LENGTH];
int headercount;
keyvaluepair req_body[MAX_LIST_LENGTH]; //means get parameters, a little bit confusing name
int bodycount;
sds rawbody;
sds rawurl;
} http_request;

extern thread_local http_request threadlocalhrq;

void init_threadlocalhrq(void);
void create_request(sds);
void print_http_req(http_request);
void requestfree(void);
void urlparser(char*, size_t);
sds get_cookie_by_name(sds);


#endif // requester_H
