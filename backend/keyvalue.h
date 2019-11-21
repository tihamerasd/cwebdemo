#ifndef keyvalue_H
#define keyvalue_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "./dynamic_string/sds.h"

typedef struct keyvaluepair{
sds key;
sds value;
} keyvaluepair;

keyvaluepair create_keyvaluepair(void);
sds get_value_by_key(sds);
keyvaluepair create_keyvalue_from_header(sds);

sds sdssplitnth(sds, int, char*, int,  int*, int );
#endif //keyvalue_H
