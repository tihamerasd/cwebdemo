#ifndef responser_H
#define responser_H

#include "keyvalue.h"
#include "requester.h"
#include <zlib.h>
#include "../../dev/config.h"

typedef sds (*FUNC_PTR)(void);

extern pthread_mutex_t cache_locker_mutex;

typedef struct Cache{
	keyvaluepair cachedpages[CACHESIZE]; 
	int counter;
} Cache;
Cache cache;

typedef struct file_extensions{
	keyvaluepair file_extension[MAX_FILE_EXTENSIONS];
} file_extensions;
file_extensions extensions;

void globalinit_cache(void);
void globalfree_cache(void);

typedef struct route{
sds url;
void* funcref; //pointers for
} route;

typedef struct routing_table{
route routes[ROUTEARRAYSIZE];
int route_count;
} routing_table;

routing_table table;

int check_route(void);
sds do_route(void);
void create_route(sds url, FUNC_PTR);
sds build_response_header(void);
void addheader(sds*, char*, char*);
sds adddefaultheaders(void);
void addheadersdone(sds *resp);
sds setresponsecode(char*);
void compress_content(char*, int, char*, int*);
void init_file_extension(void);
void clear_file_extension(void);
sds search_file_extension(sds);

#endif //responser_H
