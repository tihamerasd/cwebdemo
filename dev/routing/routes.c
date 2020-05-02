#include "routes.h"
#include "../../server/base64.h"

char* okcode ="HTTP/1.1 200 OK\r\n";

int simpleSHA512(void* input, unsigned long length, unsigned char* md)
{
    SHA512_CTX context;
    if(!SHA512_Init(&context))
        return 0;

    if(!SHA512_Update(&context, (unsigned char*)input, length))
        return 0;

    if(!SHA512_Final(md, &context))
        return 0;

    return 1;
}

sds onepostroute(void){
	//sds cachedcontent = serve_from_cache();
	//if (cachedcontent != NULL) return cachedcontent;

	char* urldecoded=NULL;
	for(int i=0; i<threadlocalhrq.bodycount; i++){
		if (strcmp(threadlocalhrq.req_body[i].key,"tittle") == 0){
			urldecoded = malloc(sdslen(threadlocalhrq.req_body[i].value)+1);
			percent_decode(urldecoded,threadlocalhrq.req_body[i].value);
			break;
		}
	}
	
	init_callback_sql();
	sds languagecookie=sdsnew("lang");
	sds cookievalue = get_cookie_by_name(languagecookie);
	Flate *f = NULL;
	sds lang=sdsnew("HUN");

	//printf("urldecoded: %s\n", urldecoded);
	
	if (sdscmp(cookievalue,lang)==0) select_by_name_hu(urldecoded);
	else select_by_name_en(urldecoded);
	free(urldecoded);

	if (sdscmp(cookievalue,lang)==0) flateSetFile(&f, ROOTPATH"templates/one_post.html");
	else flateSetFile(&f, ROOTPATH"/templates/one_post_EN.html");
	sdsfree(lang);
	sdsfree(cookievalue);
	sdsfree(languagecookie);
	
	flateSetVar(f, "contentzone", "");
	flateSetVar(f, "title", kvp_array_sqldata[0].value);
	flateSetVar(f, "category",kvp_array_sqldata[1].value );
	flateSetVar(f, "content", kvp_array_sqldata[2].value);
	flateSetVar(f, "createdat", kvp_array_sqldata[3].value);

	free_callback_sql();
	sds response = setresponsecode(okcode); //means HTTP/1.1 200 OK
	addheader(&response, "Connection", "close");
	addheader(&response, "Content-Type", "text/html");
	addheader(&response, "Strict-Transport-Security", "max-age=31536000; includeSubDomains");
	addheader(&response, "X-Frame-Options", "deny");
	addheader(&response, "X-XSS-Protection", "1; mode=block");
	addheader(&response, "X-Content-Type-Options", "nosniff");
	char *buf = flatePage(f);
	sds dynpage =sdsnew(buf);
	free(buf);
	flateFreeMem(f);
	
	response = sdscatsds(response, dynpage);
	sdsfree(dynpage);
	//add_to_cache(response);
	return response;
}

