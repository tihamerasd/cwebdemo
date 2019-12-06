#include <sys/epoll.h>

#include <wolfssl/options.h>
#include <wolfssl/ssl.h>

#include <wolfssl/test.h>
#include "./backend/keyvalue.h"
#include "./backend/requester.h"
#include "./backend/responser.h"
#include "./backend/controller.c"
#include "./backend/sql/sqlthings.h"

#include <signal.h>

#include "backend/webapplication_firewall/yarawaf.h"
//looks interesting, what is it?
//#define WOLFSSL_ASYNC_CRYPT

/* Default port to listen on. */
#define DEFAULT_PORT     8080
/* The number of EPOLL events to accept and process at one time. */
#define EPOLL_NUM_EVENTS 1000
/* The number of threads to create. */
#define NUM_THREADS      15
/* The number of concurrent connections to support. */
#define SSL_NUM_CONN     1000
/* The number of bytes to read from client. */
#define NUM_READ_BYTES   16000
/* The number of bytes to write to client. */
#define NUM_WRITE_BYTES  65536
/* The maximum number of bytes to send in a run. */
#define MAX_BYTES        -1
/* The maximum length of the queue of pending connections. */
#define NUM_CLIENTS      1000
/* The number of wolfSSL events to accept and process at one time. */
#define MAX_WOLF_EVENTS  1001

/* The default server certificate. */
#define SVR_CERT "./wolfssl/wolfssl-examples/certs/server-cert.pem"
/* The default server private key. */
#define SVR_KEY  "./wolfssl/wolfssl-examples/certs/server-key.pem"

/* The states of the SSL connection. */
typedef enum SSLState { ACCEPT, READ, WRITE, CLOSED } SSLState;

/* Type for the SSL connection data. */
typedef struct SSLConn SSLConn;

/* Data for each active connection. */
struct SSLConn {
    /* The socket listening on, reading from and writing to. */
    int sockfd;
    /* The wolfSSL object to perform TLS communications. */
    WOLFSSL* ssl;
    /* The current state of the SSL/TLS connection. */
    SSLState state;
    /* Previous SSL connection data object. */
    SSLConn* prev;
    /* Next SSL connection data object. */
    SSLConn* next;
};

/* SSL connection data for a thread. */
typedef struct ThreadData {
    /* The SSL/TLS context for all connections in thread. */
    WOLFSSL_CTX* ctx;
    /* The device id for this thread. */
    int devId;
    /* Linked list of SSL connection data. */
    SSLConn *sslConn;
    /* Free list. */
    SSLConn *freeSSLConn;
    /* The number of active SSL connections.  */
    int cnt;
    /* Accepting new connections. */
    int accepting;

    /* The thread id for the handler. */
    pthread_t thread_id;
} ThreadData;

/* The information about SSL/TLS connections. */
typedef struct SSLConn_CTX {
    /* An array of active connections. */
    ThreadData* threadData;
    /* Number of threads. */
    int numThreads;
    /* Maximum number of active connections per thread. */
    int numConns;

    /* Size of the client data buffer. */
    int bufferLen;

    /* Number of connections handled. */
    int numConnections;
    /* Number of resumed connections handled. */
    int numResumed;
    /* Maximum number of connections to perform. */
    int maxConnections;
    /* Maximum number of bytes to read/write. */
    int maxBytes;

    /* Total time handling accepts. */
    double acceptTime;
    /* Total time handling accepts - resumed connections. */
    double resumeTime;
} SSLConn_CTX;

static void SSLConn_Free(SSLConn_CTX* ctx);
static void SSLConn_Close(SSLConn_CTX* ctx, ThreadData* threadData,
    SSLConn* sslConn);
static void SSLConn_FreeSSLConn(ThreadData* threadData);
static void WolfSSLCtx_Final(ThreadData* threadData);

