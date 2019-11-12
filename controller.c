#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <ctype.h>

//Maybe routes should be sds too, i have some speed issue with sds
char* asdroute(http_request* hrq){
	printf("%s\n", "The ASD route called");
return hrq->url;
	}
char* adminroute(http_request* hrq){
	printf("%s\n", "admin route called");
	char* testtext = "Hello, it's not ready admin page.";
	return testtext;
	}


sds initdir_for_static_files(char* url){
	char fullpath[300];
	strcpy(fullpath, "/home/tihi/cweb");
	strcat(fullpath,url);
	printf("%s\n",fullpath);

	sds extension = sdsnew(fullpath);
	sdsrange(extension,sdslen(extension)-4,sdslen(extension));
	//if (strcmp(extension,".png")==0) return sdsnew("sorry pictures not supported");

	FILE* f = fopen(fullpath, "rb");
	if (f == NULL)
   {
      perror("Error while opening the file.\n");
      sds r=sdsnew("It's place for 404!");
      return r;
   }
    int c, size;
    //TODO file inclusion here...
	f = fopen(fullpath, "rb");
	//while((c = fgetc(f)) != EOF){size++;}
	//fclose(f);
	//f=fopen(fullpath, "rb");
	sds s = sdsempty();
	while((c = fgetc(f)) != EOF){
		//printf("%d\n", c);
		//if (c=='\x00') c=='\x01'; //TODO dirty hack for handle picture as string
		//if (c=='\xff') c=='\x41'; //TODO it can makes big troubles;
		s=sdscatlen(s, &c, 1);
		}
	fclose(f);
	return s;
}

void controllercall(){
	create_route("/asd", &asdroute);
	create_route("/admin", &adminroute);
}
