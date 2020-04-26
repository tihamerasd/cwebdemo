#ifndef SQLQUERIES_H
#define SQLQUERIES_H

#include "../../server/backend/sql/sqlite3.h"
#include "../../server/backend/sql/sqlthings.h"
#include <stdio.h>
#include "../../server/backend/dynamic_string/sds.h"
#include "../../server/backend/keyvalue.h"
#include "threads.h"
#include "../../dev/config.h"

void select_by_name_hu(char*);	
void select_by_category_hu(char*);	
void select_by_name_en(char*);	
void select_by_category_en(char*);	
void select_top5_by_category_en(char*);	
void select_top5_by_category_hu(char*);

#endif
