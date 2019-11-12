#ifndef responser_H
#define responser_H

#include "keyvalue.h"
#include "requester.h"

typedef char* (*FUNC_PTR)(http_request*);

//ADDING YOUR ROUTES HERE
//100 max right now 
typedef struct route{
char url[300];
void* funcref; //pointers for
} route;

typedef struct routing_table{
route routes[200];
int route_count;
} routing_table;

routing_table table;

typedef struct http_response{
char http_version[20];
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

char* resp_headerify(keyvaluepair*); //pointer will be faster than a real copy
char* resp_bodyfy(keyvaluepair);
int check_route(char*);
char* do_route(http_request*);
void create_route(char* url, FUNC_PTR);
char* build_response(http_response);

#endif //responser_H
