#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <ctype.h>

//Maybe routes should be sds too, i have some speed issue with sds
sds asdroute(http_request* hrq){
	printf("%s\n", "The ASD route called");
	sds a=sdsnew("I'm asd here: ");
return a=sdscat(a,hrq->url);
	}
sds adminroute(http_request* hrq){
	printf("%s\n", "admin route called");
	return sdsnew("Hello, it's not ready admin page.");
	}


sds initdir_for_static_files(sds url){
	sds fullpath = sdsnew("/home/tihi/cweb"); // THIS is webroot, don't keep secure things here...
	fullpath = sdscatsds(fullpath,url);
	//printf("%s\n",fullpath);
    int c, size;
    sds paramtrimm;
	int count, j;
	paramtrimm = sdssplitnth(fullpath,sdslen(fullpath),"?",1,&count,0);
	sdsfree(fullpath);

//looking for cache
	for(int i=0; i<100; i++){
		if (sdscmp(cache.cachedpages[i].key,paramtrimm)==0){
			printf("%s\n","Served from cache");
			return sdsdup(cache.cachedpages[i].value);
		}
	}

		FILE* f = fopen(paramtrimm, "rb");
		if (f == NULL)
			{
			//perror("Error while opening the file.\n");
			//sdsfreesplitres(tokens,count);
			sdsfree(paramtrimm);
			return sdsnew("It's place for 404!"); 
			}

		printf("splittedpath: %s\n", paramtrimm);
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

void controllercall(){
	sds first = sdsnew("/asd");
	sds sec = sdsnew("/admin");
	create_route(first, &asdroute);
	create_route(sec, &adminroute);
	sdsfree(first);
	sdsfree(sec);
}
