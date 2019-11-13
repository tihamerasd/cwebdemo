#ifndef responser_H
#define responser_H

#include "keyvalue.h"
#include "requester.h"

typedef sds (*FUNC_PTR)(http_request*);

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
sds resp_body; //the html code... probably it should be malloc-ed dynamically, or just split to the buffer on real time and don't store at all
} http_response;

keyvaluepair okkvp;
keyvaluepair notfoundkvp;
keyvaluepair inernalerrkvp;
//okkvp.key="200\0";
//okkvp.value="OK\0";
//okkvp.key_len=3;
//okkvp.value_len=2;

//... set values ...

sds resp_headerify(keyvaluepair*); //pointer will be faster than a real copy
sds resp_bodyfy(keyvaluepair);
int check_route(sds);
sds do_route(http_request*);
void create_route(sds url, FUNC_PTR);
sds build_response(http_response);
sds build_response_header(http_request);

#endif //responser_H
