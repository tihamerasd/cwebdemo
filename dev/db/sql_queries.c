#include "sql_queries.h"

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
    return 0;
}

/*Insert anarticle from the admin page to the database*/
void insert_post(sds title_hun, sds title_en, sds category, char* content_hun, char* content_en){
	char *sql = sqlite3_mprintf(
	"INSERT INTO posts (title_HUN,title_EN,category,content_HUN, content_EN,created_at) VALUES( '%q','%q','%q', '%q', '%q', CURRENT_TIMESTAMP);",
			title_hun,title_en,category,content_hun,content_en);   
    int rc = sqlite3_exec(db, sql, 0, 0, &mysql_err_msg);
    if (rc != SQLITE_OK ) {  
        fprintf(stderr, "Failed to select data\n");
        fprintf(stderr, "SQL error: %s\n", mysql_err_msg);
         puts("ERROR: Save FAIL!\n");
        sqlite3_free(mysql_err_msg);
        sqlite3_free(sql);
        return;
    }
    puts("Save success!\n");
    sqlite3_free(sql);
	}
	
/*Select the articles by category*/
void select_by_category_en(char* category){
	char *sql = sqlite3_mprintf("SELECT title_EN,category,content_EN,created_at from posts WHERE category='%q'", category);   
    int rc = sqlite3_exec(db, sql, callback, 0, &mysql_err_msg);
    if (rc != SQLITE_OK ) {  
        fprintf(stderr, "Failed to select data\n");
        fprintf(stderr, "SQL error: %s\n", mysql_err_msg);
        sqlite3_free(mysql_err_msg);
        sqlite3_free(sql);
        return;
    }
    
    sqlite3_free(sql);
	}

/*Select the articles by category*/
void select_by_category_hu(char* category){
	char *sql = sqlite3_mprintf("SELECT title_HUN,category,content_HUN,created_at from posts WHERE category='%q'", category);   
    int rc = sqlite3_exec(db, sql, callback, 0, &mysql_err_msg);
    if (rc != SQLITE_OK ) {  
        fprintf(stderr, "Failed to select data\n");
        fprintf(stderr, "SQL error: %s\n", mysql_err_msg);
        sqlite3_free(mysql_err_msg);
        sqlite3_free(sql);
        return;
    }
    
    sqlite3_free(sql);
	}


/*Select the articles by name*/
void select_by_name_en(char* tittle){
	char *sql = sqlite3_mprintf("SELECT title_EN,category,content_EN,created_at from posts WHERE title_EN='%q'", tittle);   
    int rc = sqlite3_exec(db, sql, callback, 0, &mysql_err_msg);
    if (rc != SQLITE_OK ) {  
        fprintf(stderr, "Failed to select data\n");
        fprintf(stderr, "SQL error: %s\n", mysql_err_msg);
        sqlite3_free(mysql_err_msg);
        sqlite3_free(sql);
        return;
    }
    
    sqlite3_free(sql);
	}

/*Select the articles by name*/
void select_by_name_hu(char* tittle){
	char *sql = sqlite3_mprintf("SELECT title_HUN,category,content_HUN,created_at from posts WHERE title_HUN='%q'", tittle);   
    int rc = sqlite3_exec(db, sql, callback, 0, &mysql_err_msg);
    if (rc != SQLITE_OK ) {  
        fprintf(stderr, "Failed to select data\n");
        fprintf(stderr, "SQL error: %s\n", mysql_err_msg);
        sqlite3_free(mysql_err_msg);
        sqlite3_free(sql);
        return;
    }
    
    sqlite3_free(sql);
	}

/*Select the articles by category*/
void select_top5_by_category_hu(char* category){
	char *sql = sqlite3_mprintf("SELECT title_HUN,category,content_HUN,created_at from posts WHERE category='%q' ORDER BY date(created_at) DESC LIMIT 5;", category);   
    int rc = sqlite3_exec(db, sql, callback, 0, &mysql_err_msg);
    if (rc != SQLITE_OK ) {  
        fprintf(stderr, "Failed to select data\n");
        fprintf(stderr, "SQL error: %s\n", mysql_err_msg);
        sqlite3_free(mysql_err_msg);
        sqlite3_free(sql);
        return;
    }
    
    sqlite3_free(sql);
	}


/*Select the articles by category*/
void select_top5_by_category_en(char* tittle){
	char *sql = sqlite3_mprintf("SELECT title_EN,category,content_EN,created_at FROM posts WHERE category='%q' ORDER BY date(created_at) DESC LIMIT 5;", tittle);   
    int rc = sqlite3_exec(db, sql, callback, 0, &mysql_err_msg);
    if (rc != SQLITE_OK ) {  
        fprintf(stderr, "Failed to select data\n");
        fprintf(stderr, "SQL error: %s\n", mysql_err_msg);
        sqlite3_free(mysql_err_msg);
        sqlite3_free(sql);
        return;
    }
    
    sqlite3_free(sql);
	}
