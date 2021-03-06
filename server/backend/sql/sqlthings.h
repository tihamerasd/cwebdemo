#ifndef SQLTHINGS_H
#define SQLTHINGS_H

#include "sqlite3.h"
#include <stdio.h>
#include "../dynamic_string/sds.h"
#include "../keyvalue.h"
#include "threads.h"
#include "../../../dev/config.h"
#include "../../../dev/db/sql_queries.h"

sqlite3 *db;
extern char *mysql_err_msg;
extern thread_local keyvaluepair kvp_array_sqldata[MAXPOSTSSHOWN];
extern thread_local int selectcounter;

int sqlite_init_function(void);
void sqlite_close_function(void);

void insert_post(sds, sds, sds, char*, char*);

int percent_decode(char* out, const char* in);
void init_callback_sql(void);
void free_callback_sql(void);

#endif //SQLTHINGS_H
