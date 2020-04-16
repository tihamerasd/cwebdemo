#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <wolfssl/options.h>
#include <wolfssl/ssl.h>
#include <pthread.h>
#include <signal.h>

#include "./backend/keyvalue.h"
#include "./backend/requester.h"
#include "./backend/responser.h"
#include "./backend/controller.c"
#include "./backend/sql/sqlthings.h"
#include "backend/webapplication_firewall/simple_waf.h"

#define MAXBUF  4096*32
#define DEFAULT_PORT 8444
#define MAX_CONCURRENT_THREADS 2
#define CERT_FILE "./certificate.pem"
#define KEY_FILE  "./key.pem"

//TODO BLOCKING SOCKET DOS IF MORE OPEN CONNECTION THEN THREAD

void portable_requester(char* rawreq, int len){
	sds s = sdsempty();
	init_threadlocalhrq();
	s=sdscatlen(s,rawreq,len);
	create_request(s);
	sdsfree(s);
}

sds portable_responser(void){
	sds response;
	if(threadlocalhrq.url==NULL) {
		threadlocalhrq.url=sdsnew("/");    //go to webroot
		threadlocalhrq.rawurl=sdsnew("/"); //go to webroot
		puts("ERROR! Somehow url is null\n");
	}
	if (check_route()!=0) {
		response = do_route();
	}
	else{
		sds response_body = initdir_for_static_files();
		if(response_body == NULL){
				response=sdsnew("HTTP/1.1 301 Moved Permanently\r\n"
								"Location: /\r\n"
								"NOTFOUND_URL: TRUE\r\n"
								"Cache-Control: no-cache, no-store, must-revalidate\r\n"
								"Pragma: no-cache\r\n"
								"Expires: 0\r\n\r\n");
			}
		else{
			response = adddefaultheaders();
			response = sdscatsds(response,response_body);
			sdsfree(response_body);
		}
	}
	return response;
}

struct targ_pkg {
    int          open;
    pthread_t    tid;
    int          num;
    int          connd;
    WOLFSSL_CTX* ctx;
    int*         shutdown;
};



void* ClientHandler(void* args)
{
    struct targ_pkg* pkg = args;
    WOLFSSL*         ssl;
    char             buff[MAXBUF];
    int				 len =sizeof(buff);
    int              ret;
    sds response=NULL;


    printf("thread %d open for business\n", pkg->num);

    /* Create a WOLFSSL object */
    if ((ssl = wolfSSL_new(pkg->ctx)) == NULL) {
        fprintf(stderr, "ERROR: failed to create WOLFSSL object\n");
        pkg->open = 1;
        pthread_exit(NULL);
    }

    /* Attach wolfSSL to the socket */
    printf("Handling thread %d on port %d\n", pkg->num, pkg->connd);
    wolfSSL_set_fd(ssl, pkg->connd);

    /* Establish TLS connection */
    do {
        ret = wolfSSL_accept(ssl);
    } while(wolfSSL_want_read(ssl));

    if (ret != SSL_SUCCESS) {
        printf("ret = %d\n", ret);
        fprintf(stderr, "wolfSSL_accept error = %d\n",
            wolfSSL_get_error(ssl, ret));
        pkg->open = 1;
        pthread_exit(NULL);
    }

    printf("Client %d connected successfully\n", pkg->num);

    /* Read the client data into our buff array */
    XMEMSET(buff, 0, len);

        ret = wolfSSL_read(ssl, buff, len-1);


    if (ret > 0) {
        /* Print to stdout any data the client sends */
        printf("Client %d said: %s\n", pkg->num, buff);
    } else {
        printf("wolfSSL_read encountered an error with code %d and msg %s\n",
               ret, wolfSSL_ERR_error_string(ret, buff));
        wolfSSL_free(ssl);      /* Free the wolfSSL object              */
        close(pkg->connd);      /* Close the connection to the server   */
        pkg->open = 1;          /* Indicate that execution is over      */
        pthread_exit(NULL);     /* End theread execution                */
    }

    /* Check for server shutdown command */
    if (XSTRNCMP(buff, "shutdown", 8) == 0) {
        printf("Shutdown command issued!\n");
        *pkg->shutdown = 1;
    }

    /* Write our reply into buff */
    //XMEMSET(buff, 0, len);
    //len = XSTRLEN(reply);
    //XMEMCPY(buff, reply, len);



if (len<0) {
	wolfSSL_free(ssl);      /* Free the wolfSSL object              */
    close(pkg->connd);      /* Close the connection to the server   */
    pkg->open = 1;          /* Indicate that execution is over      */
    pthread_exit(NULL); 
	}
	
	int match = simple_waf(buff, len);
	if( match == 1){
		char* simpleblock="HTTP/1.1 301 Moved Permanently\r\n"
						  "Location: /\r\n"
		                  "Set-Cookie: Hacker=TRUE\r\n"
		                  "Cache-Control: no-cache, no-store, must-revalidate\r\n"
		                  "Pragma: no-cache\r\n"
		                  "Expires: 0\r\n\r\n";
		response=sdsnew(simpleblock);
				}
	else{
		portable_requester(buff, len);
		response=portable_responser();
		requestfree();
	}

	len = sdslen(response);
	 do {
        ret = wolfSSL_write(ssl, response, len);
       
    } while (wolfSSL_want_write(ssl));

	sdsfree(response);
  
        

    if (ret != len) {
        printf("wolfSSL_write encountered an error with code %d and msg %s\n",
               ret, wolfSSL_ERR_error_string(ret, buff));
    }

    /* Cleanup after this connection */
    wolfSSL_free(ssl);      /* Free the wolfSSL object              */
    close(pkg->connd);      /* Close the connection to the server   */
    pkg->open = 1;          /* Indicate that execution is over      */
    pthread_exit(NULL);     /* End theread execution                */
}

