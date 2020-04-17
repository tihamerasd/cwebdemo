#include "simple_waf.h"
#define _GNU_SOURCE
#include <string.h>
#include "../../../dev/config.h"

const char *banned[BANNLEN]={BANNTHIS};

int simple_waf(char* input, int len){
	int match=0;
	for (int i=0; i<BANNLEN; i++){
		if(memmem(input,len,banned[i],strlen(banned[i])) != NULL) match=1;
	}

	return match;
}
