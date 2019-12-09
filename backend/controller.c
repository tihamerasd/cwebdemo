#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <ctype.h>
#include "html_templater/flate.h"
#include "statuscodes.h"
#include "sql/sqlthings.h"
//#include "../wolfssl/wolfssl/wolfssl/wolfcrypt/sha512.h"
#include "../wolfssl/wolfssl/wolfssl/wolfcrypt/sha256.h"

#define ROOTPATH "frontend/"

//TODO mutex locking cache object when write...
sds serve_from_cache(void);
void add_to_cache(sds);

sds onepostroute(void){
	sds cachedcontent = serve_from_cache();
	if (cachedcontent != NULL) return cachedcontent;

	char* urldecoded=NULL;
	for(int i=0; i<threadlocalhrq.bodycount; i++){
		if (strcmp(threadlocalhrq.req_body[i].key,"tittle") == 0){
			urldecoded = malloc(sdslen(threadlocalhrq.req_body[i].value)+1);
			percent_decode(urldecoded,threadlocalhrq.req_body[i].value);
			break;
		}
	}
	
	init_callback_sql();
	select_by_name(urldecoded);
	free(urldecoded);

	Flate *f = NULL;
	flateSetFile(&f, "frontend/templates/one_post.html");
	flateSetVar(f, "contentzone", "");
	flateSetVar(f, "title", kvp_array_sqldata[0].value);
	flateSetVar(f, "category",kvp_array_sqldata[1].value );
	flateSetVar(f, "content", kvp_array_sqldata[2].value);

	free_callback_sql();
	sds response = setresponsecode(okcode); //means HTTP/1.1 200 OK
	addheader(&response, "Connection", "Closed");
	addheader(&response, "Content-Type", "text/html");
	char *buf = flatePage(f);
	sds dynpage =sdsnew(buf);
	free(buf);
	flateFreeMem(f);
	
	response = sdscatsds(response, dynpage);
	sdsfree(dynpage);
	add_to_cache(response);
	return response;
}

sds listincategoryroute(void){

	sds cachedcontent = serve_from_cache();
	if (cachedcontent != NULL) return cachedcontent;

	char* urldecoded=NULL;
	for(int i=0; i<threadlocalhrq.bodycount; i++){
		if (strcmp(threadlocalhrq.req_body[i].key,"category") == 0){
			urldecoded = malloc(sdslen(threadlocalhrq.req_body[i].value)+1);
			percent_decode(urldecoded,threadlocalhrq.req_body[i].value);
			break;
		}
	}
	init_callback_sql();
	select_by_category(urldecoded);
	free(urldecoded);

	Flate *f = NULL;
	flateSetFile(&f, "frontend/templates/all_in_category.html");
	flateSetVar(f, "categorynamezone", "");
	flateSetVar(f, "categoryname", kvp_array_sqldata[1].value);

	for (int i=0; i<MAXPOSTSSHOWN; i+=3){
		if (kvp_array_sqldata[i].key!=NULL && kvp_array_sqldata[i].value!=NULL){
			sds sdsurl= sdsnew("/onepost?tittle=");
			sdsurl =sdscatsds(sdsurl, kvp_array_sqldata[i].value);

			flateSetVar(f, "urlsource",sdsurl);
			flateSetVar(f, "tittle", kvp_array_sqldata[i].value);
			//flateSetVar(f, "content", kvp_array_sqldata[i+2].value);
			flateDumpTableLine(f, "ullist");
			sdsfree(sdsurl);
		}
	}
    
	free_callback_sql();
	sds response = setresponsecode(okcode); //means HTTP/1.1 200 OK
	addheader(&response, "Connection", "Closed");
	addheader(&response, "Content-Type", "text/html");
	char *buf = flatePage(f);
	sds dynpage =sdsnew(buf);
	free(buf);
	flateFreeMem(f);
	
	response = sdscatsds(response, dynpage);
	sdsfree(dynpage);
	add_to_cache(response);
	return response;
}

