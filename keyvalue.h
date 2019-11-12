#ifndef keyvalue_H
#define keyvalue_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "sds.h"

typedef struct keyvaluepair{
char key[50];
char value[300];
int key_len;
int value_len;
} keyvaluepair;

keyvaluepair create_keyvaluepair(char*, char*);
char* get_value_by_key(char*);
keyvaluepair create_keyvalue_from_header(char*);

#endif //keyvalue_H