/* Global SSL/TLS connection data context. */
static SSLConn_CTX* sslConnCtx   = NULL;
/* Mutex for using connection count.  */
static pthread_mutex_t sslConnMutex = PTHREAD_MUTEX_INITIALIZER;
/* The port to listen on. */
static word16       port          = DEFAULT_PORT;
/* The size of the listen backlog. */
static int          numClients    = NUM_CLIENTS;
/* User specified ciphersuite list. */
static char*        cipherList    = NULL;
/* The server's certificate for authentication. */
static char*        ourCert       = SVR_CERT;
/* The server's private key for authentication. */
static char*        ourKey        = SVR_KEY;
/* The version of SSL/TLS to use. */
static int          version       = SERVER_DEFAULT_VERSION;
/* The number of threads to start. */
static int          numThreads    = NUM_THREADS;
/* The number of connections per threads to allow. */
static int          numConns      = SSL_NUM_CONN;
/* The number of bytes to read from the client. */
static int          numBytesRead  = NUM_READ_BYTES;
/* The number of bytes to write to the client. */
static int          numBytesWrite = NUM_WRITE_BYTES;

int out=0; //global variable gives information about if sigint happened on server side (exit method).

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
	threadlocalhrq.url=sdsnew("index.html");
	puts("ERROR! Somehow url is null\n");
	}
if (check_route()!=0) {
	response = do_route();
	}
else{
	sds response_body = initdir_for_static_files();
	response = adddefaultheaders();
	response = sdscatsds(response,response_body);
    sdsfree(response_body);
	}

return response;
}

static wolfSSL_method_func SSL_GetMethod(int version)
{
    wolfSSL_method_func method = NULL;
    method =wolfTLSv1_2_server_method;
    return method;
}


/* Write data to a client.
 *
 * ssl         The wolfSSL object.
 * reply       The data to send to the client.
 * replyLen    The length of the data to send to the client.
 * totalBytes  The total number of bytes sent to clients.
 * writeTime   The amount of time spent writing data to client.
 * returns 0 on failure, 1 on success, 2 on want read and 3 on want write.
 */
static int SSL_Write(WOLFSSL* ssl, char* reply, int replyLen)
{
    int  rwret = 0;
    int  error;

    rwret = wolfSSL_write(ssl, reply, replyLen);

    if (rwret == 0) {
        fprintf(stderr, "The client has closed the connection - write!\n");
        return 0;
    }

    if (rwret == replyLen)
        return 1;

    error = wolfSSL_get_error(ssl, 0);
    if (error == SSL_ERROR_WANT_READ)
        return 2;
    if (error == SSL_ERROR_WANT_WRITE)
        return 3;
    if (error == WC_PENDING_E)
        return 4;
    if (error == 0)
        return 1;

    /* Cannot do anything about other errors. */
    fprintf(stderr, "wolfSSL_write error = %d\n", error);
    return 0;
}

/* Reads data from a client.
 *
 * ssl         The wolfSSL object.
 * buffer      The buffer to place client data into.
 * len         The length of the buffer.
 * totalBytes  The total number of bytes read from clients.
 * readTime    The amount of time spent reading data from client.
 * returns 0 on failure, 1 on success, 2 on want read and 3 on want write.
 */
static int SSL_Read(WOLFSSL* ssl, char* buffer, int len)
{
    int  rwret = 0;
    int  error;

    rwret = wolfSSL_read(ssl, buffer, len);

    if (rwret == 0) {
		    //ignore if the socket closed after read but before write is done
	signal(SIGPIPE, SIG_IGN);
        return 0;
    }

    error = wolfSSL_get_error(ssl, 0);
    if (error == SSL_ERROR_WANT_READ)
        return 2;
    if (error == SSL_ERROR_WANT_WRITE)
        return 3;
    if (error == WC_PENDING_E)
        return 4;
    if (error == 0)
        return 1;

    /* Cannot do anything about other errors. */
    fprintf(stderr, "wolfSSL_read error = %d\n", error);
    return 0;
}

/* Accept/negotiate a secure connection.
 *
 * ssl         The wolfSSL object.
 * acceptTime  The amount of time spent accepting a client.
 * resumeTime  The amount of time spent resuming a connection with a client.
 * returns 0 on failure, 1 on success, 2 on want read and 3 on want write.
 */
