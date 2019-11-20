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
	int optval = 1;
	char* iv_write="12341234";
	char* seq_number_write="0";
	char* cipher_key_write="aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
	char* implicit_iv_write="1234";

 int server_fd, client_fd, err;
  struct sockaddr_in server, client;

  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  setsockopt(server_fd, SOL_TCP, TCP_ULP, "tls", sizeof("tls"));
  
  if (server_fd < 0) on_error("Could not create socket\n");

  server.sin_family = AF_INET;
  server.sin_port = htons(PORT);
  server.sin_addr.s_addr = htonl(INADDR_ANY);
	setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (void *) &optval, sizeof(int));
  err = bind(server_fd, (struct sockaddr *) &server, sizeof(server));
  //if (err < 0) on_error("Could not bind socket\n");

  err = listen(server_fd, 128);
  if (err < 0) on_error("Could not listen on socket\n");

  printf("Server is listening on %d\n", PORT);


  while (1) {
/*
	  struct tls12_crypto_info_aes_gcm_128 crypto_info;
    socklen_t client_len = sizeof(client);
    client_fd = accept(server_fd, (struct sockaddr *) &client, &client_len);

setsockopt(server_fd, SOL_TCP, TCP_ULP, "tls", sizeof("tls"));
setsockopt(server_fd, SOL_TLS, TLS_TX, &crypto_info, sizeof(crypto_info));
setsockopt(client_fd, SOL_TCP, TCP_ULP, "tls", sizeof("tls"));
crypto_info.info.version = TLS_1_2_VERSION;
crypto_info.info.cipher_type = TLS_CIPHER_AES_GCM_128;
memcpy(crypto_info.iv, iv_write, TLS_CIPHER_AES_GCM_128_IV_SIZE);
memcpy(crypto_info.rec_seq, seq_number_write,TLS_CIPHER_AES_GCM_128_REC_SEQ_SIZE);
memcpy(crypto_info.key, cipher_key_write, TLS_CIPHER_AES_GCM_128_KEY_SIZE);
memcpy(crypto_info.salt, implicit_iv_write, TLS_CIPHER_AES_GCM_128_SALT_SIZE);
setsockopt(client_fd, SOL_TLS, TLS_TX, &crypto_info, sizeof(crypto_info));*/


	if (client_fd < 0) on_error("Could not establish new connection\n");
		const char *msg = "HTTP/1.1 200 OK\r\nDate: Mon, 27 Jul 2009 12:28:53 GMT\r\nServer: Apache/2.2.14 (Win32)\r\nLast-Modified: Wed, 22 Jul 2009 19:15:56 GMT\r\nContent-Length: 88\r\nContent-Type: text/plain\r\nConnection: Closed;\r\n\r\nHello Wrold!";
		send(client_fd, msg, strlen(msg),0);

      close(client_fd);
}
return 0;
}
