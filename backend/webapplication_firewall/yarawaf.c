/*You need to install yara and libyara to use this*/
#include "yarawaf.h"

//Add your yara rules here to mitigate attacks
//do it carefully, this will slow down the server...
//This will be used to input hook, but u can do the same on the output side. (like xss filtering)
//do something similar in "yarafunction" to compile the rules
//int pass2 =yr_compiler_add_string(compiler, rule2, NULL);
//	if(pass2==0) puts("compiled");

//TODO it's a little bit uncomfortable, write a loop to read rules from array :)
char* rule="rule ExampleRule"
				"{"
					"strings:"
					"$txt_here = \"txt here\""
					""
					"condition:"
					"$txt_here"
				"}";
				
char* rule2="rule security_rule"
				"{"
					"strings:"
					"$passwd = \"/etc/passwd\""
					"$traversal = \"../\""
					""
					"condition:"
					"$passwd or $traversal"
				"}";



thread_local int match=0;

int test_max_match_data_callback(int message, void* message_data, void* user_data)
{		
	  if (message == CALLBACK_MSG_RULE_MATCHING) {match = 1; return CALLBACK_ABORT;}
	  return CALLBACK_CONTINUE;
}


int yarafunction(char* input, int len){
	YR_COMPILER* compiler = NULL;
	YR_RULES* rules = NULL;
	YR_SCANNER* scanner = NULL;
	int matches = 0;

	yr_initialize();
	yr_compiler_create(&compiler);
	
	int pass =yr_compiler_add_string(compiler, rule, NULL);
	int pass2 =yr_compiler_add_string(compiler, rule2, NULL);
	//if(pass2==0) puts("compiled"); //check if the rule is good

	yr_compiler_get_rules(compiler, &rules);
	yr_scanner_create(rules, &scanner);
	matches = yr_rules_scan_mem(rules, (uint8_t *) input, len, SCAN_FLAGS_FAST_MODE, test_max_match_data_callback, &matches, 0);
	//how to do this with files
	//matches = yr_rules_scan_file(rules, "random.txt", SCAN_FLAGS_FAST_MODE, test_max_match_data_callback, &matches, 0);
	
	yr_scanner_destroy(scanner);
	yr_rules_destroy(rules);
	yr_compiler_destroy(compiler);
	yr_finalize();

	return match;
}
