#include <stdio.h>      // printf
#include <sqlite3.h>    // SQLite header (from /usr/include)

#include "bcrypt.c"

int main()
{
    sqlite3 *db;        // database connection
    int rc;             // return code
    char *errmsg;       // pointer to an error string

    /*
     * open SQLite database file test.db
     * use ":memory:" to use an in-memory database
     */
    rc = sqlite3_open(":memory:", &db);
    if (rc != SQLITE_OK) {
        printf("ERROR opening SQLite DB in memory: %s\n", sqlite3_errmsg(db));
        goto out;
    }
    printf("opened SQLite handle successfully.\n");

char* salt = bcrypt_gensalt(12);
char* hash = bcrypt("123456", salt);

    return 0;

out:
    /*
     * close SQLite database
     */
    sqlite3_close(db);
    printf("database closed.\n");
}
