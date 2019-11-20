#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

void init_openssl()
{ 
    SSL_load_error_strings();	
    OpenSSL_add_ssl_algorithms();
}

void cleanup_openssl()
{
    EVP_cleanup();
}

SSL_CTX *create_context()
{
    const SSL_METHOD *method;
    SSL_CTX *ctx;

    method = SSLv23_server_method();

    ctx = SSL_CTX_new(method);
    if (!ctx) {
	perror("Unable to create SSL context");
	ERR_print_errors_fp(stderr);
	exit(EXIT_FAILURE);
    }

    return ctx;
}

void configure_context(SSL_CTX *ctx)
{
    SSL_CTX_set_ecdh_auto(ctx, 1);

    /* Set the key and cert */
    if (SSL_CTX_use_certificate_file(ctx, "caKey.pem", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
	exit(EXIT_FAILURE);
    }

    if (SSL_CTX_use_PrivateKey_file(ctx, "privateKey.pem", SSL_FILETYPE_PEM) <= 0 ) {
        ERR_print_errors_fp(stderr);
	exit(EXIT_FAILURE);
    }
}

static int
create_and_bind (char *port)
{
  struct addrinfo hints;
  struct addrinfo *result, *rp;
  int s, sfd;

  memset (&hints, 0, sizeof (struct addrinfo));
  hints.ai_family = AF_UNSPEC;     /* Return IPv4 and IPv6 choices */
  hints.ai_socktype = SOCK_STREAM; /* We want a TCP socket */
  hints.ai_flags = AI_PASSIVE;     /* All interfaces */

  s = getaddrinfo (NULL, port, &hints, &result);
  if (s != 0)
    {
      fprintf (stderr, "getaddrinfo: %s\n", gai_strerror (s));
      return -1;
    }

  for (rp = result; rp != NULL; rp = rp->ai_next)
    {
	  //InitializeSSL();
      sfd = socket (rp->ai_family, rp->ai_socktype, rp->ai_protocol);
      if (sfd == -1)
        continue;

      s = bind (sfd, rp->ai_addr, rp->ai_addrlen);
      if (s == 0)
        {
          /* We managed to bind successfully! */
          break;
        }

      close (sfd);
    }

  if (rp == NULL)
    {
      fprintf (stderr, "Could not bind\n");
      return -1;
    }

  freeaddrinfo (result);

  return sfd;
}

static int
make_socket_non_blocking (int sfd)
{
  int flags, s;

  flags = fcntl (sfd, F_GETFL, 0);
  if (flags == -1)
    {
      perror ("fcntl");
      return -1;
    }

  flags |= O_NONBLOCK;
  s = fcntl (sfd, F_SETFL, flags);
  if (s == -1)
    {
      perror ("fcntl");
      return -1;
    }

  return 0;
}

#define MAXEVENTS 1000

static const char reply[] =
"HTTP/1.0 200 OK\r\n"
"Content-type: text/html\r\n"
"Connection: close\r\n"
"Content-Length: 82\r\n"
"\r\n"
"<html>\n"
"<head>\n"
"<title>performance test</title>\n"
"</head>\n"
"<body>\n"
"test\n"
"</body>\n"
"</html>"
;

int
main (int argc, char *argv[])
{
  int sfd, s;
  int efd;
  struct epoll_event event;
  struct epoll_event *events;

  SSL_CTX *ctx;

    init_openssl();
    ctx = create_context();

    configure_context(ctx);

  if (argc != 2)
    {
      fprintf (stderr, "Usage: %s [port]\n", argv[0]);
      exit (EXIT_FAILURE);
    }

  sfd = create_and_bind (argv[1]);
  if (sfd == -1)
    abort ();

  s = make_socket_non_blocking (sfd);
  if (s == -1)
    abort ();

  s = listen (sfd, SOMAXCONN);
  if (s == -1)
    {
      perror ("listen");
      abort ();
    }

/*
while(1){
    struct sockaddr_in addr;
                  socklen_t in_len;
                  int infd;
                  SSL *ssl;
                  char buf2[2048];
#if 0
                  char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
#endif

                  in_len = sizeof addr;
                  infd = accept (sfd, (struct sockaddr*)&addr, &in_len);
                   ssl = SSL_new(ctx);
        SSL_set_fd(ssl, infd);

        if (SSL_accept(ssl) <= 0) {
            ERR_print_errors_fp(stderr);
        }
        else {
			SSL_read(ssl, buf2, sizeof buf2);
			printf("%s\n",buf2);
            SSL_write(ssl, reply, strlen(reply));
        }

        SSL_shutdown(ssl);
        SSL_free(ssl);
                  //exit(1);

}
*/
  efd = epoll_create1 (0);
  if (efd == -1)
    {
      perror ("epoll_create");
      abort ();
    }

  event.data.fd = sfd;
  event.events = EPOLLIN | EPOLLET;
  s = epoll_ctl (efd, EPOLL_CTL_ADD, sfd, &event);
  if (s == -1)
    {
      perror ("epoll_ctl");
      abort ();
    }

  /* Buffer where events are returned */
  events = calloc (MAXEVENTS, sizeof event);
  SSL *ssl;
  /* The event loop */
  while (1)
    {
      int n, i;

      n = epoll_wait (efd, events, MAXEVENTS, -1);
      for (i = 0; i < n; i++)
	{
	  if ((events[i].events & EPOLLERR) ||
              (events[i].events & EPOLLHUP) ||
              (!(events[i].events & EPOLLIN)))
	    {
              /* An error has occured on this fd, or the socket is not
               * ready for reading (why were we notified then?) */
	      fprintf (stderr, "epoll error. events=%u\n", events[i].events);
	      close (events[i].data.fd);
	      continue;
	    }

	  else if (sfd == events[i].data.fd)
	    {
              /* We have a notification on the listening socket, which
               * means one or more incoming connections. */

              while (1)
                {
                 struct sockaddr_in addr;
                  socklen_t in_len;
                  int infd;
#if 0
                  char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
#endif

                  in_len = sizeof addr;
///////////////////////////////////
                  SSL *ssl;
                  char buf2[2048];
//////////////////////////////////
                  infd = accept (sfd, (struct sockaddr*)&addr, &in_len);
/////////////////////////////////////////////////////////////////////////////////////////////////////////////6

                           ssl = SSL_new(ctx);
        SSL_set_fd(ssl, infd);

        if (SSL_accept(ssl) <= 0) {
            ERR_print_errors_fp(stderr);
        }
        else {
			SSL_read(ssl, buf2, sizeof buf2);
			printf("%s\n",buf2);
            SSL_write(ssl, reply, strlen(reply));
        }

        SSL_shutdown(ssl);
        SSL_free(ssl);
////////////////////////////////////////////////////////////////////////////////////////
                  
                  if (infd == -1)
                    {
printf("errno=%d, EAGAIN=%d, EWOULDBLOCK=%d\n", errno, EAGAIN, EWOULDBLOCK);
                      if ((errno == EAGAIN) ||
                          (errno == EWOULDBLOCK))
                        {
                          /* We have processed all incoming
                           * connections. */
                          printf ("processed all incoming connections.\n");
                          break;
                        }
                      else
                        {
                          perror ("accept");
                          break;
                        }
                    }

#if 0
                  s = getnameinfo (&in_addr, in_len,
                                   hbuf, sizeof hbuf,
                                   sbuf, sizeof sbuf,
                                   NI_NUMERICHOST | NI_NUMERICSERV);
                  if (s == 0)
                    {
                      printf("Accepted connection on descriptor %d "
                             "(host=%s, port=%s)\n", infd, hbuf, sbuf);
                    }
#endif

                  /* Make the incoming socket non-blocking and add it to the
                   * list of fds to monitor. */
                  s = make_socket_non_blocking (infd);
                  if (s == -1)
                    abort ();

                  event.data.fd = infd;
                  event.events = EPOLLIN | EPOLLET;
printf("set events %u, infd=%d\n", event.events, infd);
                  s = epoll_ctl (efd, EPOLL_CTL_ADD, infd, &event);
                  if (s == -1)
                    {
                      perror ("epoll_ctl");
                      abort ();
                    }
                }
/*              continue; */
            }
          else
            {
              /* We have data on the fd waiting to be read. Read and
               * display it. We must read whatever data is available
               * completely, as we are running in edge-triggered mode
               * and won't get a notification again for the same
               * data. */
              int done = 0;

              while (1)
                {
                  ssize_t count;
                  char buf[2048];

                  ssl = SSL_new(ctx);
				SSL_set_fd(ssl, s);

        if (SSL_accept(ssl) <= 0) {
            ERR_print_errors_fp(stderr);
        }
        else {
           count = SSL_read(ssl, buf, sizeof buf);
           printf("%s\n","buf");
        }

                  //count = read (events[i].data.fd, buf, sizeof buf);
                  if (count == -1)
                    {
                      /* If errno == EAGAIN, that means we have read all
                       * data. So go back to the main loop. */
                      if (errno != EAGAIN)
                        {
                          perror ("read");
                          done = 1;
                        }
                      break;
                    }
                  else if (count == 0)
                    {
                      /* End of file. The remote has closed the
                       * connection. */
                      done = 1;
                      break;
                    }
				s = SSL_write(ssl, reply, sizeof(reply));

                  /* Write the reply to connection */
                  //s = write (events[i].data.fd, reply, sizeof(reply));
                  if (s == -1)
                    {
                      perror ("write");
                      abort ();
                    }
                    close(events[i].data.fd);
                }

              if (done)
                {
                  printf ("Closed connection on descriptor %d\n",
                          events[i].data.fd);

                  /* Closing the descriptor will make epoll remove it
                   * from the set of descriptors which are monitored. */
                  SSL_shutdown(ssl);
        SSL_free(ssl);
   close (events[i].data.fd);
                }
            }
        }
    }

  free (events);

  close (sfd);

  return EXIT_SUCCESS;
}
