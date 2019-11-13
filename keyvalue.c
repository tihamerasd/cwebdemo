#include "keyvalue.h"

keyvaluepair create_keyvaluepair(sds key, sds value){
	keyvaluepair kvp;
	return kvp;
	}

keyvaluepair create_keyvalue_from_header(sds headerline){
	char delim[] = ":";
	keyvaluepair kvp;

	return kvp;
	}

sds sdssplitnth(sds l, int len, char* exp, int lenexp, int *count, int nth){
sds *tokens;
int  j;

tokens = sdssplitlen(l,sdslen(l),exp,1,count);

for (j = 0; j < *count; j++)
    printf("%s\n", tokens[j]);
sds ret = sdsdup(tokens[nth]);
sdsfreesplitres(tokens,*count);
return ret;
}
