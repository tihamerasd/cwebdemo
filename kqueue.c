#include <sys/socket.h>
#include <sys/event.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "./backend/keyvalue.h"
#include "./backend/requester.h"
#include "./backend/responser.h"
#include "./backend/controller.c"
#include "./backend/sql/sqlthings.h"
#include <signal.h>
#include "backend/webapplication_firewall/simple_waf.h"

#define KMAXEVENTS 10000
const int kReadEvent = 1;

int out=0; //global variable gives information about if sigint happened on server side (exit method).

void  INThandler(int sig)
{	 out=1;	 
}

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

void updateEvents(int efd, int fd, int events, bool modify) {
    struct kevent ev[2];
    int n = 0;
    if (events & kReadEvent) {
        EV_SET(&ev[n++], fd, EVFILT_READ, EV_ADD|EV_ENABLE, 0, 0, (void*)(intptr_t)fd);
    } else if (modify){
        EV_SET(&ev[n++], fd, EVFILT_READ, EV_DELETE, 0, 0, (void*)(intptr_t)fd);
    }
    
    printf("%s fd %d events read %d write \n",modify ? "mod" : "add", fd, events & kReadEvent);
    kevent(efd, ev, n, NULL, 0, NULL);
}

void handleAccept(int efd, int fd) {
    struct sockaddr_in raddr;
    socklen_t rsz = sizeof(raddr);
    int cfd = accept(fd,(struct sockaddr *)&raddr,&rsz);
    struct sockaddr_in peer;
    socklen_t alen = sizeof(peer);
    getpeername(cfd, (struct sockaddr*)&peer, &alen);
    printf("accept a connection from %s\n", inet_ntoa(raddr.sin_addr));
    int flags = fcntl(fd, F_GETFL, 0); //set nonblocking socket
    fcntl(fd, F_SETFL, flags | O_NONBLOCK); //set nonblock
    updateEvents(efd, cfd, kReadEvent, false); //noify about read
    }

void handleRead(int efd, int fd) {
    char buffer[4096];
    int len=0;
    memset(buffer,0,4096);
    len=read(fd, buffer, sizeof buffer);
    int match = simple_waf(buffer, len);
	//int match=0;
	if( match == 1){
		char* yarablock="HTTP/1.1 200 OK\r\n"
						"Server: asm_server\r\n"
						"Content-Type:text/html\r\n"
						"Connection: Closed\r\n\r\n"
						"WAF is here, go away hacker!\0";
		write(fd, yarablock, strlen(yarablock));
		close(fd);
		return;
	}
	portable_requester(buffer, len);
    sds response=portable_responser();
	requestfree();
	write(fd, response, sdslen(response));
    sdsfree(response);
    close(fd);
}

int main() {
	controllercall();
	globalinit_cache();
	sqlite_init_function();
	init_callback_sql();
	signal(SIGINT, INThandler);
	signal(SIGPIPE, SIG_IGN); // ignore broken pipe signal
    
    short port = 80;
    int epollfd = kqueue();
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof addr);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;
    int r = bind(listenfd,(struct sockaddr *)&addr, sizeof(struct sockaddr));
    r = listen(listenfd, 20);
    printf("fd %d listening at %d\n", listenfd, port);
    int flags = fcntl(listenfd, F_GETFL, 0); //set nonblocking socket
    fcntl(listenfd, F_SETFL, flags | O_NONBLOCK); //set nonblock
    updateEvents(epollfd, listenfd, kReadEvent, false);
    
    while(out != 1) {
		struct kevent activeEvs[KMAXEVENTS];
		int n = kevent(epollfd, NULL, 0, activeEvs, KMAXEVENTS, 0);
		printf("epoll_wait return %d\n", n);
		for (int i = 0; i < n; i ++) {
			int fd = (int)(intptr_t)activeEvs[i].udata;
			int events = activeEvs[i].filter;
			if (events == EVFILT_READ) {
				if (fd == listenfd) {
					handleAccept(epollfd, fd);
				} else {
					handleRead(epollfd, fd);
					}
			}
		}
    }
    for(int i=0; i<table.route_count; i++) sdsfree(table.routes[i].url);
	globalfree_cache();
	sqlite_close_function();
    return 0;
}
