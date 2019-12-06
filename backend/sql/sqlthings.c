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

/*DANGER be carefull, the kvp_array is an array of keyvalue pairs, but the return object is 3 piece
 of key-value object, need too loop it with i+=3 and handle inside
 Once I should refactor...*/
int callback(void *NotUsed, int argc, char **argv, 
                    char **azColName) {
    
    NotUsed = 0;
    
    for (int i = 0; i < argc; i++) {

        //printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
        kvp_array_sqldata[selectcounter++] = createkeyvalue(azColName[i], argv[i] ? argv[i] : "NULL");
		
    }
    //if(selectcounter<MAXPOSTSSHOWN-1) selectcounter++;
    printf("\n");
    
    return 0;
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

/*Insert anarticle from the admin page to the database*/
void insert_post(sds tittle, sds category, char* content){
	char *sql = sqlite3_mprintf("INSERT INTO posts (tittle,category,content) VALUES( '%q','%q','%q');", tittle,category,content);   
    int rc = sqlite3_exec(db, sql, 0, 0, &mysql_err_msg);
    if (rc != SQLITE_OK ) {  
        fprintf(stderr, "Failed to select data\n");
        fprintf(stderr, "SQL error: %s\n", mysql_err_msg);
         puts("ERROR: Save FAIL!\n");
        sqlite3_free(mysql_err_msg);
        return;
    }
    puts("Save success!\n");
    sqlite3_free(sql);
	}
	
/*Select the articles by category*/
void select_by_category(char* category){
	char *sql = sqlite3_mprintf("SELECT tittle,category,content from posts WHERE category='%q'", category);   
    int rc = sqlite3_exec(db, sql, callback, 0, &mysql_err_msg);
    if (rc != SQLITE_OK ) {  
        fprintf(stderr, "Failed to select data\n");
        fprintf(stderr, "SQL error: %s\n", mysql_err_msg);
        sqlite3_free(mysql_err_msg);
        return;
    }
    
    sqlite3_free(sql);
	}

/*Select the articles by name*/
void select_by_name(char* tittle){
	char *sql = sqlite3_mprintf("SELECT tittle,category,content from posts WHERE tittle='%q'", tittle);   
    int rc = sqlite3_exec(db, sql, callback, 0, &mysql_err_msg);
    if (rc != SQLITE_OK ) {  
        fprintf(stderr, "Failed to select data\n");
        fprintf(stderr, "SQL error: %s\n", mysql_err_msg);
        sqlite3_free(mysql_err_msg);
        return;
    }
    
    sqlite3_free(sql);
	}

/*close the sql connection*/
void sqlite_close_function(void){
sqlite3_close(db);
}