static int SSL_Accept(WOLFSSL* ssl)
{
    int ret;
    int error;
    
    ret = wolfSSL_accept(ssl);

    if (ret == 0) {
        fprintf(stderr, "The client has closed the connection - accept!\n");
        return 0;
    }

    if (ret == SSL_SUCCESS)
        return 1;

    error = wolfSSL_get_error(ssl, 0);
    if (error == SSL_ERROR_WANT_READ)
        return 2;
    if (error == SSL_ERROR_WANT_WRITE)
        return 3;
    if (error == WC_PENDING_E)
        return 4;
        

    /* Cannot do anything about other errors. */
    fprintf(stderr, "wolfSSL_accept error = %d\n", error);
    return 0;
}

static SSLConn_CTX* SSLConn_New(int numThreads, int numConns, int bufferLen,
                                int replyLen)
{
    SSLConn_CTX* ctx;
    ThreadData*  threadData;
    int          i;

    ctx = (SSLConn_CTX*)malloc(sizeof(*ctx));
    if (ctx == NULL)
        return NULL;
    memset(ctx, 0, sizeof(*ctx));

    ctx->numThreads = numThreads;
    ctx->numConns = numConns;
    ctx->bufferLen = bufferLen;
    /* Pre-allocate the SSL connection data. */
    ctx->threadData = (ThreadData*)malloc(ctx->numThreads *
                                          sizeof(*ctx->threadData));
    if (ctx->threadData == NULL) {
        SSLConn_Free(ctx);
        return NULL;
    }
    for (i = 0; i < ctx->numThreads; i++) {
        threadData = &ctx->threadData[i];

        threadData->ctx = NULL;
        threadData->devId = INVALID_DEVID;
        threadData->sslConn = NULL;
        threadData->freeSSLConn = NULL;
        threadData->cnt = 0;
        threadData->thread_id = 0;
    }

    return ctx;
}

/* Free the SSL/TLS connection data.
 *
 * ctx  The connection data.
 */
static void SSLConn_Free(SSLConn_CTX* ctx)
{
    int i;
    ThreadData* threadData;

    if (ctx == NULL)
        return;

    for (i = 0; i < ctx->numThreads; i++) {
        threadData = &ctx->threadData[i];

        while (threadData->sslConn != NULL)
            SSLConn_Close(ctx, threadData, threadData->sslConn);
        SSLConn_FreeSSLConn(threadData);
        WolfSSLCtx_Final(threadData);
    }
    free(ctx->threadData);
    ctx->threadData = NULL;

    free(ctx);
}

static void SSLConn_Close(SSLConn_CTX* ctx, ThreadData* threadData,
                          SSLConn* sslConn)
{
    //int ret;

    if (sslConn->state == CLOSED)
        return;

	//TODO sg wrong here
   /* pthread_mutex_lock(&sslConnMutex);
    ret = (ctx->numConnections == 0);
    ctx->numConnections++;
    if (wolfSSL_session_reused(sslConn->ssl))
        ctx->numResumed++;
    pthread_mutex_unlock(&sslConnMutex);

    if (ret) {
        WOLFSSL_CIPHER* cipher;
        cipher = wolfSSL_get_current_cipher(sslConn->ssl);
        printf("SSL cipher suite is %s\n", wolfSSL_CIPHER_get_name(cipher));
    }*/

    sslConn->state = CLOSED;

    /* Take it out of the double-linked list. */
    if (threadData->sslConn == sslConn)
        threadData->sslConn = sslConn->next;
    if (sslConn->next != NULL)
        sslConn->next->prev = sslConn->prev;
    if (sslConn->prev != NULL)
        sslConn->prev->next = sslConn->next;

    /* Put object at head of free list */
    sslConn->next = threadData->freeSSLConn;
    sslConn->prev = NULL;
    threadData->freeSSLConn = sslConn;

    threadData->cnt--;
}

static void SSLConn_FreeSSLConn(ThreadData* threadData)
{
    SSLConn* sslConn = threadData->freeSSLConn;

    threadData->freeSSLConn = NULL;

    while (sslConn != NULL) {
        SSLConn* next = sslConn->next;

        wolfSSL_free(sslConn->ssl);
        sslConn->ssl = NULL;
        close(sslConn->sockfd);
        free(sslConn);

        sslConn = next;
    }
}

