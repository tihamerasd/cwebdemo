#include "sqlthings.h"

char *mysql_err_msg = 0;
thread_local keyvaluepair kvp_array_sqldata[MAXPOSTSSHOWN];
thread_local int selectcounter=0;

/*URL_decode function*/
int percent_decode(char* out, const char* in) {
    static const char tbl[256] = {
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
         0, 1, 2, 3, 4, 5, 6, 7,  8, 9,-1,-1,-1,-1,-1,-1,
        -1,10,11,12,13,14,15,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,10,11,12,13,14,15,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1
    };
    char c, v1, v2, *beg=out;
    if(in != NULL) {
        while((c=*in++) != '\0') {
            if(c == '%') {
                if((v1=tbl[(unsigned char)*in++])<0 || 
                   (v2=tbl[(unsigned char)*in++])<0) {
                    *beg = '\0';
                    return -1;
                }
                c = (v1<<4)|v2;
            }
            *out++ = c;
        }
    }
    *out = '\0';
    return 0;
}
/*TODO sqlite really threadsafe? It's can be fatal in high pressure*/
/*set the thread local values connected to sql*/
void init_callback_sql(void){
	for(int i=0; i<MAXPOSTSSHOWN; i++){
		kvp_array_sqldata[i].key=NULL;
		kvp_array_sqldata[i].value=NULL;
	}
	selectcounter=0;
}

/*clear sql connected thread global variables*/
void free_callback_sql(void){
	for(int i=0; i<MAXPOSTSSHOWN; i++){
		sdsfree(kvp_array_sqldata[i].key);
		sdsfree(kvp_array_sqldata[i].value);
		selectcounter =0;
	}
}

/*set up the connection -> no socket, so just read the file.*/
int sqlite_init_function(void){
    int rc = sqlite3_open(DATABASEFILE, &db);
    
    if (rc != SQLITE_OK) {
        
        fprintf(stderr, "Cannot open database: %s\n", 
                sqlite3_errmsg(db));
        sqlite3_close(db);
        
        return 1;
    }
return 0;
}



/*close the sql connection*/
void sqlite_close_function(void){
sqlite3_close(db);
}