sds listincategoryroute(void){

	//sds cachedcontent = serve_from_cache();
	//if (cachedcontent != NULL) return cachedcontent;

	char* urldecoded=NULL;
	for(int i=0; i<threadlocalhrq.bodycount; i++){
		if (strcmp(threadlocalhrq.req_body[i].key,"category") == 0){
			urldecoded = malloc(sdslen(threadlocalhrq.req_body[i].value)+1);
			percent_decode(urldecoded,threadlocalhrq.req_body[i].value);
			break;
		}
	}
	init_callback_sql();
	sds languagecookie=sdsnew("lang");
	sds cookievalue = get_cookie_by_name(languagecookie);
	Flate *f = NULL;
	sds lang=sdsnew("HUN");
	
	if (sdscmp(cookievalue,lang)==0) select_by_category_hu(urldecoded);
	else select_by_category_en(urldecoded);
	free(urldecoded);

	if (sdscmp(cookievalue,lang)==0) flateSetFile(&f, ROOTPATH"templates/all_in_category.html");
	else flateSetFile(&f, ROOTPATH"templates/all_in_category_EN.html");
	sdsfree(lang);
	sdsfree(languagecookie);
	sdsfree(cookievalue);
	
	flateSetVar(f, "categorynamezone", "");
	flateSetVar(f, "categoryname", kvp_array_sqldata[1].value);

	for (int i=0; i<MAXPOSTSSHOWN; i+=4){
		if (kvp_array_sqldata[i].key!=NULL && kvp_array_sqldata[i].value!=NULL){
			sds sdsurl= sdsnew("/onepost?tittle=");
			sdsurl =sdscatsds(sdsurl, kvp_array_sqldata[i].value);

			flateSetVar(f, "urlsource",sdsurl);
			flateSetVar(f, "tittle", kvp_array_sqldata[i].value);
			flateSetVar(f, "createdat", kvp_array_sqldata[i+3].value);
			flateDumpTableLine(f, "ullist");
			sdsfree(sdsurl);
		}
	}
    
	free_callback_sql();
	sds response = setresponsecode(okcode); //means HTTP/1.1 200 OK
	addheader(&response, "Connection", "close");
	addheader(&response, "Content-Type", "text/html");
	addheader(&response, "Strict-Transport-Security", "max-age=31536000; includeSubDomains");
	addheader(&response, "X-Frame-Options", "deny");
	addheader(&response, "X-XSS-Protection", "1; mode=block");
	addheader(&response, "X-Content-Type-Options", "nosniff");
	char *buf = flatePage(f);
	sds dynpage =sdsnew(buf);
	free(buf);
	flateFreeMem(f);
	
	response = sdscatsds(response, dynpage);
	sdsfree(dynpage);
	//add_to_cache(response);
	return response;
}

sds saveroute(void){
	int authentication_exist = 0;
	//check authetntication
	for(int i=0; i<threadlocalhrq.headercount; i++){
		if(strcmp(threadlocalhrq.req_headers[i].key,"Authentication")==0) {
			authentication_exist = 1;
			//GET the hash from http
			unsigned char md[SHA512_DIGEST_LENGTH]; // 32 bytes
			void * pw_from_header = (void*) threadlocalhrq.req_headers[i].value;
			simpleSHA512(pw_from_header, sdslen(pw_from_header), md);

			char stored_pw_from_header[(SHA512_DIGEST_LENGTH*2)+1]; //array vs ptr type bypass, fixme later...
			stored_pw_from_header[SHA512_DIGEST_LENGTH*2]='\0'; 
			for (int j = 0; j < SHA512_DIGEST_LENGTH; j++) {
				sprintf(&stored_pw_from_header[j*2], "%02x", md[j]);
				}

			//get the hash from FILE
			FILE *fp;
			//read from file, so if the file length <255 no overflow
			char pwbuff[255];
			memset(pwbuff,0,254);
			fp = fopen("./dev/db/password.txt", "r");
			fscanf(fp, "%s", pwbuff);
			fclose(fp);
			if (strcmp(pwbuff, stored_pw_from_header) != 0) return sdsnew("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\nBAD PASSWORD");
		}
	}
	if (authentication_exist == 0) return sdsnew("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\nPLEASE AUTHENTICATE");
//TODO decoded base64 \0 byte cause error
	char* content_hun=NULL;
	char* content_en=NULL;
	char* title_hun = NULL;
	char* title_en = NULL;
	sds category = NULL;
	//TODO DO function for getting a  GET/POST variable
	for(int i=0; i<threadlocalhrq.bodycount; i++){
		printf(" BASE64 KEYVALUEPAIRS: %s: %s\n", threadlocalhrq.req_body[i].key, threadlocalhrq.req_body[i].value);
		int b64_len =sdslen(threadlocalhrq.req_body[i].value);
		int b64_decoded_len = 0;
		if (strcmp(threadlocalhrq.req_body[i].key,"content_hun") == 0){
			content_hun = unbase64(threadlocalhrq.req_body[i].value, b64_len, &b64_decoded_len);
			printf("KEYVALUEPAIRS: %s: %s\n", threadlocalhrq.req_body[i].key, content_hun);
			continue;
		}

		if (strcmp(threadlocalhrq.req_body[i].key,"content_eng") == 0){
			content_en=unbase64(threadlocalhrq.req_body[i].value, b64_len, &b64_decoded_len);
			printf("KEYVALUEPAIRS: %s: %s\n", threadlocalhrq.req_body[i].key, content_en);
			continue;
		}
		
		if (strcmp(threadlocalhrq.req_body[i].key,"title_hun") == 0){
			title_hun= unbase64(threadlocalhrq.req_body[i].value, b64_len, &b64_decoded_len);
			printf("KEYVALUEPAIRS: %s: %s\n", threadlocalhrq.req_body[i].key, title_hun);
			continue;
		}

		if (strcmp(threadlocalhrq.req_body[i].key,"title_en") == 0){
			title_en = unbase64(threadlocalhrq.req_body[i].value, b64_len, &b64_decoded_len);
			init_callback_sql();
			delete_post(title_en);
			free_callback_sql();
			continue;
		}
		
		if (strcmp(threadlocalhrq.req_body[i].key,"category") == 0){
			category=sdsnew(threadlocalhrq.req_body[i].value);
			continue;
		}
		
	}
	init_callback_sql();
	insert_post(title_hun, title_en, category, content_hun, content_en);
	free_callback_sql();
	free(content_en);
	free(content_hun);
	free(title_en);
	free(title_hun);
	sdsfree(category);

	//reset the cache
	//yup if someone getting data from cache in an other thread while that is uninitialized
	//this will be race condition and undefined behaviour
	pthread_mutex_lock(&cache_locker_mutex);
	globalfree_cache();
	globalinit_cache();
	pthread_mutex_unlock(&cache_locker_mutex);


return sdsnew("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\nSaved! Yaay, Backend C u! :)");
	}

