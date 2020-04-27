#ifndef CONFIG_H
#define CONFIG_H
//HTTP server means EPOLL and KQUEUE
//HTTPS server means goserver

//PORT
#define PORT 8000

//DOCUMENT ROOT
#define ROOTPATH "/home/tihi/cweb/dev/frontend/"

//QUERY CACHE SIZE (defines how many queries stored in the Cache ringbuffer)
#define CACHESIZE 100

//DATABASE FILE
#define DATABASEFILE "/home/tihi/cweb/dev/db/database.db"
#define MAXPOSTSSHOWN 100

/** only for HTTP servers **/
//SOCKET READ SIZE 
#define MAXBUF  4096*32
//MAXIMUM NUMBER of EPOLL/kqueue EVEVENTS 
#define MAX_EVENTS 1000

/** only for HTTPS server **/
//CERTFILE 
#define CERTFILE "/home/tihi/cweb/certificate.pem"
//KEYFILE 
#define KEYFILE "/home/tihi/cweb/key.pem"


//REQUEST OBJECT INTERNAL
//MAXIMUM NUMBER OF http header on overflow last ones will be ignored.
//also means maximum number of GET variables
#define MAX_LIST_LENGTH 100

//INIT OBJECTS INTERNAL
//HTTP MIME TYPE EXTENSIONS MAXIMUM
//this don't do anything while you not define more file extensions, in function void init_file_extension(void)
// if you want more mime types, you need to edit, that and sync with this
#define MAX_FILE_EXTENSIONS 10

// DEFLATE COMPRESSION LEVEL: number between 0-9
//0 means bad compress rate but fast computing
//9 means best compress rate but slower to calculate that
#define COMPRESSRATE 9

//DYNAMIC urls, if you have more dynamic url, the app will crash
#define ROUTEARRAYSIZE 200

//WEB APPLICATION FIREWALL SETTINGS
#define BANNLEN 3 //number of banned words
//array to bann
#define BANNTHIS "../","..%2F","templates/"

#endif
