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

sds onepostroute(void){
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
	return response;
}

sds listincategoryroute(void){
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
	
return sdsnew("Saved! Yaay, Backend C u! :)");
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