/*route for the index page*/
sds rootroute(void){

	sds response = setresponsecode(okcode); //means HTTP/1.1 200 OK
	addheader(&response, "Connection", "close");
	addheader(&response, "Content-Type", "text/html");
	addheader(&response, "Strict-Transport-Security", "max-age=31536000; includeSubDomains");
	addheader(&response, "X-Frame-Options", "deny");
	addheader(&response, "X-XSS-Protection", "1; mode=block");
	addheader(&response, "X-Content-Type-Options", "nosniff");
	//addheader(&response, "Content-Encoding", "deflate");
    addheadersdone(&response);
    
	init_callback_sql();
	sds languagecookie=sdsnew("lang");
	sds cookievalue = get_cookie_by_name(languagecookie);
	Flate *f = NULL;
	sds lang=sdsnew("HUN");

	if (sdscmp(cookievalue,lang)==0) select_top5_by_category_hu("CTF");
	else select_top5_by_category_en("CTF");

	if (sdscmp(cookievalue,lang)==0) flateSetFile(&f, ROOTPATH"templates/index.html");
	else flateSetFile(&f, ROOTPATH"templates/index_EN.html");
	
	for (int i=0; i<MAXPOSTSSHOWN; i+=4){
		if (kvp_array_sqldata[i].key!=NULL && kvp_array_sqldata[i].value!=NULL){
			sds sdsurl= sdsnew("/onepost?tittle=");
			sdsurl =sdscatsds(sdsurl, kvp_array_sqldata[i].value);

			flateSetVar(f, "LINK",sdsurl);
			flateSetVar(f, "TITTLE", kvp_array_sqldata[i].value);
			flateSetVar(f, "createdat", kvp_array_sqldata[i+3].value);
			flateDumpTableLine(f, "listctf");
			sdsfree(sdsurl);
		}
	}
	free_callback_sql();
	init_callback_sql();
	if (sdscmp(cookievalue,lang)==0) select_top5_by_category_hu("Security");
	else select_top5_by_category_en("Security");

	for (int i=0; i<MAXPOSTSSHOWN; i+=4){
		if (kvp_array_sqldata[i].key!=NULL && kvp_array_sqldata[i].value!=NULL){
			sds sdsurl= sdsnew("/onepost?tittle=");
			sdsurl =sdscatsds(sdsurl, kvp_array_sqldata[i].value);

			flateSetVar(f, "LINK2",sdsurl);
			flateSetVar(f, "TITTLE2", kvp_array_sqldata[i].value);
			flateSetVar(f, "createdat", kvp_array_sqldata[i+3].value);
			flateDumpTableLine(f, "listsecurity");
			sdsfree(sdsurl);
		}
	}

	free_callback_sql();
	init_callback_sql();
	if (sdscmp(cookievalue,lang)==0) select_top5_by_category_hu("Linux");
	else select_top5_by_category_en("Linux");

	for (int i=0; i<MAXPOSTSSHOWN; i+=4){
		if (kvp_array_sqldata[i].key!=NULL && kvp_array_sqldata[i].value!=NULL){
			sds sdsurl= sdsnew("/onepost?tittle=");
			sdsurl =sdscatsds(sdsurl, kvp_array_sqldata[i].value);

			flateSetVar(f, "LINK3",sdsurl);
			flateSetVar(f, "TITTLE3", kvp_array_sqldata[i].value);
			flateSetVar(f, "createdat", kvp_array_sqldata[i+3].value);
			flateDumpTableLine(f, "listnix");
			sdsfree(sdsurl);
		}
	}
    
	free_callback_sql();

	sdsfree(lang);
	sdsfree(languagecookie);
	sdsfree(cookievalue);

	char *buf = flatePage(f);
	sds dynpage =sdsnew(buf);
	free(buf);
	flateFreeMem(f);
	response = sdscatsds(response, dynpage);
	sdsfree(dynpage);
	
	//add_to_cache(response);
	return  response;
	}

