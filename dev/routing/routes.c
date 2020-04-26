#include "routes.h"

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
	//check authetntication
	for(int i=0; i<threadlocalhrq.headercount; i++){
		if(strcmp(threadlocalhrq.req_headers[i].key,"Authentication")==0) {
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
			fp = fopen("backend/password.txt", "r");
			fscanf(fp, "%s", pwbuff);
			fclose(fp);
			if (strcmp(pwbuff, stored_pw_from_header) != 0) return sdsnew("BAD PASSWORD");
		}
	}
///admin/save?title_hun=huntitle&title_en=entitle&category=Security&content_hun=huncontetn%3Cbr%3E&content_eng=huncontetn%3Cbr%3E
	char* content_hun=NULL;
	char* content_en=NULL;
	sds title_hun = NULL;
	sds title_en = NULL;
	sds category = NULL;
	for(int i=0; i<threadlocalhrq.bodycount; i++){
		if (strcmp(threadlocalhrq.req_body[i].key,"content_hun") == 0){
			content_hun = malloc(sdslen(threadlocalhrq.req_body[i].value)+1);
			percent_decode(content_hun,threadlocalhrq.req_body[i].value);
			continue;
		}

		if (strcmp(threadlocalhrq.req_body[i].key,"content_eng") == 0){
			content_en = malloc(sdslen(threadlocalhrq.req_body[i].value)+1);
			percent_decode(content_en,threadlocalhrq.req_body[i].value);
			continue;
		}
		
		if (strcmp(threadlocalhrq.req_body[i].key,"title_hun") == 0){
			char* decodedtitle= malloc(sdslen(threadlocalhrq.req_body[i].value)+1);
			memset(decodedtitle,0, sdslen(threadlocalhrq.req_body[i].value));
			percent_decode(decodedtitle,threadlocalhrq.req_body[i].value);
			title_hun=sdsnew(decodedtitle);
			free(decodedtitle);
			continue;
		}

		if (strcmp(threadlocalhrq.req_body[i].key,"title_en") == 0){
			char* decodedtitle2= malloc(sdslen(threadlocalhrq.req_body[i].value)+1);
			memset(decodedtitle2,0, sdslen(threadlocalhrq.req_body[i].value));
			percent_decode(decodedtitle2,threadlocalhrq.req_body[i].value);
			title_en=sdsnew(decodedtitle2);
			free(decodedtitle2);
			continue;
		}
		
		if (strcmp(threadlocalhrq.req_body[i].key,"category") == 0){
			category=sdsnew(threadlocalhrq.req_body[i].value);
			continue;
		}
		
	}
	insert_post(title_hun, title_en, category, content_hun, content_en);
	free(content_en);
	free(content_hun);
	sdsfree(title_en);
	sdsfree(title_hun);
	sdsfree(category);

	//reset the cache
	//yup if someone getting data from cache in an other thread while that is uninitialized
	//this will be race condition and undefined behaviour
	pthread_mutex_lock(&cache_locker_mutex);
	globalfree_cache();
	globalinit_cache();
	pthread_mutex_unlock(&cache_locker_mutex);


return sdsnew("Saved! Yaay, Backend C u! :)");
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

sds ifconfigroute(void){
	sds response = setresponsecode(okcode); //means HTTP/1.1 200 OK
	addheader(&response, "Connection", "close");
	addheader(&response, "Content-Type", "text/html");
	addheadersdone(&response);
	//FILE *fp = popen("ifconfig lo "
	//FILE *fp = popen("ifconfig wlan0 "
	FILE *fp = popen("ifconfig eth0 "
					 "| grep 'inet' "
					 "| cut -d: -f2 "
					 "| awk '{print $2}' "
					 "|tr -s \"\n\"","r");
	char ipaddr[100];
	memset(ipaddr, 0, sizeof(ipaddr));
	fread(ipaddr, 1, sizeof(ipaddr),fp);
	response=sdscat(response,"<h1>My public ip is: ");
	response=sdscat(response,ipaddr);
	response=sdscat(response, "</h1>");
	fclose(fp);
	return response;
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

/*register the routes here*/
void controllercall(void){
	create_route("listincategory", &listincategoryroute);
	create_route("", &rootroute);
	create_route("admin/save", &saveroute);
	create_route("onepost", &onepostroute);
	create_route("ifconfig", &ifconfigroute);
	create_route("language", &languageroute);
}
