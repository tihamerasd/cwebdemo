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

#include "../../dev/config.h"
#include "../backend/dynamic_string/sds.h"
#include "../backend/keyvalue.h"
#include "../backend/requester.h"
#include "../backend/controller.h"
#include "../backend/sql/sqlthings.h"
#include "../backend/webapplication_firewall/simple_waf.h"
#include <signal.h>

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
    char buffer[MAXBUF];
    int len=0;
    memset(buffer,0,MAXBUF);
    len=read(fd, buffer, sizeof buffer);
    int match = simple_waf(buffer, len);
	//int match=0;
	if( match == 1){
		char* simpleblock="HTTP/1.1 301 Moved Permanently\r\n"
						  "Location: /\r\n"
		                  "Set-Cookie: Hacker=TRUE\r\n"
		                  "Cache-Control: no-cache, no-store, must-revalidate\r\n"
		                  "Pragma: no-cache\r\n"
		                  "Expires: 0\r\n\r\n";
		send(fd, simpleblock, strlen(simpleblock),0);
		close(fd);
		return;
	}
	portable_requester(buffer, len);
    sds response=portable_responser();
	requestfree();
	send(fd, response, sdslen(response),0);
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
    
    short port = PORT;
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