sds languageroute(void){
	char* urldecoded=NULL;
	sds lang=sdsnew("lang");
	for(int i=0; i<threadlocalhrq.bodycount; i++){
		if (sdscmp(threadlocalhrq.req_body[i].key,lang) == 0){
			urldecoded = malloc(sdslen(threadlocalhrq.req_body[i].value)+1);
			percent_decode(urldecoded,threadlocalhrq.req_body[i].value);
			break;
		}
	}
	sdsfree(lang);
		sds response = NULL;
		sds hun=sdsnew("HUN");
		sds sdsurldecoded=sdsnew(urldecoded);
		if (sdscmp(sdsurldecoded,hun)==0)
			response = sdsnew("HTTP/1.1 301 Moved Permanently\r\nLocation: /\r\nSet-Cookie: lang=HUN\r\nCache-Control: no-cache, no-store, must-revalidate\r\nPragma: no-cache\r\nExpires: 0\r\n\r\n");
		else
			response = sdsnew("HTTP/1.1 301 Moved Permanently\r\nLocation: /\r\nSet-Cookie: lang=EN\r\nCache-Control: no-cache, no-store, must-revalidate\r\nPragma: no-cache\r\nExpires: 0\r\n\r\n");
		free(urldecoded);
		sdsfree(hun);
		sdsfree(sdsurldecoded);
	return response;
	}

