#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netinet/tcp.h>
#include <linux/tls.h>
#include <string.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define on_error(...) { fprintf(stderr, __VA_ARGS__); fflush(stderr); exit(1); }

int main(){

 int server_fd, client_fd, err;
  struct sockaddr_in server, client;

  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  
  server.sin_family = AF_INET;
  server.sin_port = htons(PORT);
  server.sin_addr.s_addr = htonl(INADDR_ANY);
  err = bind(server_fd, (struct sockaddr *) &server, sizeof(server));

  err = listen(server_fd, 128);

  printf("Server is listening on %d\n", PORT);


  while (1) {

	if (client_fd < 0) on_error("Could not establish new connection\n");
		const char *msg = "HTTP/1.1 200 OK\r\nDate: Mon, 27 Jul 2009 12:28:53 GMT\r\nServer: Apache/2.2.14 (Win32)\r\nLast-Modified: Wed, 22 Jul 2009 19:15:56 GMT\r\nContent-Length: 88\r\nContent-Type: text/plain\r\nConnection: Closed;\r\n\r\nHello Wrold!";
		send(client_fd, msg, strlen(msg),0);

      close(client_fd);
}
return 0;
}
