#include "keyvalue.h"

keyvaluepair create_keyvaluepair(char* key, char* value){
	keyvaluepair kvp;
	if (strlen(key)<50) {strcpy(kvp.key,key); kvp.key_len=strlen(key);} else printf("%s\n","keyvaluepair overflow");
	if (strlen(value)<300){strcpy(kvp.value,value); kvp.value_len=strlen(value);} else printf("%s\n","keyvaluepair overflow");
	return kvp;
	}

keyvaluepair create_keyvalue_from_header(char* headerline){
	char delim[] = ":";
	keyvaluepair kvp;

	char *ptr = strtok(headerline, delim);
	if (strlen(ptr)>10) {strcpy(kvp.key,"OVERFLOW"); kvp.key_len=8;}
	else {strcpy(kvp.key, ptr); kvp.key_len=(strlen(ptr));}

		ptr = strtok(NULL, delim);
	if (strlen(ptr)>10) {strcpy(kvp.value,"OVERFLOW"); kvp.value_len=8;}
	else {strcpy(kvp.value, ptr); kvp.value_len=strlen(ptr);}
	return kvp;
	}

//char* get_value(keyvaluepair kvp){
	//if (strlen(key)>50) return "OVERFLOW";
	//return kvp.value;
	//}