sds updateroute(void){
	//TODO create authenticate function
	int authentication_exist = 0;
	//check authetntication
	for(int i=0; i<threadlocalhrq.headercount; i++){
		if(strcmp(threadlocalhrq.req_headers[i].key,"Authentication")==0) {
			authentication_exist = 1;
			//GET the hash from http
			unsigned char md[SHA512_DIGEST_LENGTH]; // 32 bytes
			void * pw_from_header = (void*) threadlocalhrq.req_headers[i].value;
			simpleSHA512(pw_from_header, sdslen(pw_from_header), md);

			char stored_pw_from_header[(SHA512_DIGEST_LENGTH*2)+1]; //array vs ptr type bypass, fixme later...
			stored_pw_from_header[SHA512_DIGEST_LENGTH*2]='\0'; 
			for (int j = 0; j < SHA512_DIGEST_LENGTH; j++) {
				sprintf(&stored_pw_from_header[j*2], "%02x", md[j]);
				}

			//get the hash from FILE
			FILE *fp;
			//read from file, so if the file length <255 no overflow
			char pwbuff[255];
			memset(pwbuff,0,254);
			fp = fopen("./dev/db/password.txt", "r");
			fscanf(fp, "%s", pwbuff);
			fclose(fp);
			if (strcmp(pwbuff, stored_pw_from_header) != 0) return sdsnew("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\nBAD PASSWORD");
		}
	}
	if (authentication_exist == 0) return sdsnew("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\nPLEASE AUTHENTICATE");

	for(int i=0; i<threadlocalhrq.bodycount; i++){
		if (strcmp(threadlocalhrq.req_body[i].key,"title") == 0){
			init_callback_sql();
			fwrite(threadlocalhrq.req_body[i].value, sdslen(threadlocalhrq.req_body[i].value), 1, stdout);
			fflush(stdout);
			puts("\n");
			select_top1_by_name(threadlocalhrq.req_body[i].value);
			break;
		}
	}
	sds response = sdsnew("HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n{");
	int flen =0;
	for (int i=0; i<5; i++){
		if (kvp_array_sqldata[i].key==NULL || kvp_array_sqldata[i].value == NULL){
			free_callback_sql();
			sdsfree(response);
			return sdsnew("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\nProbably not found, but maybe other error!");
		}
		response = sdscat(response,"\"");
		response = sdscatsds(response,kvp_array_sqldata[i].key);
		response = sdscat(response,"\":\"");
		char* b64_value = base64(kvp_array_sqldata[i].value, sdslen(kvp_array_sqldata[i].value), &flen);
		response = sdscatlen(response,b64_value, flen);
		response = sdscat(response,"\",");
		if (i==4) response[sdslen(response)-1] = '}';
		free(b64_value);
	}
	
	free_callback_sql();
	return response;
}

sds deleteroute(void){
	//TODO create authenticate function
	int authentication_exist = 0;
	//check authetntication
	for(int i=0; i<threadlocalhrq.headercount; i++){
		if(strcmp(threadlocalhrq.req_headers[i].key,"Authentication")==0) {
			authentication_exist = 1;
			//GET the hash from http
			unsigned char md[SHA512_DIGEST_LENGTH]; // 32 bytes
			void * pw_from_header = (void*) threadlocalhrq.req_headers[i].value;
			simpleSHA512(pw_from_header, sdslen(pw_from_header), md);

			char stored_pw_from_header[(SHA512_DIGEST_LENGTH*2)+1]; //array vs ptr type bypass, fixme later...
			stored_pw_from_header[SHA512_DIGEST_LENGTH*2]='\0'; 
			for (int j = 0; j < SHA512_DIGEST_LENGTH; j++) {
				sprintf(&stored_pw_from_header[j*2], "%02x", md[j]);
				}

			//get the hash from FILE
			FILE *fp;
			//read from file, so if the file length <255 no overflow
			char pwbuff[255];
			memset(pwbuff,0,254);
			fp = fopen("./dev/db/password.txt", "r");
			fscanf(fp, "%s", pwbuff);
			fclose(fp);
			if (strcmp(pwbuff, stored_pw_from_header) != 0) return sdsnew("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\nBAD PASSWORD");
		}
	}
	if (authentication_exist == 0) return sdsnew("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\nPLEASE AUTHENTICATE");

	for(int i=0; i<threadlocalhrq.bodycount; i++){
		if (strcmp(threadlocalhrq.req_body[i].key,"title") == 0){
			init_callback_sql();
			//TODO percent decode or POST?
			delete_post(threadlocalhrq.req_body[i].value);
			free_callback_sql();
			break;
		}
	}

	return sdsnew("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\nI tried to delete Check what happened...");
}
/*register the routes here*/
void controllercall(void){
	create_route("listincategory", &listincategoryroute);
	create_route("", &rootroute);
	create_route("admin/save", &saveroute);
	create_route("admin/delete", &deleteroute);
	create_route("admin/update", &updateroute);
	create_route("onepost", &onepostroute);
	create_route("language", &languageroute);
}
