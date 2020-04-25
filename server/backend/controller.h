#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <ctype.h>
#include "html_templater/flate.h"
#include "sql/sqlthings.h"
#include "../../dev/config.h"
#include "responser.h"
#include <openssl/sha.h>
#include <sys/stat.h>

sds initdir_for_static_files(void);
sds serve_from_cache(void);
void add_to_cache(sds);
int path_traversal(void);
int isDirectory(const char*);

#endif 