static int SSLConn_Accept(ThreadData* threadData, WOLFSSL_CTX* sslCtx,
                          socklen_t sockfd, SSLConn** sslConn)
{
    struct sockaddr_in clientAddr = {0};
    socklen_t          size = sizeof(clientAddr);
    SSLConn*           conn;

    conn = malloc(sizeof(*conn));
    if (conn == NULL)
        return EXIT_FAILURE;

    /* Accept the client connection. */
    conn->sockfd = accept(sockfd, (struct sockaddr *)&clientAddr, &size);
    if (conn->sockfd == -1) {
        free(conn);
        fprintf(stderr, "ERROR: failed to accept\n");
        return EXIT_FAILURE;
    }
    /* Set the new socket to be non-blocking. */
    fcntl(conn->sockfd, F_SETFL, O_NONBLOCK);

    /* Setup SSL/TLS connection. */
    if ((conn->ssl = wolfSSL_new(sslCtx)) == NULL) {
        free(conn);
        fprintf(stderr, "wolfSSL_new error.\n");
        return EXIT_FAILURE;
    }
    /* Set the socket to communicate over into the wolfSSL object. */
    wolfSSL_set_fd(conn->ssl, conn->sockfd);
    wolfSSL_set_using_nonblock(conn->ssl, 1);

    conn->state = ACCEPT;
    conn->next = threadData->sslConn;
    conn->prev = NULL;
    if (threadData->sslConn != NULL)
        threadData->sslConn->prev = conn;
    threadData->sslConn = conn;
    threadData->cnt++;

    *sslConn = conn;

    return EXIT_SUCCESS;
}

static int SSLConn_ReadWrite(SSLConn_CTX* ctx, ThreadData* threadData,
                             SSLConn* sslConn)
{
    int ret=0;
    int len=0;
    char buffer[NUM_READ_BYTES];

    /* Perform TLS handshake if in accept state. */
    switch (sslConn->state) {
        case ACCEPT:
            ret = SSL_Accept(sslConn->ssl);
            if (ret == 0) {
                printf("ERROR: Accept failed\n");
                SSLConn_Close(ctx, threadData, sslConn);
                //return EXIT_FAILURE;
            }

            if (ret == 1)
            printf("ERROR: success");
                sslConn->state = READ;
            break;

        case READ:
            {
                len = ctx->bufferLen;
         
                if (len == 0)
                    break;

                /* Read application data. */
                memset(buffer, 0, NUM_READ_BYTES);
                ret = SSL_Read(sslConn->ssl, buffer, len);
                //yarafunction(buffer, len);
				match=0;
				if( match == 1){
					char* yarablock="HTTP/1.1 200 OK\r\n"
									"Server: asm_server\r\n"
									"Content-Type:text/html\r\n"
									"Connection: Closed\r\n\r\n"
									"YaraWaf is here, go away hacker!\0";
					wolfSSL_write(sslConn->ssl, yarablock, strlen(yarablock));
					SSLConn_Close(ctx, threadData, sslConn);
					 return EXIT_SUCCESS;
					}
                if (ret == 0) {
                    SSLConn_Close(ctx, threadData, sslConn);
                   return EXIT_FAILURE;
                }
            }

            if (ret != 1) return EXIT_FAILURE;
            sslConn->state = WRITE;
        case WRITE:
			; // DONT delete empty line ...c coding standards hacked... lol
		    portable_requester(buffer, len);
            sds response=portable_responser();
			requestfree();
			ret = SSL_Write(sslConn->ssl, response, sdslen(response));
            
            sdsfree(response);
		   SSLConn_Close(ctx, threadData, sslConn);
		   sslConn->state = CLOSED;
            break;

        case CLOSED:
            break;
    }
    return EXIT_SUCCESS;
}

