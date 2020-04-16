/* server-tls-epoll-perf.c
 *
 * Copyright (C) 2006-2016 wolfSSL Inc.
 *
 * This file is part of wolfSSL. (formerly known as CyaSSL)
 *
 * wolfSSL is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * wolfSSL is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 *=============================================================================
 *
 * This is an example of a TCP Server that uses non-blocking input and output to
 * handle a large number of connections. Reports performance figures.
*/

#include <sys/epoll.h>

#include <wolfssl/options.h>
#include <wolfssl/ssl.h>
#include <openssl/sha.h>
#include <wolfssl/test.h>
#include <signal.h>
#include "./backend/keyvalue.h"
#include "./backend/requester.h"
#include "./backend/responser.h"
#include "./backend/controller.c"
#include "backend/webapplication_firewall/simple_waf.h"
#include "base64.h"
#include <unistd.h>
#include "./backend/sql/sqlthings.h"

/* Default port to listen on. */
#define DEFAULT_PORT     11111
/* The number of EPOLL events to accept and process at one time. */
#define EPOLL_NUM_EVENTS 10000
/* The number of concurrent connections to support. */
#define SSL_NUM_CONN     10000
/* The number of bytes to read from client. */
#define NUM_READ_BYTES   8000
/* The number of bytes to write to client. */
#define NUM_WRITE_BYTES  16000
/* The maximum number of bytes to send in a run. */
#define MAX_BYTES        -1
/* The maximum length of the queue of pending connections. */
#define NUM_CLIENTS      10000
/* The number of wolfSSL events to accept and process at one time. */
#define MAX_WOLF_EVENTS  10000

/* The command line options. */
#define OPTIONS          "?p:v:al:c:k:A:n:N:R:W:B:"

/* The default server certificate. */
#define SVR_CERT "./wolfssl/wolfssl/certs/server-cert.pem"
/* The default server private key. */
#define SVR_KEY  "./wolfssl/wolfssl/certs/server-key.pem"
/* The default certificate/CA file for the client. */
#define CLI_CERT "./wolfssl/wolfssl/certs/client-cert.pem"

/* The states of the SSL connection. */
typedef enum SSLState { ACCEPT, READ, WRITE, CLOSED } SSLState;

/* Type for the SSL connection data. */
typedef struct SSLConn SSLConn;

/* Data for each active connection. */
struct SSLConn {
    /* The socket listening on, reading from and writing to. */
    int sockfd;
    /*for the socket it shows if it's upgraded*/
    int new_created;
    /* The wolfSSL object to perform TLS communications. */
    WOLFSSL* ssl;
    /* The current state of the SSL/TLS connection. */
    SSLState state;
    /* Previous SSL connection data object. */
    SSLConn* prev;
    /* Next SSL connection data object. */
    SSLConn* next;
};

/* The information about SSL/TLS connections. */
typedef struct SSLConn_CTX {
    /* An array of active connections. */
    SSLConn* sslConn;
    /* Free list. */
    SSLConn* freeSSLConn;
    /* Maximum number of active connections. */
    int numConns;
    /* Count of currently active connections. */
    int cnt;
    /* Accepting new connections. */
    int accepting;

    /* Size of the client data buffer. */
    int bufferLen;
    /* Number of bytes to write. */
    int replyLen;

    /* Number of connections handled. */
    int numConnections;
    /* Number of resumed connections handled. */
    int numResumed;

    /* Maximum number of bytes to read/write. */
    int maxBytes;

#ifdef WOLFSSL_ASYNC_CRYPT
    /* Total time handling aynchronous operations. */
    double asyncTime;
#endif
} SSLConn_CTX;

#define WS_FIN    128

/* Frame types. */
#define WS_FR_OP_TXT  1
#define WS_FR_OP_CLSE 8

#define WS_FR_OP_UNSUPPORTED 0xF
#define MESSAGE_LENGTH 2048

void portable_requester(char* rawreq, int len){
	sds s = sdsempty();
	s=sdscatlen(s,rawreq,len);
	//sds get =sdsnew("GET ");
	//printf("s_start: %c%c%c%c\n",s[0],s[1],s[2],s[3]);
	//if (memcmp(get, s, 4) !=0 ) {threadlocalhrq.req_type=sdsnew("GET"); threadlocalhrq.url=sdsnew("index.html"); return;}
	//sdsfree(get);;
	init_threadlocalhrq();
	create_request(s);
	sdsfree(s);
}

