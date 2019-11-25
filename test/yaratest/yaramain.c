/*You need to install yara and libyara to use this*/
#include "stdio.h"
#include "stdlib.h"
#include <yara.h>

int match=0;

int test_max_match_data_callback(int message, void* message_data, void* user_data)
{		
	  if (message == CALLBACK_MSG_RULE_MATCHING) {match = 1; return CALLBACK_ABORT;}
	  return CALLBACK_CONTINUE;
}


int yarafunction(char* input){
	YR_COMPILER* compiler = NULL;
	YR_RULES* rules = NULL;
	YR_SCANNER* scanner = NULL;

	char* rule="rule ExampleRule"
				"{"
					"strings:"
					"$txt_here = \"txt here\""
					"$hexstring = { E2 34 A1 C8 23 FB }"
					""
					"condition:"
					"$txt_here and $hexstring"
				"}";
				
	char* rule2="rule testrule"
				"{"
					"strings:"
					"$my_text_string = \"test example\""
					"$A_bytes = { aa aa aa aa }"
					""
					"condition:"
					"$my_text_string or $A_bytes"
				"}";

	int matches = 0;

	yr_initialize();
	yr_compiler_create(&compiler);
	
	int pass =yr_compiler_add_string(compiler, rule, NULL);
	int pass2 =yr_compiler_add_string(compiler, rule2, NULL);
	if(pass2==0) puts("compiled");

	yr_compiler_get_rules(compiler, &rules);
	yr_scanner_create(rules, &scanner);
	matches = yr_rules_scan_mem(rules, (uint8_t *) input, strlen(input), SCAN_FLAGS_FAST_MODE, test_max_match_data_callback, &matches, 0);
	matches = yr_rules_scan_file(rules, "random.txt", SCAN_FLAGS_FAST_MODE, test_max_match_data_callback, &matches, 0);
	
	yr_scanner_destroy(scanner);
	yr_rules_destroy(rules);
	yr_compiler_destroy(compiler);
	yr_finalize();

	return match;
}

int main(){
	char* target="omg random things here text here more random things";
	printf("target: %s\n",target);
	int result = yarafunction(target);
	if (result == 1) puts("yara said: MATCH\n"); else puts("yara said: NOTHING\n");
	match=0;
	
	char* target2="you never catch me yara lol";
	printf("target2: %s\n",target2);
	result = yarafunction(target);
	if (result == 1) puts("yara said: MATCH\n"); else puts("yara said: NOTHING\n");
	match=0;

	char* target3= "asdqwe""\xa1""\x42""\xe2""sdfsdf""\x98""cxv""\xe2""\x34""\xA1""\xc8""\x23""\xf""bqwe123""\x12""\xdf";
	printf("target3: %s\n",target3);
	result = yarafunction(target3);
	if (result == 1) puts("yara said: MATCH\n"); else puts("yara said: NOTHING\n");
	match=0;
	
	char* target4= "asdqwe""\xa1""\x42""\xe""2sdfsdf""\x98""c""\xaa""\xa""a""\xaa""\xaa""v\xe2""\x34""\xA1""\xc8""\x23""\xfb""qwe123""\x12""\xdf";
	printf("target4: %s\n",target4);
	result = yarafunction(target4);
	if (result == 1) puts("yara said: MATCH\n"); else puts("yara said: NOTHING\n");
	match=0;
	
	char* target5= "asdqwe""\xa1""\xe2""sdfsdf""\x98""c""\xaa""\xa""\xaa""ov""\xe2""\x34""\xA1""\xc8""\x23""\xf""bqwe123""\x12""\xdf";
	printf("target5: %s\n",target5);
	result = yarafunction(target5);
	if (result == 1) puts("yara said: MATCH\n"); else puts("yara said: NOTHING\n");
	match=0;
	
	char* target6= "qweqweqwe";
	printf("target6: %s\n",target6);
	result = yarafunction(target6);
	if (result == 1) puts("yara said: MATCH\n"); else puts("yara said: NOTHING\n");
	match=0;
	
return 0;
}