static int WolfSSLCtx_Init(ThreadData* threadData, int version,
    char* cert, char* key, char* cipherList)
{
    wolfSSL_method_func method = NULL;

    method = SSL_GetMethod(version);
    if (method == NULL)
        return(EXIT_FAILURE);

    /* Create and initialize WOLFSSL_CTX structure */
    if ((threadData->ctx = wolfSSL_CTX_new(method(NULL))) == NULL) {
        fprintf(stderr, "wolfSSL_CTX_new error.\n");
        return(EXIT_FAILURE);
    }

    /* Load server certificate into WOLFSSL_CTX */
    if (wolfSSL_CTX_use_certificate_file(threadData->ctx, cert, SSL_FILETYPE_PEM)
            != SSL_SUCCESS) {
        fprintf(stderr, "Error loading %s, please check the file.\n", cert);
        WolfSSLCtx_Final(threadData);
        return(EXIT_FAILURE);
    }

    /* Load server key into WOLFSSL_CTX */
    if (wolfSSL_CTX_use_PrivateKey_file(threadData->ctx, key, SSL_FILETYPE_PEM)
            != SSL_SUCCESS) {
        fprintf(stderr, "Error loading %s, please check the file.\n", key);
        WolfSSLCtx_Final(threadData);
        return(EXIT_FAILURE);
    }

    if (cipherList != NULL) {
        if (wolfSSL_CTX_set_cipher_list(threadData->ctx, cipherList) != SSL_SUCCESS) {
            fprintf(stderr, "Server can't set cipher list.\n");
            WolfSSLCtx_Final(threadData);
            return(EXIT_FAILURE);
        }
    }
    return EXIT_SUCCESS;
}

/* Cleanup the wolfSSL context and wolfSSL library.
 *
 * ctx  The wolfSSL context object.
 */
static void WolfSSLCtx_Final(ThreadData* threadData)
{
    wolfSSL_CTX_free(threadData->ctx);
    threadData->ctx = NULL;
}

static int CreateSocketListen(int port, int numClients, socklen_t* socketfd) {
    int                 ret;
    socklen_t           sockfd;
    struct sockaddr_in  serverAddr = {0};
    int                 on = 1;
    socklen_t           len = sizeof(on);

    /* Set the server's address. */
    memset((char *)&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family      = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port        = htons(port);

   //nonblocking socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == (socklen_t)-1) {
        fprintf(stderr, "ERROR: failed to create the socket\n");
        return(EXIT_FAILURE);
    }
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &on, len) < 0)
    //if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, len) < 0)
        fprintf(stderr, "setsockopt SO_REUSEADDR failed\n");
    if (setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &on, len) < 0)
        fprintf(stderr, "setsockopt TCP_NODELAY failed\n");

    if (bind(sockfd, (struct sockaddr *)&serverAddr,
             sizeof(serverAddr)) < 0) {
        fprintf(stderr, "ERROR: failed to bind\n");
        return(EXIT_FAILURE);
    }

    printf("Waiting for a connection...\n");

    ret = listen(sockfd, numClients);
    if (ret == -1) {
        fprintf(stderr, "ERROR: failed to listen\n");
        return(EXIT_FAILURE);
    }

    *socketfd = sockfd;
    return EXIT_SUCCESS;
}