sds saveroute(void){
	//TODO very bad, once do it not like a retard...
	//check authetntication
	for(int i=0; i<threadlocalhrq.headercount; i++){
		if(strcmp(threadlocalhrq.req_headers[i].key,"Authentication")==0) {
			unsigned char hardcodedhash[SHA256_DIGEST_SIZE];
			unsigned char pwhash[SHA256_DIGEST_SIZE];
			FILE *fp;
			//read from file, so if the file length <255 no overflow
			char pwbuff[255];
			fp = fopen("backend/password.txt", "r");
			fscanf(fp, "%s", pwbuff);
			fclose(fp);

			wc_Sha256Hash(pwbuff, strlen(pwbuff), hardcodedhash);
			wc_Sha256Hash(threadlocalhrq.req_headers[i].value, sdslen(threadlocalhrq.req_headers[i].value), pwhash);
			if (memcmp(hardcodedhash,pwhash, SHA256_DIGEST_SIZE)!=0) return sdsnew("BAD PASSWORD");	;
		}
	}

	char* urldecoded=NULL;
	sds tittle = NULL;
	sds category = NULL;
	for(int i=0; i<threadlocalhrq.bodycount; i++){
		if (strcmp(threadlocalhrq.req_body[i].key,"content") == 0){
			urldecoded = malloc(sdslen(threadlocalhrq.req_body[i].value)+1);
			percent_decode(urldecoded,threadlocalhrq.req_body[i].value);
			continue;
		}
		if (strcmp(threadlocalhrq.req_body[i].key,"tittle") == 0){
			char* decodedtitle= malloc(sdslen(threadlocalhrq.req_body[i].value)+1);
			memset(decodedtitle,0, sdslen(threadlocalhrq.req_body[i].value));
			percent_decode(decodedtitle,threadlocalhrq.req_body[i].value);
			tittle=sdsnew(decodedtitle);
			free(decodedtitle);
			continue;
		}
		if (strcmp(threadlocalhrq.req_body[i].key,"category") == 0){
			category=sdsnew(threadlocalhrq.req_body[i].value);
			continue;
		}
		
	}
	insert_post(tittle, category, urldecoded);
	printf("urldecoded: %s\n",urldecoded);
	free(urldecoded);
	sdsfree(tittle);
	sdsfree(category);

	//reset the cache
	//yup if someone getting data from cache in an other thread while that is uninitialized
	//this will be race condition and undefined behaviour
	pthread_mutex_lock(&cache_locker_mutex);
	globalfree_cache();
	globalinit_cache();
	pthread_mutex_unlock(&cache_locker_mutex);


return sdsnew("Saved! Yaay, Backend C u! :)");
	}
/*A test page for showing how get parameters and template rendering works.*/
sds adminroute(void){

	sds cachedcontent = serve_from_cache();
	if (cachedcontent != NULL) return cachedcontent;
	
	sds response = setresponsecode(okcode); //means HTTP/1.1 200 OK
	addheader(&response, "Connection", "Closed");
	addheader(&response, "Content-Type", "text/html");
	//WARNING it's for deflate test
		addheader(&response, "Content-Encoding", "deflate\r\n");

	
	Flate *f = NULL;
    flateSetFile(&f, "frontend/templates/temp.html");
	flateSetVar(f, "titlezone", "");
	flateSetVar(f, "title", "DYNAMIC");

	if(0) flateSetVar(f, "arch", "");
	else  flateSetVar(f, "steampunk", "");
	
	flateSetVar(f, "urlsource", "/asd");
	flateSetVar(f, "listelem", "first elem");
	flateDumpTableLine(f, "ullist");

	flateSetVar(f, "urlsource", "/nope");
	flateSetVar(f, "listelem", "second elem");
	flateDumpTableLine(f, "ullist");

	flateSetVar(f, "urlsource", "/admin");
	flateSetVar(f, "listelem", "third elem");
	flateDumpTableLine(f, "ullist");

	for(int i=0; i<threadlocalhrq.bodycount; i++){
	flateSetVar(f, "key", threadlocalhrq.req_body[i].key);
	flateSetVar(f, "value", threadlocalhrq.req_body[i].value);
	flateDumpTableLine(f, "parameters");
	}
	
	char *buf = flatePage(f);
	sds dynpage =sdsnew(buf);
	free(buf);
	flateFreeMem(f);

		//cookie example
	sds testcookiename=sdsnew("test");
	sds cookievalue = get_cookie_by_name(testcookiename);
	if (sdslen(cookievalue) == 0 ) cookievalue = sdscat(cookievalue,"NOTFOUND");
	dynpage = sdscat(dynpage, "<br><h3>Cookies:</h3><br>");
	dynpage = sdscatsds(dynpage, testcookiename);
	dynpage = sdscat(dynpage, ": ");
	dynpage = sdscatsds(dynpage, cookievalue);
	sdsfree(testcookiename);
	sdsfree(cookievalue);

	//deflate test
	int compresslen = 0;
	char* compressed_data = malloc(sdslen(dynpage)+1);
	compress_content(dynpage, sdslen(dynpage), compressed_data, &compresslen);
	
	sdsfree(dynpage);
	response = sdscatlen(response, compressed_data, compresslen);
	free(compressed_data);
	
	//add_to_cache(response);
	return response;
	}

