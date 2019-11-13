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
	FILE* f = fopen(paramtrimm, "rb");
	if (f == NULL)
   {
      //perror("Error while opening the file.\n");
	   //sdsfreesplitres(tokens,count);
	   sdsfree(paramtrimm);
       return sdsnew("It's place for 404!"); 
   }

	printf("splittedpath: %s\n", paramtrimm);
    //TODO file inclusion here... this solution not works
    if(strstr(paramtrimm, "../") == 0){paramtrimm=sdstrim(paramtrimm,".");}
	f = fopen(paramtrimm, "rb");
	fseek(f, 0, SEEK_END);				//seek to the end of the tfile
	int file_size = ftell(f);				//get the byte offset of the pointer(the size of the file)
	fseek(f, 0, SEEK_SET);				//go back
	//sds s = sdsempty();
	char* data_file;
	data_file = (char *)malloc(file_size + 1);	//Little bit overkill
	fread(data_file, 1,file_size, f); //read
	data_file[file_size] = '\0';
	sds s = sdsnewlen(data_file,file_size);
	free(data_file);
	//sds s = sdsempty();
	//while((c = fgetc(f)) != EOF){
	//	s=sdscatlen(s, &c, 1);
	//	}
	fclose(f);
	//sdsfreesplitres(tokens,count);
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
