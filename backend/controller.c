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

sds asdroute(){
	sds response = adddefaultheaders();
	response=sdscat(response, "<h1> it's my url! :)</h1>");
	
return response;
	}
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


sds initdir_for_static_files(void){
	//printf("%s\n", url);
	for (int traversal=0; traversal< sdslen(threadlocalhrq.url)-1;traversal++){
		if (threadlocalhrq.url[traversal]==threadlocalhrq.url[traversal+1] && threadlocalhrq.url[traversal]=='.')
			return sdsnew("Go away hacker, path traversal detected!");
			}
	puts("notraversal");
	sds fullpath = sdsnew(ROOTPATH); // THIS is webroot, don't keep secure things here...
	fullpath = sdscatsds(fullpath,threadlocalhrq.url);
	printf("fullpath: %s\n",fullpath);
    int c;
    //TODO move this to request parsing
    sds paramtrimm;
	int count;
	paramtrimm = sdssplitnth(fullpath,sdslen(fullpath),"? ",1,&count,0); //second level defense, later i should remove this
	sdsfree(fullpath);
printf("paramtrimm: %s\n",paramtrimm);
//looking for cache
	for(int i=0; i<100; i++){
		if (sdscmp(cache.cachedpages[i].key,paramtrimm)==0){
			printf("%s\n","Served from cache");
			sdsfree(paramtrimm);
			return sdsdup(cache.cachedpages[i].value);
		}
	}
	
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
		keyvaluepair cachelog;
		cachelog.key=sdsdup(paramtrimm);
		cachelog.value=sdsdup(s);
		sdsfree(cache.cachedpages[cache.counter].key);
		sdsfree(cache.cachedpages[cache.counter].value);
		cache.cachedpages[cache.counter++]=cachelog;
		if (cache.counter>100) cache.counter=0;
		sdsfree(paramtrimm);
		return s;
		
}

sds rootroute(void){
	sds response = setresponsecode(okcode); //means HTTP/1.1 200 OK
	addheader(&response, "Connection", "Closed");
	addheader(&response, "Content-Type", "text/html\r\n");
	sdsfree(threadlocalhrq.url);
	threadlocalhrq.url = sdsnew("index.html");
	sds content = initdir_for_static_files();
	response = sdscatsds(response, content);
	sdsfree(content);
	//sdsfree(index);
	return  response;
	}

void controllercall(){
	sds first = sdsnew("/asd");
	sds sec = sdsnew("/admin");
	sds root = sdsnew("/");
	create_route(first, &asdroute);
	create_route(sec, &adminroute);
	create_route(root, &rootroute);
	sdsfree(first);
	sdsfree(sec);
	sdsfree(root);
}
