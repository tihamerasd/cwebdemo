#ifndef routes_H
#define routes_H

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <ctype.h>
#include "../../server/backend/html_templater/flate.h"
#include "../../server/backend/sql/sqlthings.h"
#include "../../server/backend/../../dev/config.h"
#include "../../server/backend/responser.h"
#include <openssl/sha.h>
#include <sys/stat.h>

int simpleSHA512(void*, unsigned long, unsigned char*);	
sds onepostroute(void);	
sds listincategoryroute(void);	
sds saveroute(void);	
sds rootroute(void);	
sds ifconfigroute(void);	
sds languageroute(void);	

#endif
