#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <ctype.h>
#include "html_templater/flate.h"
#include "statuscodes.h"

#define ROOTPATH "frontend/"

/*basic test page*/
sds asdroute(){
	sds response = adddefaultheaders();
	response=sdscat(response, "<h1> it's my url! :)</h1>");
	
return response;
	}

/*A test page for showing how get parameters and template rendering works.*/
sds adminroute(void){
	sds response = setresponsecode(okcode); //means HTTP/1.1 200 OK
	addheader(&response, "Connection", "Closed");
	addheader(&response, "Content-Type", "text/html");
	
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
	
	response = sdscatsds(response, dynpage);
	sdsfree(dynpage);
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

/*Serve the static files from filesystem to cache and to user
 * returns the content of the file*/
sds initdir_for_static_files(void){

	//check traversal
	if (path_traversal() == 1) return sdsnew("Go away hacker!");

	//converting url path to filesystem path
	sds paramtrimm = sdsnew(ROOTPATH); // THIS is webroot, don't keep secure things here...
	paramtrimm = sdscatsds(paramtrimm,threadlocalhrq.url);
	//printf("fullpath: %s\n",paramtrimm);
	
	//looking for static page in cache
	for(int i=0; i<100; i++){
		if (sdscmp(cache.cachedpages[i].key,paramtrimm)==0){
			//printf("%s\n","Served from cache");
			sdsfree(paramtrimm);
			return sdsdup(cache.cachedpages[i].value);
		}
	}


		//if not found in cache, serve from file system
		int c=0;
		
		FILE* f = fopen(paramtrimm, "rb");
		if (f == NULL)
			{
			sdsfree(paramtrimm);
			return sdsnew("It's place for 404!"); 
			}
		sds s = sdsempty();
		while((c = fgetc(f)) != EOF){
			s=sdscatlen(s, &c, 1);
		}
		fclose(f);
		
		//anything happens, put the page into cache
		keyvaluepair cachelog;
		cachelog.key=sdsdup(paramtrimm);
		cachelog.value=sdsdup(s);
		sdsfree(cache.cachedpages[cache.counter].key);
		sdsfree(cache.cachedpages[cache.counter].value);
		cache.cachedpages[cache.counter++]=cachelog;

		//ring buffer turns, don't let overflow the cache
		if (cache.counter>100) cache.counter=0;
		sdsfree(paramtrimm);
		return s;
		
}

/*route for the index page*/
sds rootroute(void){
	sds response = setresponsecode(okcode); //means HTTP/1.1 200 OK
	addheader(&response, "Connection", "Closed");
	addheader(&response, "Content-Type", "text/html\r\n");
	sdsfree(threadlocalhrq.url);
	threadlocalhrq.url = sdsnew("index.html");
	sds content = initdir_for_static_files();
	response = sdscatsds(response, content);
	sdsfree(content);
	return  response;
	}

/*register the routes here*/
void controllercall(){
	sds first = sdsnew("asd");
	sds sec = sdsnew("admin");
	sds root = sdsnew("");
	create_route(first, &asdroute);
	create_route(sec, &adminroute);
	create_route(root, &rootroute);
	sdsfree(first);
	sdsfree(sec);
	sdsfree(root);
}
