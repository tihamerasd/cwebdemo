#include "controller.h"

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
	for(int i=0; i<CACHESIZE; i++){
		if (sdscmp(cache.cachedpages[i].key,threadlocalhrq.rawurl)==0){
			//printf("%s\n","Served from cache");
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
		if (cache.counter>CACHESIZE-1) cache.counter=0;
		pthread_mutex_unlock(&cache_locker_mutex);

}

int isDirectory(const char *path)
{
     struct stat status;
     stat(path, &status);
   
     if (S_ISDIR(status.st_mode))
        return 1;
   
    return 0;
   
}

/*Serve the static files from filesystem to cache and to user
 * returns the content of the file*/
sds initdir_for_static_files(){

	//check traversal
	if (path_traversal() == 1) return  NULL;

	//converting url path to filesystem path
	sds paramtrimm = sdsnew(ROOTPATH); // THIS is webroot, don't keep secure things here...
	paramtrimm = sdscatsds(paramtrimm,threadlocalhrq.url);
	//printf("fullpath: %s\n",paramtrimm);

	if (isDirectory(paramtrimm)) {
		sdsfree(paramtrimm);
		return NULL;
		}


	sds s= serve_from_cache();
	if (s!=NULL){sdsfree(paramtrimm); return s;}


		FILE *fileptr;
		char *buffer;
		long filelen;
		fileptr = fopen(paramtrimm, "rb");
		if (fileptr == NULL)
			{
			sdsfree(paramtrimm);
			return  NULL;
			}

		fseek(fileptr, 0, SEEK_END);          			// Jump to the end of the file
		filelen = ftell(fileptr);             			// Get the current byte offset in the file
		rewind(fileptr);                      			// Jump back to the beginning of the file
		buffer = (char *)malloc(filelen * sizeof(char));// Enough memory for the file
		fread(buffer, filelen, 1, fileptr); 			// Read in the entire file
		fclose(fileptr); // Close the file
		s=sdsnewlen(buffer, filelen);
		free(buffer);

		//deflate the query
		int compresslen = 0;
		char* compressed_data = malloc(sdslen(s)+1);
		compress_content(s, sdslen(s), compressed_data, &compresslen);
		sdsfree(s);
		s=sdsnewlen(compressed_data, compresslen);
		free(compressed_data);
		//static files are cacheable
		add_to_cache(s);
		sdsfree(paramtrimm);
		return s;
		
}


