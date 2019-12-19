#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <string.h>

#include "mbedtls/config.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/certs.h"
#include "mbedtls/x509.h"
#include "mbedtls/ssl.h"
#include "mbedtls/net_sockets.h"
#include "mbedtls/error.h"

#include "./backend/keyvalue.h"
#include "./backend/requester.h"
#include "./backend/responser.h"
#include "./backend/controller.c"
#include "./backend/sql/sqlthings.h"
#include "backend/webapplication_firewall/simple_waf.h"

#if defined(MBEDTLS_SSL_CACHE_C)
#include "mbedtls/ssl_cache.h"
#endif

#if defined(MBEDTLS_MEMORY_BUFFER_ALLOC_C)
#include "mbedtls/memory_buffer_alloc.h"
#endif

#define MBEDTLS_SSL_CACHE_C
#define PORT_NUMBER "8081"
#define CERTFILE "certificate.pem"
#define KEYFILE  "key.pem"
#define DEBUG_LEVEL 0
#define MAX_NUM_THREADS 1000
#define MAXREADSIZE 1024*16
#define SOCKET_TIMEOUT 2

#define mbedtls_fprintf    fprintf
#define mbedtls_printf     printf
#define mbedtls_snprintf   snprintf
#define mbedtls_exit            exit
#define MBEDTLS_EXIT_SUCCESS    EXIT_SUCCESS
#define MBEDTLS_EXIT_FAILURE    EXIT_FAILURE

mbedtls_threading_mutex_t debug_mutex;

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


static void my_mutexed_debug( void *ctx, int level,
                      const char *file, int line,
                      const char *str )
{
    long int thread_id = (long int) pthread_self();

    mbedtls_mutex_lock( &debug_mutex );

    ((void) level);
    mbedtls_fprintf( (FILE *) ctx, "%s:%04d: [ #%ld ] %s",
                                    file, line, thread_id, str );
    fflush(  (FILE *) ctx  );

    mbedtls_mutex_unlock( &debug_mutex );
}

typedef struct {
    mbedtls_net_context client_fd;
    int thread_complete;
    const mbedtls_ssl_config *config;
} thread_info_t;

typedef struct {
    int active;
    thread_info_t   data;
    pthread_t       thread;
} pthread_info_t;

static thread_info_t    base_info;
static pthread_info_t   threads[MAX_NUM_THREADS];

