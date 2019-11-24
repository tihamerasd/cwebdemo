#ifndef responser_H
#define responser_H

#include "keyvalue.h"
#include "requester.h"

typedef sds (*FUNC_PTR)(void);

typedef struct Cache{
keyvaluepair cachedpages[100]; 
int counter;
} Cache;

Cache cache;

void globalinit_cache(void);
void globalfree_cache(void);

typedef struct route{
sds url;
void* funcref; //pointers for
} route;

typedef struct routing_table{
route routes[200];
int route_count;
} routing_table;

routing_table table;

typedef struct http_response{
sds http_version;
keyvaluepair response_code_plus_name; //200 OK or 404 Not found eg.
keyvaluepair resp_headers[100];
int headers_counter;
sds resp_body; //the html code... probably it should be malloc-ed dynamically, or just split to the buffer on real time and don't store at all
} http_response;

int check_route(void);
sds do_route(void);
void create_route(sds url, FUNC_PTR);
sds build_response(http_response);
sds build_response_header(void);
void addheader(sds*, char*, char*);
sds adddefaultheaders(void);
sds setresponsecode(char*);

#endif //responser_H
