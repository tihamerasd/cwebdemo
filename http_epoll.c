#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>

#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "./backend/keyvalue.h"
#include "./backend/requester.h"
#include "./backend/responser.h"
#include "./backend/controller.c"
#include "./backend/sql/sqlthings.h"
#include "backend/webapplication_firewall/simple_waf.h"

#define MAX_EVENTS 1000
#define MAXBUF  4096*32

#define handle_error(msg) \
           do { perror(msg); exit(EXIT_FAILURE); } while (0)

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

int setnonblocking(int fd)
{
    int flags;
    if (-1 == (flags = fcntl(fd, F_GETFL, 0)))
        flags = 0;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

void do_use_fd(int epoll_fd, int fd)
{
	sds response=NULL;
	char buffer[MAXBUF];
	memset(buffer,0,MAXBUF);
	int len = recv(fd,buffer, MAXBUF,0);
	fwrite(buffer,1,len,stdout);
	if (len<0) return; //error handling
	int match = simple_waf(buffer, len);
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
		portable_requester(buffer, len);
		response=portable_responser();
		requestfree();
	}
    len = sdslen(response);
    send(fd, response, len, 0);
	sdsfree(response);

    if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL) == -1)
       handle_error("epoll delete");

    close(fd);
}

void run(int bind_port)
{
    struct epoll_event ev, events[MAX_EVENTS];
    struct sockaddr_in my_addr;
    struct sockaddr_in peer_addr;
    //char buffer[MAXBUF];
    int listen_sock, conn_sock, nfds, epoll_fd;
    int n = 0;

	memset(events,0,sizeof(events));
	memset(&ev,0,sizeof(ev));
    listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sock == -1)
        handle_error("socket");

    memset(&peer_addr, 0, sizeof(struct sockaddr_in));
    memset(&my_addr, 0, sizeof(struct sockaddr_in));
    socklen_t addrlen= sizeof(peer_addr);

    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(bind_port);
    my_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(listen_sock , (struct sockaddr*)&my_addr,
         sizeof my_addr) != 0)
        handle_error("bind");

    if(listen(listen_sock, 20) != 0)
        handle_error("listen");

    /* Initialize the epoll */
    epoll_fd = epoll_create(10);
    if (epoll_fd == -1) {
        handle_error("epoll_create");
    }

    ev.events = EPOLLIN;
    ev.data.fd = listen_sock;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_sock, &ev) == -1)
        handle_error("epoll_ctl: listen_sock");

    for (;;) {

        nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (nfds == -1)
            handle_error("epoll_wait");

         for (n = 0; n < nfds ; ++n) {
             if (events[n].data.fd == listen_sock) {
                 conn_sock = accept(listen_sock,
                     (struct sockaddr*) &peer_addr, &addrlen);
             if (conn_sock == -1)
                 handle_error("accept");
             setnonblocking(conn_sock);
             printf("%s:%d connected\n",
                 inet_ntoa(peer_addr.sin_addr), ntohs(peer_addr.sin_port));
             ev.events = EPOLLIN | EPOLLET;
             ev.data.fd = conn_sock;
             if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, conn_sock,
                 &ev) == -1)
                 handle_error("epoll_ctl: conn_sock");
             } else {
                 do_use_fd(epoll_fd, events[n].data.fd);
             }
         }
    }
}

void  INThandler(int sig){
	puts("OK I'm exit...");
	for(int i=0; i<table.route_count; i++) sdsfree(table.routes[i].url);
	globalfree_cache();
	sqlite_close_function();
    exit (0);
}

int main(int argc, char* argv[])
{int bind_port = -1;
	if( argc == 2 ) {
      printf("Server runing on: %s\n", argv[1]);
      bind_port= atoi(argv[1]);
   }
   else if( argc > 2 ) {
      printf("Too many arguments supplied.\n");
      exit(0);
   }
   else {
      printf("One argument expected.\n");
      exit(0);
   }
   controllercall();
	globalinit_cache();
	sqlite_init_function();
	init_callback_sql();
	signal(SIGINT, INThandler);
	signal(SIGPIPE, SIG_IGN); // ignore broken pipe signal
    run(bind_port);
//should never happen
return 0;
}