void  INThandler(int sig){
	puts("OK I'm exit...");
	for(int i=0; i<table.route_count; i++) sdsfree(table.routes[i].url);
	globalfree_cache();
	sqlite_close_function();
    exit (0);
}

int main()
{
    int                sockfd;
    int                connd;
    struct sockaddr_in servAddr;
    struct sockaddr_in clientAddr;
    socklen_t          size = sizeof(clientAddr);
    int                shutdown = 0;

    /* declare wolfSSL objects */
    WOLFSSL_CTX* ctx;
    controllercall();
	globalinit_cache();
	sqlite_init_function();
	init_callback_sql();
	signal(SIGINT, INThandler);
	signal(SIGPIPE, SIG_IGN); // ignore broken pipe signal

    /* declare thread variable */
    struct targ_pkg thread[MAX_CONCURRENT_THREADS];
    int             i;



    /* Initialize wolfSSL */
    wolfSSL_Init();
    wolfSSL_Debugging_ON();


    /* Create a socket that uses an internet IPv4 address,
     * Sets the socket to be stream based (TCP),
     * 0 means choose the default protocol. */
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        fprintf(stderr, "ERROR: failed to create the socket\n");
        return -1;
    }

    /* Set the socket options to use nonblocking I/O */
    if (fcntl(sockfd, F_SETFL, O_NONBLOCK) == -1) {
        fprintf(stderr, "ERROR: failed to set socket options\n");
        return -1;
    }



    /* Create and initialize WOLFSSL_CTX */
    if ((ctx = wolfSSL_CTX_new(wolfTLSv1_2_server_method())) == NULL) {
        fprintf(stderr, "ERROR: failed to create WOLFSSL_CTX\n");
        return -1;
    }

    /* Load server certificates into WOLFSSL_CTX */
    if (wolfSSL_CTX_use_certificate_file(ctx, CERT_FILE, SSL_FILETYPE_PEM)
        != SSL_SUCCESS) {
        fprintf(stderr, "ERROR: failed to load %s, please check the file.\n",
                CERT_FILE);
        return -1;
    }

    /* Load server key into WOLFSSL_CTX */
    if (wolfSSL_CTX_use_PrivateKey_file(ctx, KEY_FILE, SSL_FILETYPE_PEM)
        != SSL_SUCCESS) {
        fprintf(stderr, "ERROR: failed to load %s, please check the file.\n",
                KEY_FILE);
        return -1;
    }



    /* Initialize the server address struct with zeros */
    memset(&servAddr, 0, sizeof(servAddr));

    /* Fill in the server address */
    servAddr.sin_family      = AF_INET;             /* using IPv4      */
    servAddr.sin_port        = htons(DEFAULT_PORT); /* on DEFAULT_PORT */
    servAddr.sin_addr.s_addr = INADDR_ANY;          /* from anywhere   */



    /* Bind the server socket to our port */
    if (bind(sockfd, (struct sockaddr*)&servAddr, sizeof(servAddr)) == -1) {
        fprintf(stderr, "ERROR: failed to bind\n");
        return -1;
    }

    /* Listen for a new connection, allow 5 pending connections */
    if (listen(sockfd, 5) == -1) {
        fprintf(stderr, "ERROR: failed to listen\n");
        return -1;
    }



    /* initialise thread array */
    for (i = 0; i < MAX_CONCURRENT_THREADS; ++i) {
        printf("Creating %d thread\n", i);
        thread[i].open = 1;
        thread[i].num = i;
        thread[i].ctx = ctx;
        thread[i].shutdown = &shutdown;
    }



    printf("Now open for connections\n");

    /* Continue to accept clients until shutdown is issued */
    while (!shutdown) {
        /* find an open thread or continue if there is none */
        for (i = 0; i < MAX_CONCURRENT_THREADS && !thread[i].open; ++i);
        if (i == MAX_CONCURRENT_THREADS) {
            continue;
        }

        /* Accept client connections */
        if ((connd = accept(sockfd, (struct sockaddr*)&clientAddr, &size))
            == -1) {
            continue;
        }



        /* Fill out the relevent thread argument package information */
        thread[i].open = 0;
        thread[i].connd = connd;

        /* Launch a thread to deal with the new client */
        pthread_create(&thread[i].tid, NULL, ClientHandler, &thread[i]);

        /* State that we won't be joining this thread */
        pthread_detach(thread[i].tid);
    }



    /* Suspend shutdown until all threads are closed */
    do {
        shutdown = 1;

        for (i = 0; i < MAX_CONCURRENT_THREADS; ++i) {
            if (!thread[i].open) {
                shutdown = 0;
            }
        }
    } while (!shutdown);

    printf("Shutdown complete\n");



    /* Cleanup and return */
    wolfSSL_CTX_free(ctx);  /* Free the wolfSSL context object          */
    wolfSSL_Cleanup();      /* Cleanup the wolfSSL environment          */
    close(sockfd);          /* Close the socket listening for clients   */
    return 0;               /* Return reporting a success               */
}