/*detecting path traversal, ".." means some hacky thing*/
int path_traversal(void){
	//printf("traversal_check: %s\n",threadlocalhrq.url);
	for (int traversal=1; traversal< sdslen(threadlocalhrq.url);traversal++){
		if (threadlocalhrq.url[traversal]==threadlocalhrq.url[traversal-1] && threadlocalhrq.url[traversal]=='.'){
			puts("path traversal found!");
			return 1;}
			}
	return 0;
	}

//looking for static page in cache
sds serve_from_cache(void){
	for(int i=0; i<100; i++){
		if (sdscmp(cache.cachedpages[i].key,threadlocalhrq.rawurl)==0){
			printf("%s\n","Served from cache");
			return sdsdup(cache.cachedpages[i].value);
		}
	}
return NULL;
}

void add_to_cache(sds content){
		pthread_mutex_lock(&cache_locker_mutex);
		//clear old values
		sdsfree(cache.cachedpages[cache.counter].key);
		sdsfree(cache.cachedpages[cache.counter].value);
		//add new ones
		cache.cachedpages[cache.counter].key=sdsdup(threadlocalhrq.rawurl);
		cache.cachedpages[cache.counter].value=sdsdup(content);
		cache.counter++;
		
		//ring buffer turns, don't let overflow the cache
		if (cache.counter>99) cache.counter=0;
		pthread_mutex_unlock(&cache_locker_mutex);

}
/*Serve the static files from filesystem to cache and to user
 * returns the content of the file*/
sds initdir_for_static_files(void){

	//check traversal
	if (path_traversal() == 1) {
			int compresslen = 0;
			char *notthis="Go away hacker, I see you trying.";
			char* compressed_data = malloc(strlen(notthis)+1);
			compress_content(notthis, strlen(notthis)+1, compressed_data, &compresslen);
			sds ret = sdsnewlen(compressed_data, compresslen);
			free(compressed_data);
			return  ret;
		};

	//converting url path to filesystem path
	sds paramtrimm = sdsnew(ROOTPATH); // THIS is webroot, don't keep secure things here...
	paramtrimm = sdscatsds(paramtrimm,threadlocalhrq.url);
	//printf("fullpath: %s\n",paramtrimm);

	sds s= serve_from_cache();
	if (s!=NULL){sdsfree(paramtrimm); return s;}

		//if not found in cache, serve from file system
		int c=0;
		
		FILE* f = fopen(paramtrimm, "rb");
		if (f == NULL)
			{
			sdsfree(paramtrimm);
			int compresslen = 0;
			char *notthis="It is place for 404! But it's a quite bad place :(";
			char* compressed_data = malloc(strlen(notthis)+10); //TODO need a normal solution for negative deflate performance
			compress_content(notthis, strlen(notthis)+1, compressed_data, &compresslen);
			sds ret = sdsnewlen(compressed_data, compresslen);
			free(compressed_data);
			return  ret;
			}
			s = sdsempty();
		while((c = fgetc(f)) != EOF){
			s=sdscatlen(s, &c, 1);
		}
		fclose(f);

		//deflate the query
		int compresslen = 0;
		char* compressed_data = malloc(sdslen(s)+1);
		compress_content(s, sdslen(s), compressed_data, &compresslen);
		sdsfree(s);
		s=sdsnewlen(compressed_data, compresslen);
		free(compressed_data);
		//anything happens, put the page into cache
		add_to_cache(s);
		sdsfree(paramtrimm);
		return s;
		
}

/*route for the index page*/
sds rootroute(void){

	//sds cachedcontent = serve_from_cache();
	//if (cachedcontent != NULL) return cachedcontent;
	
	sds response = setresponsecode(okcode); //means HTTP/1.1 200 OK
	addheader(&response, "Connection", "Closed");
	addheader(&response, "Content-Type", "text/html");
	addheader(&response, "Content-Encoding", "deflate\r\n");
	sdsfree(threadlocalhrq.url);
	threadlocalhrq.url = sdsnew("index.html");
	sds content = initdir_for_static_files();
	response = sdscatsds(response, content);
	sdsfree(content);
	//add_to_cache(response);
	return  response;
	}

/*register the routes here*/
void controllercall(){
	sds sec = sdsnew("admin");
	sds root = sdsnew("");
	sds save = sdsnew("admin/save");
	sds categ = sdsnew("listincategory");
	sds onepost = sdsnew("onepost");

	create_route(categ, &listincategoryroute);
	create_route(sec, &adminroute);
	create_route(root, &rootroute);
	create_route(save, &saveroute);
	create_route(onepost, &onepostroute);

	sdsfree(sec);
	sdsfree(root);
	sdsfree(save);
	sdsfree(categ);
	sdsfree(onepost);
}