sds portable_responser(void){

sds response;
if(threadlocalhrq.url==NULL) {
	threadlocalhrq.url=sdsnew("index.html");
	threadlocalhrq.rawurl=sdsnew("index.html");
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


char* ws_sendframe(char *msg)
{
	unsigned char *response;  /* Response data.  */
	unsigned char frame[10];  /* Frame.          */
	uint8_t idx_first_rData;  /* Index data.     */
	uint64_t length;          /* Message length. */
	int idx_response;      /* Index response. */
	int output;               /* Bytes sent.     */

	/* Text data. */
	length   = strlen( (const char *) msg);
	frame[0] = (WS_FIN | WS_FR_OP_TXT);

	/* Split the size between octects. */
	if (length <= 125)
	{
		frame[1] = length & 0x7F;
		idx_first_rData = 2;
	}

	/* Size between 126 and 65535 bytes. */
	else if (length >= 126 && length <= 65535)
	{
		frame[1] = 126;
		frame[2] = (length >> 8) & 255;
		frame[3] = length & 255;
		idx_first_rData = 4;
	}

	/* More than 65535 bytes. */
	else
	{
		frame[1] = 127;
		frame[2] = (unsigned char) ((length >> 56) & 255);
		frame[3] = (unsigned char) ((length >> 48) & 255);
		frame[4] = (unsigned char) ((length >> 40) & 255);
		frame[5] = (unsigned char) ((length >> 32) & 255);
		frame[6] = (unsigned char) ((length >> 24) & 255);
		frame[7] = (unsigned char) ((length >> 16) & 255);
		frame[8] = (unsigned char) ((length >> 8) & 255);
		frame[9] = (unsigned char) (length & 255);
		idx_first_rData = 10;
	}

	/* Add frame bytes. */
	idx_response = 0;
	response = malloc( sizeof(unsigned char) * (idx_first_rData + length + 1) );
	for (int i = 0; i < idx_first_rData; i++)
	{
		response[i] = frame[i];
		idx_response++;
	}

	/* Add data bytes. */
	for (int i = 0; i < length; i++)
	{
		response[idx_response] = msg[i];
		idx_response++;
	}

	response[idx_response] = '\0';
	return response;
}

char* ws_receiveframe(unsigned char *frame, size_t length, int *type)
{
	char *msg;     /* Decoded message.        */
	uint8_t mask;           /* Payload is masked?      */
	uint8_t flength;        /* Raw length.             */
	uint8_t idx_first_mask; /* Index masking key.      */
	uint8_t idx_first_data; /* Index data.             */
	size_t  data_length;    /* Data length.            */
	uint8_t masks[4];       /* Masking key.            */
	int     i,j;            /* Loop indexes.           */

	msg = NULL;
	
	/* Checks the frame type and parse the frame. */
	if (frame[0] == (WS_FIN | WS_FR_OP_TXT) )
	{
		*type = WS_FR_OP_TXT;
		idx_first_mask = 2;
		mask           = frame[1];
		flength        = mask & 0x7F;

		if (flength == 126)
			idx_first_mask = 4;
		else if (flength == 127)
			idx_first_mask = 10;

		idx_first_data = idx_first_mask + 4;
		data_length = length - idx_first_data;

		masks[0] = frame[idx_first_mask+0];
		masks[1] = frame[idx_first_mask+1];
		masks[2] = frame[idx_first_mask+2];
		masks[3] = frame[idx_first_mask+3];

		msg = malloc(sizeof(unsigned char) * (data_length+1) );
		printf("datalength: %ld\n",length);
		memset(msg, 0, data_length+1);
		for (i = idx_first_data, j = 0; i < length; i++, j++)
			msg[j] = frame[i] ^ masks[j % 4];

		msg[j] = '\0';
	}

	/* Close frame. */
	else if (frame[0] == (WS_FIN | WS_FR_OP_CLSE) )
		*type = WS_FR_OP_CLSE;
	
	/* Not supported frame yet. */
	else
		*type = frame[0] & 0x0F;

	return msg;
}

static void SSLConn_Free(SSLConn_CTX* ctx);
static void SSLConn_Close(SSLConn_CTX* ctx, SSLConn* sslConn);
static void SSLConn_FreeSSLConn(SSLConn_CTX* ctx);


/* The index of the command line option. */
int   myoptind = 0;
/* The current command line option. */
char* myoptarg = NULL;
#ifdef WOLFSSL_ASYNC_CRYPT
/* Global device identifier. */
static int devId = INVALID_DEVID;
#endif
/* The data to reply with. */
static char reply[NUM_WRITE_BYTES];


/* Get the wolfSSL server method function for the specified version.
 *
 * version  Protocol version to use.
 * returns The server method function or NULL when version not supported.
 */
static wolfSSL_method_func SSL_GetMethod(int version, int allowDowngrade)
{
    wolfSSL_method_func method = wolfTLSv1_2_server_method;;
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

    /* Accept the connection. */
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
    puts("wolfSSL_accept error \n");
    return 0;
}

/* Create a new SSL/TLS connection data object.
 *
 * max        The maximum number of concurrent connections.
 * bufferLen  The number of data bytes to read from client.
 * replyLen   The number of data bytes to write to client.
 * maxConns   The number of connections to process this run.
 *            -1 indicates no maximum.
 * maxBytes   The number of bytes to send this run.
 *            -1 indicates no maximum.
 * returns an allocated and initialized connection data object or NULL on error.
 */
static SSLConn_CTX* SSLConn_New(int numConns, int bufferLen, int replyLen, int maxBytes)
{
    SSLConn_CTX* ctx;

    ctx = (SSLConn_CTX*)malloc(sizeof(*ctx));
    if (ctx == NULL)
        return NULL;
    memset(ctx, 0, sizeof(*ctx));

    ctx->numConns = numConns;
    ctx->bufferLen = bufferLen;
    ctx->replyLen = replyLen;
    ctx->maxBytes = maxBytes;
    ctx->sslConn = NULL;
	


    return ctx;
}

/* Free the SSL/TLS connection data.
 *
 * ctx  The connection data.
 */
static void SSLConn_Free(SSLConn_CTX* ctx)
{
    if (ctx == NULL)
        return;

    while (ctx->sslConn != NULL)
        SSLConn_Close(ctx, ctx->sslConn);
    SSLConn_FreeSSLConn(ctx);

    free(ctx);
}

/* Close an active connection.
 *
 * ctx      The SSL/TLS connection data.
 * sslConn  The SSL connection data object.
 */
static void SSLConn_Close(SSLConn_CTX* ctx, SSLConn* sslConn)
{
    if (sslConn->state == CLOSED)
        return;

    /* Display cipher suite all connection will use. */
    if (ctx->numConnections == 0) {
        WOLFSSL_CIPHER* cipher;
        cipher = wolfSSL_get_current_cipher(sslConn->ssl);
        printf("SSL cipher suite is %s\n",
               wolfSSL_CIPHER_get_name(cipher));
    }

    if (wolfSSL_session_reused(sslConn->ssl))
        ctx->numResumed++;
    ctx->numConnections++;

    sslConn->state = CLOSED;

    /* Take it out of the double-linked list. */
    if (ctx->sslConn == sslConn)
        ctx->sslConn = sslConn->next;
    if (sslConn->next != NULL)
        sslConn->next->prev = sslConn->prev;
    if (sslConn->prev != NULL)
        sslConn->prev->next = sslConn->next;

    /* Put object at head of free list */
    sslConn->next = ctx->freeSSLConn;
    sslConn->prev = NULL;
    ctx->freeSSLConn = sslConn;

    ctx->cnt--;
}

/* Free the SSL/TLS connections that are closed.
 *
 * ctx  The connection data.
 */
static void SSLConn_FreeSSLConn(SSLConn_CTX* ctx)
{
    SSLConn* sslConn = ctx->freeSSLConn;

    ctx->freeSSLConn = NULL;

    while (sslConn != NULL) {
        SSLConn* next = sslConn->next;

        wolfSSL_free(sslConn->ssl);
        close(sslConn->sockfd);
        free(sslConn);

        sslConn = next;
    }
}

/* Accepts a new connection.
 *
 * ctx      The SSL/TLS connection data.
 * sslCtx   The SSL/TLS context.
 * sockfd   The socket file descriptor to accept on.
 * sslConn  The newly create SSL connection data object.
 * returns EXIT_SUCCESS if a new connection was accepted or EXIT_FAILURE
 * otherwise.
 */
static int SSLConn_Accept(SSLConn_CTX* ctx, WOLFSSL_CTX* sslCtx,
                          socklen_t sockfd, SSLConn** sslConn)
{
    struct sockaddr_in clientAddr = {0};
    socklen_t          size = sizeof(clientAddr);
    SSLConn*           conn;

    if (ctx->cnt == ctx->numConns) {
        fprintf(stderr, "ERROR: Too many connections!\n");
        return EXIT_FAILURE;
    }

    conn = malloc(sizeof(*conn));
    conn->new_created=1;
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
    conn->next = ctx->sslConn;
    conn->prev = NULL;
    if (ctx->sslConn != NULL)
        ctx->sslConn->prev = conn;

    ctx->sslConn = conn;
    ctx->cnt++;

    *sslConn = conn;

    return EXIT_SUCCESS;
}

/* Read/write from/to client at the specified socket.
 *
 * ctx      The SSL/TLS connection data.
 * sslConn  The SSL connection data object.
 * returns EXIT_FAILURE on failure and EXIT_SUCCESS otherwise.
 */
static int SSLConn_ReadWrite(SSLConn_CTX* ctx, SSLConn* sslConn)
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
                SSLConn_Close(ctx, sslConn);
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
				
                int waf = simple_waf(buffer, len);
				if( waf== 1){
					char* yarablock="HTTP/1.1 200 OK\r\n"
									"Server: asm_server\r\n"
									"Content-Type:text/html\r\n"
									"Connection: Closed\r\n\r\n"
									"Waf is here, go away hacker!\0";
					wolfSSL_write(sslConn->ssl, yarablock, strlen(yarablock));
					SSLConn_Close(ctx, sslConn);
					 return EXIT_SUCCESS;
					}
                if (ret == 0) {
                    SSLConn_Close(ctx, sslConn);
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
		   SSLConn_Close(ctx, sslConn);
		   sslConn->state = CLOSED;
            break;

        case CLOSED:
            break;
    }
    return EXIT_SUCCESS;
}


/* Initialize the wolfSSL library and create a wolfSSL context.
 *
 * version      The protocol version.
 * cert         The server's certificate.
 * key          The server's private key matching the certificate.
 * verifyCert   The certificate for client authentication.
 * cipherList   The list of negotiable ciphers.
 * wolfsslCtx  The new wolfSSL context object.
 * returns EXIT_SUCCESS when a wolfSSL context object is created and
 * EXIT_FAILURE otherwise.
 */
static int WolfSSLCtx_Init(int version, int allowDowngrade, char* cert,
    char* key, char* verifyCert, char* cipherList, WOLFSSL_CTX** wolfsslCtx)
{
    WOLFSSL_CTX* ctx;
    wolfSSL_method_func method = NULL;

    method = SSL_GetMethod(version, allowDowngrade);
    if (method == NULL)
        return(EXIT_FAILURE);

    /* Create and initialize WOLFSSL_CTX structure */
    if ((ctx = wolfSSL_CTX_new(method(NULL))) == NULL) {
        fprintf(stderr, "wolfSSL_CTX_new error.\n");
        return(EXIT_FAILURE);
    }

#ifdef WOLFSSL_ASYNC_CRYPT
    if (wolfAsync_DevOpen(&devId) != 0) {
        fprintf(stderr, "Async device open failed\nRunning without async\n");
    }

    wolfSSL_CTX_UseAsync(ctx, devId);
#endif

    /* Load server certificate into WOLFSSL_CTX */
    if (wolfSSL_CTX_use_certificate_file(ctx, cert, SSL_FILETYPE_PEM)
            != SSL_SUCCESS) {
        fprintf(stderr, "Error loading %s, please check the file.\n", cert);
        wolfSSL_CTX_free(ctx);
        return(EXIT_FAILURE);
    }

    /* Load server key into WOLFSSL_CTX */
    if (wolfSSL_CTX_use_PrivateKey_file(ctx, key, SSL_FILETYPE_PEM)
            != SSL_SUCCESS) {
        fprintf(stderr, "Error loading %s, please check the file.\n", key);
        wolfSSL_CTX_free(ctx);
        return(EXIT_FAILURE);
    }

    /* Setup client authentication. */
    wolfSSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, 0);
    if (wolfSSL_CTX_load_verify_locations(ctx, verifyCert, 0) != SSL_SUCCESS) {
        fprintf(stderr, "Error loading %s, please check the file.\n",
                verifyCert);
        wolfSSL_CTX_free(ctx);
        return(EXIT_FAILURE);
    }

    if (cipherList != NULL) {
        if (wolfSSL_CTX_set_cipher_list(ctx, cipherList) != SSL_SUCCESS) {
            fprintf(stderr, "Server can't set cipher list.\n");
            wolfSSL_CTX_free(ctx);
            return(EXIT_FAILURE);
        }
    }

#ifndef NO_DH
    SetDHCtx(ctx);
#endif

    *wolfsslCtx = ctx;
    return EXIT_SUCCESS;
}

