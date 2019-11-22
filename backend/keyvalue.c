#include "keyvalue.h"

keyvaluepair create_keyvaluepair(){
	keyvaluepair kvp;
	kvp.key=sdsempty();
	kvp.value=sdsempty();
	
	return kvp;
	}

keyvaluepair createkeyvalue(char* key,char* val){
keyvaluepair k;
k.value=sdsnew(val);
k.key=sdsnew(key);
	}

keyvaluepair create_keyvalue_from_header(sds headerline){
	//char delim[] = ":";
	keyvaluepair kvp=create_keyvaluepair();

	return kvp;
	}

void freekeyvalue(keyvaluepair k){free(k.key); free(k.value);}

//TODO what if the n-th elem not exsist in the array? it should return sdsempty
sds sdssplitnth(sds l, int len, char* exp, int lenexp, int *count, int nth){
sds *tokens;
int  j=0;

tokens = sdssplitlen(l,sdslen(l),exp,1,count);
if(tokens[j] == NULL) return sdsempty();
if (*count<=nth){    
	sdsfreesplitres(tokens,*count);
	return sdsempty();
	}

//for (j = 0; j < *count; j++)
    //printf("tokens: %s\n", tokens[j]);
sds ret = sdsdup(tokens[nth]);
sdsfreesplitres(tokens,*count);
return ret;
}