static void *ThreadHandler(void *data)
{
    int                 ret;
    socklen_t           socketfd = -1;
    int                 efd;
    struct epoll_event  event;
    struct epoll_event  event_conn;
    struct epoll_event* events = NULL;
    ThreadData*         threadData = (ThreadData*)data;

    /* Initialize wolfSSL and create a context object. */
    if (WolfSSLCtx_Init(threadData, version, ourCert, ourKey, cipherList) == -1) {
        //exit(EXIT_FAILURE);
        puts("unknown error");
    }

    /* Allocate space for EPOLL events to be stored. */
    events = (struct epoll_event*)malloc(EPOLL_NUM_EVENTS * sizeof(*events));
    if (events == NULL)
        //exit(EXIT_FAILURE);
        puts("unknown error");

    /* Create a socket and listen for a client. */
    if (CreateSocketListen(port, numClients, &socketfd) == EXIT_FAILURE)
        //exit(EXIT_FAILURE);
        puts("unknown error");

    /* Create an EPOLL file descriptor. */
    efd = epoll_create1(0);
    if (efd == -1) {
        fprintf(stderr, "ERROR: failed to create epoll\n");
        //exit(EXIT_FAILURE);
        puts("unknown error");
    }

    /* Add the event for communications on listening socket. */
    memset(&event, 0, sizeof(event));
    event.events = EPOLLIN;
    event.data.ptr = NULL;
    ret = epoll_ctl(efd, EPOLL_CTL_ADD, socketfd, &event);
    if (ret == -1) {
        fprintf(stderr, "ERROR: failed to add event to epoll\n");
        //exit(EXIT_FAILURE);
        puts("unknown error");
    }
    threadData->accepting = 1;

    while (out!=1) {
        int n;
        int i;

        SSLConn_FreeSSLConn(threadData);
        n = epoll_wait(efd, events, EPOLL_NUM_EVENTS, 1);

        /* Process all returned events. */
        for (i = 0; i < n; i++) {
            /* Error event on socket. */
            if (!(events[i].events & EPOLLIN)) {
                if (events[i].data.ptr == NULL) {
                    /* Not a client, therefore the listening connection. */
                    close(socketfd);
                    socketfd = -1;
                }
                else {
                    /* Client connection. */
                    SSLConn_Close(sslConnCtx, threadData, events[i].data.ptr);
                    ret = epoll_ctl(efd, EPOLL_CTL_ADD, socketfd, &event);
                }
            }
            else if (events[i].data.ptr == NULL) {
                SSLConn* sslConn;

                ret = SSLConn_Accept(threadData, threadData->ctx, socketfd,
                                     &sslConn);
                if (ret  == EXIT_SUCCESS) {
                    /* Set EPOLL to check for events on the new socket. */
                    memset(&event_conn, 0, sizeof(event_conn));
                    event_conn.events = EPOLLIN | EPOLLET;
                    event_conn.data.ptr = sslConn;
                    ret = epoll_ctl(efd, EPOLL_CTL_ADD, sslConn->sockfd,
                                    &event_conn);
                    if (ret == -1) {
                        fprintf(stderr, "ERROR: failed add event to epoll\n");
                        //exit(EXIT_FAILURE);
                        puts("unknown error");
                    }
                }

                if (threadData->cnt == sslConnCtx->numConns) {
                    /* Don't accept any more TCP connections. */
                    ret = epoll_ctl(efd, EPOLL_CTL_DEL, socketfd, &event);
                    if (ret == -1) {
                        fprintf(stderr, "ERROR: failed delete epoll event\n");
                        //exit(EXIT_FAILURE);
                        puts("unknown error");
                    }
                    threadData->accepting = 0;
                }
            }
            else {
                
                ret = SSLConn_ReadWrite(sslConnCtx, threadData,
                                        events[i].data.ptr);
            }
        }

        /* Accept more connections again up to the maximum concurrent. */
        if (!threadData->accepting &&
            threadData->cnt < sslConnCtx->numConns) {
            ret = epoll_ctl(efd, EPOLL_CTL_ADD, socketfd, &event);
            if (ret == -1) {
                fprintf(stderr, "ERROR: failed add event to epoll\n");
                //exit(EXIT_FAILURE);
            }
            threadData->accepting = 1;
        }
    }

    if (socketfd != -1)
        close(socketfd);
    free(events);

    return NULL;
}

void  INThandler(int sig)
{	 out=1;	 
}

int main(int argc, char* argv[])
{
    int i;

	controllercall();
	globalinit_cache();
	sqlite_init_function();
	init_callback_sql();
    wolfSSL_Init();
	signal(SIGINT, INThandler);
	signal(SIGPIPE, SIG_IGN); // ignore broken pipe signal
	
    /* Create SSL/TLS connection data object. */
    sslConnCtx = SSLConn_New(numThreads, numConns, numBytesRead, numBytesWrite);
    if (sslConnCtx == NULL)
        //exit(EXIT_FAILURE);
		puts("unknown error");
		
    for (i = 0; i < numThreads; i++) {
        if (pthread_create(&sslConnCtx->threadData[i].thread_id, NULL,
                           ThreadHandler, &sslConnCtx->threadData[i]) < 0) {
            perror("ERRROR: could not create thread");
        }
    }

    for (i = 0; i < numThreads; i++)
        pthread_join(sslConnCtx->threadData[i].thread_id, NULL) ;

    SSLConn_Free(sslConnCtx);
    wolfSSL_Cleanup();
    //TODO proper exit method... it's never called but we should free
    //some global object when exit
    for(int i=0; i<table.route_count; i++) sdsfree(table.routes[i].url);
	globalfree_cache();
	sqlite_close_function();
    exit(EXIT_SUCCESS);
    //return 0;
}