/* Cleanup the wolfSSL context and wolfSSL library.
 *
 * ctx  The wolfSSL context object.
 */
static void WolfSSLCtx_Final(WOLFSSL_CTX* ctx)
{
    wolfSSL_CTX_free(ctx);
#ifdef WOLFSSL_ASYNC_CRYPT
    wolfAsync_DevClose(&devId);
#endif
}

/* Create a random reply.
 *
 * reply     The buffer to put the random data into.
 * replyLen  The amount of data to generate.
 */
static void RandomReply(char* reply, int replyLen)
{
 char* resp="HTTP/1.1 200 OK\r\nConnection: close\r\nContent-Type: text/html\r\n\r\n<h1>hello</h1>";
 reply=resp;
}


/* Create a socket to listen on and wait for first client.
 *
 * port        The port to listen on.
 * numClients  The number of clients for listen to support.
 * socketfd    The socket file descriptor to accept on.
 * returns EXIT_SUCCESS on success and EXIT_FAILURE otherwise.
 */
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

    /* Create a non-blocking socket to listen on for new connections. */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == (socklen_t)-1) {
        fprintf(stderr, "ERROR: failed to create the socket\n");
        return(EXIT_FAILURE);
    }
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, len) < 0)
        fprintf(stderr, "setsockopt SO_REUSEADDR failed\n");
    if (setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &on, len) < 0)
        fprintf(stderr, "setsockopt TCP_NODELAY failed\n");

    struct timeval timeout;      
				timeout.tv_sec = 2;
				timeout.tv_usec = 1;

				if (setsockopt (sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,
                sizeof(timeout)) < 0)
				puts("setsockopt failed\n");

				if (setsockopt (sockfd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout,
                sizeof(timeout)) < 0)
				puts("setsockopt failed\n");

    if (bind(sockfd, (struct sockaddr *)&serverAddr,
             sizeof(serverAddr)) < 0) {
        fprintf(stderr, "ERROR: failed to bind\n");
        return(EXIT_FAILURE);
    }

    printf("Waiting for a connection...\n");

    /* Listen for a client to connect. */
    ret = listen(sockfd, numClients);
    if (ret == -1) {
        fprintf(stderr, "ERROR: failed to listen\n");
        return(EXIT_FAILURE);
    }

    *socketfd = sockfd;
    return EXIT_SUCCESS;
}

