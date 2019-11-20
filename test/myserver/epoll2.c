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
#define MAXEVENTS 64

typedef struct wrapper{
	struct epoll_event *events;
	SSL* ssl[MAXEVENTS];
	}wrapper;

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
  wrapper mywrapper;

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
  mywrapper.events = calloc (MAXEVENTS, sizeof event);
  //mywrapper.ssl = calloc(MAXEVENTS, sizeof mywrapper.ssl);

  /* The event loop */
  while (1)
    {
      int n, i;

      n = epoll_wait (efd, mywrapper.events, MAXEVENTS, -1);
      for (i = 0; i < n; i++)
	{
	  if ((mywrapper.events[i].events & EPOLLERR) ||
              (mywrapper.events[i].events & EPOLLHUP) ||
              (!(mywrapper.events[i].events & EPOLLIN)))
	    {
              /* An error has occured on this fd, or the socket is not
               * ready for reading (why were we notified then?) */
	      fprintf (stderr, "epoll error. events=%u\n", mywrapper.events[i].events);
	      close (mywrapper.events[i].data.fd);
	      continue;
	    }

	  else if (sfd == mywrapper.events[i].data.fd)
	    {
              /* We have a notification on the listening socket, which
               * means one or more incoming connections. */
              while (1)
                {
                  struct sockaddr in_addr;
                  socklen_t in_len;
                  int infd;
#if 0
                  char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
#endif

                  in_len = sizeof in_addr;
                  infd = accept (sfd, &in_addr, &in_len);
                    mywrapper.ssl[i] = SSL_new(ctx);
					SSL_set_fd(mywrapper.ssl[i], infd);
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
                  //puts("ssl_accept happened");
			if (SSL_accept(mywrapper.ssl[i]) <= 0) {
				ERR_print_errors_fp(stderr);
				}
			else {
				 //count = read (mywrapper.events[i].data.fd, buf, sizeof buf);
                  count = SSL_read (mywrapper.ssl[i], buf, sizeof buf);
					printf("%s\n",buf);
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

                  /* Write the reply to connection */
                  //s = write (mywrapper.events[i].data.fd, reply, sizeof(reply));
                  s = SSL_write (mywrapper.ssl[i], reply, sizeof(reply));
                  if (s == -1)
                    {
                      perror ("write");
                      abort ();
                    }
                }
			}

              if (done)
                {
                  printf ("Closed connection on descriptor %d\n",
                          mywrapper.events[i].data.fd);

                  /* Closing the descriptor will make epoll remove it
                   * from the set of descriptors which are monitored. */
                  SSL_shutdown(mywrapper.ssl[i]);
				  SSL_free(mywrapper.ssl[i]);
                  close (mywrapper.events[i].data.fd);
                }
            }
        }
    }

  free (mywrapper.events);
  //free (mywrapper.ssl);

  close (sfd);
  SSL_CTX_free(ctx);
  cleanup_openssl();

  return EXIT_SUCCESS;
}
