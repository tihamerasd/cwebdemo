#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include "http_parser.h"
#include <string.h>
#include <assert.h>

#define MAX_HEADERS 10
#define MAX_ELEMENT_SIZE 500

static http_parser parser;
typedef struct message {
  const char *name; // for debugging purposes
  const char *raw;
  enum http_parser_type type;
  int method;
  int status_code;
  char request_path[MAX_ELEMENT_SIZE];
  char request_uri[MAX_ELEMENT_SIZE];
  char fragment[MAX_ELEMENT_SIZE];
  char query_string[MAX_ELEMENT_SIZE];
  char body[MAX_ELEMENT_SIZE];
  int num_headers;
  enum { NONE=0, FIELD, VALUE } last_header_element;
  char headers [MAX_HEADERS][2][MAX_ELEMENT_SIZE];
  int should_keep_alive;

  int message_begin_cb_called;
  int headers_complete_cb_called;
  int message_complete_cb_called;
} message;
message messages;

struct line {
  char *field;
  size_t field_len;
  char *value;
  size_t value_len;
};

#define CURRENT_LINE (&header[nlines-1])
#define MAX_HEADER_LINES 2000

static struct line header[MAX_HEADER_LINES];
static int nlines = 0;
static int last_was_value = 0;
static int num_messages;

int my_url_callback (http_parser *_, const char *at, size_t len){
	
	return 0;
}

int request_path_cb (http_parser *parser, const char *p, size_t len)
{
  strncat(messages.request_path, p, len);
  return 0;
}

int
request_uri_cb (http_parser *parser, const char *p, size_t len)
{
  strncat(messages.request_uri, p, len);
  return 0;
}

int
message_complete_cb (http_parser *parser)
{
  messages.method = parser->method;
  messages.status_code = parser->status_code;

  messages.message_complete_cb_called = 1;

  num_messages++;
  return 0;
}

int
on_header_field (http_parser *_, const char *at, size_t len)
{
  if (last_was_value) {
    nlines++;

    if (nlines == MAX_HEADER_LINES) ;// error!
    
    CURRENT_LINE->value = NULL;
    CURRENT_LINE->value_len = 0;

    CURRENT_LINE->field_len = len;
    CURRENT_LINE->field = malloc(len+1);
    strncpy(CURRENT_LINE->field, at, len);
      
  } else {

    CURRENT_LINE->field_len += len;
    CURRENT_LINE->field = realloc(CURRENT_LINE->field,
        CURRENT_LINE->field_len+1);
    strncat(CURRENT_LINE->field, at, len);
  }

  CURRENT_LINE->field[CURRENT_LINE->field_len] = '\0';
  last_was_value = 0;
  printf("%s\n", CURRENT_LINE->field);

return 0;
}

int
on_header_value (http_parser *_, const char *at, size_t len)
{
  if (!last_was_value) {
    CURRENT_LINE->value_len = len;
    CURRENT_LINE->value = malloc(len+1);
    strncpy(CURRENT_LINE->value, at, len);

  } else {
    CURRENT_LINE->value_len += len;
    CURRENT_LINE->value = realloc(CURRENT_LINE->value,
        CURRENT_LINE->value_len+1);
    strncat(CURRENT_LINE->value, at, len);
  }

  CURRENT_LINE->value[CURRENT_LINE->value_len] = '\0';

  last_was_value = 1;
  printf("%s\n", CURRENT_LINE->value);
return 0;
}

int main() {
    
  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  struct sockaddr_in server;
  server.sin_family = AF_INET;
  server.sin_port = htons(8080);
  server.sin_addr.s_addr = htonl(INADDR_ANY);
  bind(server_fd, (struct sockaddr*) &server, sizeof(server));
  listen(server_fd, 128);
  for (;;) {
    int client_fd = accept(server_fd, NULL, NULL);

    http_parser_settings settings;
    settings.on_url = my_url_callback;
    settings.on_headers_complete =message_complete_cb;
	settings.on_header_value = on_header_value;
	settings.on_header_field = on_header_field;
	http_parser_init(&parser, HTTP_REQUEST);

	size_t len = 80*1024, nparsed;
	char buf[len];
	ssize_t recved;

	recved = recv(client_fd, buf, len, 0);

	if (recved < 0) {
	/* Handle error. */
		close(client_fd);
}

	/* Start up / continue the parser.
	* Note we pass recved==0 to signal that EOF has been received.
	*/
	nparsed = http_parser_execute(&parser, &settings, buf, recved);
	if (parser.upgrade) {
		close(client_fd);
	} else if (nparsed != recved) {		
	close(client_fd);
	}
printf("%s\n messages");
    char response[] = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\nConnection: close\r\n\r\nHello, world!";
    for (int sent = 0; sent < sizeof(response); sent += send(client_fd, response+sent, sizeof(response)-sent, 0));
    close(client_fd);
  }

  return 0;
}