int out=0;
void  INThandler(int sig)
{
out=1;
}

/* returns 0 on success and 1 otherwise.*/
int main(int argc, char* argv[])
{
    int                 ret = 0;
    socklen_t           socketfd = -1;
    int                 efd;
    struct epoll_event  event;
    struct epoll_event  event_conn;
    struct epoll_event* events = NULL;
    WOLFSSL_CTX*        ctx = NULL;
    SSLConn_CTX*        sslConnCtx;
    word16              port          = wolfSSLPort;
    char*               cipherList    = NULL;
    char*               ourCert       = SVR_CERT;
    char*               ourKey        = SVR_KEY;
    char*               verifyCert    = CLI_CERT;
    int                 version       = SERVER_DEFAULT_VERSION;
    int                 allowDowngrade= 0;
    int                 numConns      = SSL_NUM_CONN;
    int                 numBytesRead  = NUM_READ_BYTES;
    int                 numBytesWrite = NUM_WRITE_BYTES;
    int                 maxBytes      = MAX_BYTES;
    int                 numClients    = NUM_CLIENTS;
#ifdef WOLFSSL_ASYNC_CRYPT
    WOLF_EVENT*         wolfEvents[MAX_WOLF_EVENTS];
#endif

	signal(SIGINT, INThandler);
	signal(SIGPIPE, SIG_IGN); // ignore broken pipe signal
    /* Allocate space for EPOLL events to be stored. */
    events = (struct epoll_event*)malloc(EPOLL_NUM_EVENTS * sizeof(*events));
    if (events == NULL)
        exit(EXIT_FAILURE);

#ifdef DEBUG_WOLFSSL
    wolfSSL_Debugging_ON();
#endif

	controllercall();
	globalinit_cache();
	sqlite_init_function();
	init_callback_sql();
	signal(SIGINT, INThandler);
	signal(SIGPIPE, SIG_IGN); // ignore broken pipe signal
    /* Initialize wolfSSL */
    wolfSSL_Init();

    /* Initialize wolfSSL and create a context object. */
    if (WolfSSLCtx_Init(version, allowDowngrade, ourCert, ourKey, verifyCert, cipherList, &ctx)
            == -1)
        exit(EXIT_FAILURE);

    RandomReply(reply, sizeof(reply));

    /* Create SSL/TLS connection data object. */
    sslConnCtx = SSLConn_New(numConns, numBytesRead, numBytesWrite, maxBytes);
    if (sslConnCtx == NULL)
        exit(EXIT_FAILURE);

    /* Create a socket and listen for a client. */
    if (CreateSocketListen(port, numClients, &socketfd) == EXIT_FAILURE)
        exit(EXIT_FAILURE);

    /* Create an EPOLL file descriptor. */
    efd = epoll_create1(0);
    if (efd == -1) {
        fprintf(stderr, "ERROR: failed to create epoll\n");
        exit(EXIT_FAILURE);
    }

    /* Add the event for communications on listening socket. */
    memset(&event, 0, sizeof(event));
    event.events = EPOLLIN;
    event.data.ptr = NULL;
    ret = epoll_ctl(efd, EPOLL_CTL_ADD, socketfd, &event);
    if (ret == -1) {
        fprintf(stderr, "ERROR: failed to add event to epoll\n");
        exit(EXIT_FAILURE);
    }
    sslConnCtx->accepting = 1;

    /* Keep handling clients until done. */
    while (out!=1) {
        int n;
        int i;

#ifdef WOLFSSL_ASYNC_CRYPT
        /* Look for events. */
        n = epoll_wait(efd, events, EPOLL_NUM_EVENTS, 0);
#else
        /* Wait for events. */
        n = epoll_wait(efd, events, EPOLL_NUM_EVENTS, -1);
#endif
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
                    SSLConn_Close(sslConnCtx, events[i].data.ptr);
                    ret = epoll_ctl(efd, EPOLL_CTL_ADD, socketfd, &event);
                }
            }
            else if (events[i].data.ptr == NULL) {
                SSLConn* sslConn;

                /* Accept a new client on the listener. */
                ret = SSLConn_Accept(sslConnCtx, ctx, socketfd, &sslConn);
                if (ret  == EXIT_SUCCESS) {
                    /* Set EPOLL to check for events on the new socket. */
                    memset(&event_conn, 0, sizeof(event_conn));
                    event_conn.events = EPOLLIN | EPOLLET;
                    event_conn.data.ptr = sslConn;
                    ret = epoll_ctl(efd, EPOLL_CTL_ADD, sslConn->sockfd,
                                    &event_conn);
                    if (ret == -1) {
                        fprintf(stderr, "ERROR: failed add event to epoll\n");
                        exit(EXIT_FAILURE);
                    }
                }

                if (sslConnCtx->cnt == sslConnCtx->numConns) {
                    /* Don't accept any more TCP connections. */
                    ret = epoll_ctl(efd, EPOLL_CTL_DEL, socketfd, &event);
                    if (ret == -1) {
                        fprintf(stderr, "ERROR: failed delete epoll event\n");
                        exit(EXIT_FAILURE);
                    }
                    sslConnCtx->accepting = 0;
                }
            }
            else {
                SSLConn_ReadWrite(sslConnCtx, events[i].data.ptr);
            }
        }

        SSLConn_FreeSSLConn(sslConnCtx);

        /* Accept more connections again up to the maximum concurrent. */
        if (!sslConnCtx->accepting &&
            sslConnCtx->cnt < sslConnCtx->numConns) {
            ret = epoll_ctl(efd, EPOLL_CTL_ADD, socketfd, &event);
            if (ret == -1) {
                fprintf(stderr, "ERROR: failed add event to epoll\n");
                exit(EXIT_FAILURE);
            }
            sslConnCtx->accepting = 1;
        }
    }

    puts("CTRL+C I'm out.");
    if (socketfd != -1) close(socketfd);
    free(events);
    SSLConn_Free(sslConnCtx);
    WolfSSLCtx_Final(ctx);
    wolfSSL_Cleanup();
    for(int i=0; i<table.route_count; i++) sdsfree(table.routes[i].url);
	globalfree_cache();
	sqlite_close_function();
    exit(EXIT_SUCCESS);

return 0;
}