static void *handle_ssl_connection( void *data )
{
	//if u want join comment pthread
	pthread_detach(pthread_self());
    int ret, len;
    thread_info_t *thread_info = (thread_info_t *) data;
    mbedtls_net_context *client_fd = &thread_info->client_fd;
	struct timeval timeout;      
    timeout.tv_sec = SOCKET_TIMEOUT;
    timeout.tv_usec = 0;
    if (setsockopt (client_fd->fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
        puts("setsockopt failed\n");

		if (setsockopt (client_fd->fd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout,sizeof(timeout)) < 0)
        puts("setsockopt failed\n");
    long int thread_id = (long int) pthread_self();
    unsigned char buf[MAXREADSIZE];
    mbedtls_ssl_context ssl;

    /* Make sure memory references are valid */
    mbedtls_ssl_init( &ssl );

    mbedtls_printf( "  [ #%ld ]  Setting up SSL/TLS data\n", thread_id );

    /*
     * 4. Get the SSL context ready
     */
    if( ( ret = mbedtls_ssl_setup( &ssl, thread_info->config ) ) != 0 )
    {
        mbedtls_printf( "  [ #%ld ]  failed: mbedtls_ssl_setup returned -0x%04x\n",
                thread_id, -ret );
        goto thread_exit;
    }

    mbedtls_ssl_set_bio( &ssl, client_fd, mbedtls_net_send, mbedtls_net_recv, NULL );

    /*
     * 5. Handshake
     */
    mbedtls_printf( "  [ #%ld ]  Performing the SSL/TLS handshake\n", thread_id );

    while( ( ret = mbedtls_ssl_handshake( &ssl ) ) != 0 )
    {
        if( ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE )
        {
            mbedtls_printf( "  [ #%ld ]  failed: mbedtls_ssl_handshake returned -0x%04x\n",
                    thread_id, -ret );
            goto thread_exit;
        }
    }

    mbedtls_printf( "  [ #%ld ]  ok\n", thread_id );

    /*
     * 6. Read the HTTP Request
     */
    mbedtls_printf( "  [ #%ld ]  < Read from client\n", thread_id );

	sds response=NULL;
    do
    {
        len = sizeof( buf ) - 1;
        memset( buf, 0, sizeof( buf ) );
        ret = mbedtls_ssl_read( &ssl, buf, len );
        int match = simple_waf( (char*) buf, len);
				if( match == 1){
					char* yarablock="HTTP/1.1 200 OK\r\n"
									"Server: asm_server\r\n"
									"Content-Type:text/html\r\n"
									"Connection: Closed\r\n\r\n"
									"WAF is here, go away hacker!\0";
					response=sdsnew(yarablock);
				}
				else {
					portable_requester( (char*) buf, len);
					response=portable_responser();
					requestfree();
		
				}

        if( ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE )
            continue;

        if( ret <= 0 )
        {
            switch( ret )
            {
                case MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY:
                    mbedtls_printf( "  [ #%ld ]  connection was closed gracefully\n",
                            thread_id );
                            sdsfree(response);
                    goto thread_exit;

                case MBEDTLS_ERR_NET_CONN_RESET:
                    mbedtls_printf( "  [ #%ld ]  connection was reset by peer\n",
                            thread_id );
                            sdsfree(response);
                    goto thread_exit;

                default:
                    mbedtls_printf( "  [ #%ld ]  mbedtls_ssl_read returned -0x%04x\n",
                            thread_id, -ret );
                    sdsfree(response);
                    goto thread_exit;
            }
        }

        len = ret;
        mbedtls_printf( "  [ #%ld ]  %d bytes read\n=====\n%s\n=====\n",
                thread_id, len, (char *) buf );

        if( ret > 0 )
            break;
    }
    while( 1 );

    /*
     * 7. Write the 200 Response
     */
    mbedtls_printf( "  [ #%ld ]  > Write to client:\n", thread_id );

    while( ( ret = mbedtls_ssl_write( &ssl, (unsigned char*)response, sdslen(response) ) ) <= 0 )
    {
        if( ret == MBEDTLS_ERR_NET_CONN_RESET )
        {
            mbedtls_printf( "  [ #%ld ]  failed: peer closed the connection\n",
                    thread_id );
            sdsfree(response);
            goto thread_exit;
        }

        if( ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE )
        {
            mbedtls_printf( "  [ #%ld ]  failed: mbedtls_ssl_write returned -0x%04x\n",
                    thread_id, ret );
			sdsfree(response);
            goto thread_exit;
        }
    }

    len = ret;
    mbedtls_printf( "  [ #%ld ]  %d bytes written\n=====\n%s\n=====\n",
            thread_id, len, (char *) buf );

    mbedtls_printf( "  [ #%ld ]  . Closing the connection...", thread_id );

    while( ( ret = mbedtls_ssl_close_notify( &ssl ) ) < 0 )
    {
        if( ret != MBEDTLS_ERR_SSL_WANT_READ &&
            ret != MBEDTLS_ERR_SSL_WANT_WRITE )
        {
            mbedtls_printf( "  [ #%ld ]  failed: mbedtls_ssl_close_notify returned -0x%04x\n",
                    thread_id, ret );
			sdsfree(response);
            goto thread_exit;
        }
    }

    mbedtls_printf( " ok\n" );
		
    ret = 0;
sdsfree(response);
thread_exit:

#ifdef MBEDTLS_ERROR_C
    if( ret != 0 )
    {
        char error_buf[100];
        mbedtls_strerror( ret, error_buf, 100 );
        mbedtls_printf("  [ #%ld ]  Last error was: -0x%04x - %s\n\n",
               thread_id, -ret, error_buf );
    }
#endif

    mbedtls_net_free( client_fd );
    mbedtls_ssl_free( &ssl );

    thread_info->thread_complete = 1;

    return( NULL );
}

static int thread_create( mbedtls_net_context *client_fd )
{
    int ret, i;

    /*
     * Find in-active or finished thread slot
     */
     
    for( i = 0; i < MAX_NUM_THREADS; i++ )
    {
        if( threads[i].active == 0 )
            break;

        if( threads[i].data.thread_complete == 1 )
        {
            mbedtls_printf( "  [ main ]  Cleaning up thread %d\n", i );
			// If u don't use detached thread u need join
            //pthread_join(threads[i].thread, NULL );
            memset( &threads[i], 0, sizeof(pthread_info_t) );
            break;
        }
    }
	
    if( i == MAX_NUM_THREADS )
        return( -1 );

    /*
     * Fill thread-info for thread
     */
    memcpy( &threads[i].data, &base_info, sizeof(base_info) );
    threads[i].active = 1;
    memcpy( &threads[i].data.client_fd, client_fd, sizeof( mbedtls_net_context ) );

    if( ( ret = pthread_create( &threads[i].thread, NULL, handle_ssl_connection,
                                &threads[i].data ) ) != 0 )
    {
        return( ret );
    }

    return( 0 );
}

	mbedtls_net_context listen_fd;
	mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_ssl_config conf;
    mbedtls_x509_crt srvcert;
    mbedtls_x509_crt cachain;
    mbedtls_pk_context pkey;
#if defined(MBEDTLS_MEMORY_BUFFER_ALLOC_C)
    unsigned char alloc_buf[100000];
#endif
#if defined(MBEDTLS_SSL_CACHE_C)
    mbedtls_ssl_cache_context mbedtls_cache;
#endif

void mbedtls_clear_func(void){
    mbedtls_x509_crt_free( &srvcert );
    mbedtls_pk_free( &pkey );
#if defined(MBEDTLS_SSL_CACHE_C)
    mbedtls_ssl_cache_free( &mbedtls_cache );
#endif
    mbedtls_ctr_drbg_free( &ctr_drbg );
    mbedtls_entropy_free( &entropy );
    mbedtls_ssl_config_free( &conf );

    mbedtls_net_free( &listen_fd );

    mbedtls_mutex_free( &debug_mutex );

#if defined(MBEDTLS_MEMORY_BUFFER_ALLOC_C)
    mbedtls_memory_buffer_alloc_free();
#endif
	}

void  INThandler(int sig){
	puts("SIGINT catched!");
	for(int i=0; i<table.route_count; i++) sdsfree(table.routes[i].url);
	globalfree_cache();
	sqlite_close_function();
	mbedtls_clear_func();
	exit(0);
}

int main( void )
{
    int ret;
    mbedtls_net_context client_fd;
    const char pers[] = "ssl_pthread_server";

    mbedtls_x509_crt_init( &srvcert );
    mbedtls_x509_crt_init( &cachain );

    mbedtls_ssl_config_init( &conf );
    mbedtls_ctr_drbg_init( &ctr_drbg );
    memset( threads, 0, sizeof(threads) );
    mbedtls_net_init( &listen_fd );
    mbedtls_net_init( &client_fd );

    mbedtls_mutex_init( &debug_mutex );

    base_info.config = &conf;

    #if defined(MBEDTLS_MEMORY_BUFFER_ALLOC_C)
    mbedtls_memory_buffer_alloc_init( alloc_buf, sizeof(alloc_buf) );
#endif

#if defined(MBEDTLS_SSL_CACHE_C)
    mbedtls_ssl_cache_init( &mbedtls_cache );
#endif


    /*
     * We use only a single entropy source that is used in all the threads.
     */
    mbedtls_entropy_init( &entropy );

    controllercall();
	globalinit_cache();
	sqlite_init_function();
	init_callback_sql();

	signal(SIGINT, INThandler);
	signal(SIGPIPE, SIG_IGN); // ignore broken pipe signal

    /*
     * 1. Load the certificates and private RSA key
     */
    mbedtls_printf( "\n  . Loading the server cert. and key..." );
    fflush( stdout );

    /*
     * This demonstration program uses embedded test certificates.
     * Instead, you may want to use mbedtls_x509_crt_parse_file() to read the
     * server and CA certificates, as well as mbedtls_pk_parse_keyfile().
     */
    ret = mbedtls_x509_crt_parse_file( &srvcert, CERTFILE );
    if( ret != 0 )
    {
        mbedtls_printf( " failed\n  !  mbedtls_x509_crt_parse returned %d\n\n", ret );
        goto exit;
    }

    mbedtls_pk_init( &pkey );
    ret =  mbedtls_pk_parse_keyfile( &pkey, KEYFILE, NULL );
    if( ret != 0 )
    {
        mbedtls_printf( " failed\n  !  mbedtls_pk_parse_key returned %d\n\n", ret );
        goto exit;
    }

    mbedtls_printf( " ok\n" );

    /*
     * 1b. Seed the random number generator
     */
    mbedtls_printf( "  . Seeding the random number generator..." );

    if( ( ret = mbedtls_ctr_drbg_seed( &ctr_drbg, mbedtls_entropy_func, &entropy,
                               (const unsigned char *) pers,
                               strlen( pers ) ) ) != 0 )
    {
        mbedtls_printf( " failed: mbedtls_ctr_drbg_seed returned -0x%04x\n",
                -ret );
        goto exit;
    }

    mbedtls_printf( " ok\n" );

    /*
     * 1c. Prepare SSL configuration
     */
    mbedtls_printf( "  . Setting up the SSL data...." );

    if( ( ret = mbedtls_ssl_config_defaults( &conf,
                    MBEDTLS_SSL_IS_SERVER,
                    MBEDTLS_SSL_TRANSPORT_STREAM,
                    MBEDTLS_SSL_PRESET_DEFAULT ) ) != 0 )
    {
        mbedtls_printf( " failed: mbedtls_ssl_config_defaults returned -0x%04x\n",
                -ret );
        goto exit;
    }

    mbedtls_ssl_conf_rng( &conf, mbedtls_ctr_drbg_random, &ctr_drbg );
    mbedtls_ssl_conf_dbg( &conf, my_mutexed_debug, stdout );

    /* mbedtls_ssl_cache_get() and mbedtls_ssl_cache_set() are thread-safe if
     * MBEDTLS_THREADING_C is set.
     */
#if defined(MBEDTLS_SSL_CACHE_C)
    mbedtls_ssl_conf_session_cache( &conf, &mbedtls_cache,
                                   mbedtls_ssl_cache_get,
                                   mbedtls_ssl_cache_set );
#endif

    mbedtls_ssl_conf_ca_chain( &conf, &cachain, NULL );
    if( ( ret = mbedtls_ssl_conf_own_cert( &conf, &srvcert, &pkey ) ) != 0 )
    {
        mbedtls_printf( " failed\n  ! mbedtls_ssl_conf_own_cert returned %d\n\n", ret );
        goto exit;
    }

    mbedtls_printf( " ok\n" );

    /*
     * 2. Setup the listening TCP socket
     */
    mbedtls_printf( "  . Bind on https://localhost:"PORT_NUMBER"/ ..." );
    fflush( stdout );

    if( ( ret = mbedtls_net_bind( &listen_fd, NULL, PORT_NUMBER, MBEDTLS_NET_PROTO_TCP ) ) != 0 )
    {
        mbedtls_printf( " failed\n  ! mbedtls_net_bind returned %d\n\n", ret );
        goto exit;
    }

    mbedtls_printf( " ok\n" );

reset:

#ifdef MBEDTLS_ERROR_C
    if( ret != 0 )
    {
        char error_buf[100];
        mbedtls_strerror( ret, error_buf, 100 );
        mbedtls_printf( "  [ main ]  Last error was: -0x%04x - %s\n", -ret, error_buf );
    }
#endif


    /*
     * 3. Wait until a client connects
     */
    mbedtls_printf( "  [ main ]  Waiting for a remote connection\n" );

    if( ( ret = mbedtls_net_accept( &listen_fd, &client_fd,
                                    NULL, 0, NULL ) ) != 0 )
    {
        mbedtls_printf( "  [ main ] failed: mbedtls_net_accept returned -0x%04x\n", ret );
		puts("SG STRANGE HERE");
        //goto exit;
        ret=0;
        goto reset;
    }

    mbedtls_printf( "  [ main ]  ok\n" );
    mbedtls_printf( "  [ main ]  Creating a new thread\n" );

    if( ( ret = thread_create( &client_fd ) ) != 0 )
    {
        mbedtls_printf( "  [ main ]  failed: thread_create returned %d\n", ret );
        mbedtls_net_free( &client_fd );
        goto reset;
    }

    ret = 0;
    goto reset;

exit:
	mbedtls_clear_func();
    return( ret );
}
