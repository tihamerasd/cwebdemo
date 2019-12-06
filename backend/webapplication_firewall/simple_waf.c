#include "simple_waf.h"


int simple_waf(char* input, int len){
	int match=0;
	if(memmem(input,len,"../",3) != NULL) match=1;
	if(memmem(input,len,"..%2F",5) != NULL) match=1;
	if(memmem(input,len,"templates/",10) != NULL) match=1;
	return match;
}
