CC = gcc

ws:
	$(CC) ./backend/html_templater/flate.c -c -I.
	ar -r libflate.a flate.o
	$(CC) ./backend/http_parser/http_parser.c -c -I.
	$(CC) -std=gnu11 -pedantic -o wss_socket websocket_tls.c ./http_parser.o ./libflate.a \
		./backend/webapplication_firewall/simple_waf.c ./backend/sql/sqlthings.c ./backend/dynamic_string/sds.c ./backend/keyvalue.c \
		./backend/responser.c ./backend/requester.c -Wall -I/usr/local/include -I/home/tihi/cweb/wolfssl/wolfssl \
		-Os -pthread -L/usr/local/lib -L/home/tihi/cweb/wolfssl/wolfssl -lm -lwolfssl -lssl -lcrypto -lsqlite3

http:
	nasm -f elf64 server.asm -o server_asm.o;
	$(CC) ./backend/html_templater/flate.c -c -I.
	$(CC) ./backend/http_parser/http_parser.c -c -I.
	ar -r libflate.a flate.o
	$(CC) -pedantic -no-pie -fPIC server.c server_asm.o http_parser.o ./backend/requester.c \
		./backend/webapplication_firewall/simple_waf.c ./backend/sql/sqlthings.c ./libflate.a ./backend/keyvalue.c ./backend/responser.c \
		./backend/dynamic_string/sds.c -o asmengine_server -Wall -pthread -lsqlite3 -lwolfssl;
	
https_multi:
	$(CC) ./backend/html_templater/flate.c -c -I.
	ar -r libflate.a flate.o
	$(CC) ./backend/http_parser/http_parser.c -c -I.
	$(CC) -std=gnu11 -pedantic -o https_server server-https-epoll-threaded.c ./http_parser.o ./libflate.a\
		./backend/webapplication_firewall/simple_waf.c ./backend/sql/sqlthings.c ./backend/dynamic_string/sds.c\
		./backend/keyvalue.c ./backend/responser.c ./backend/requester.c -Wall -I/usr/local/include -I/home/tihi/cweb/wolfssl/wolfssl\
		-Os -pthread -L/usr/local/lib -L/home/tihi/cweb/wolfssl/wolfssl -lm -lwolfssl -lsqlite3
	
https_single:
	$(CC) ./backend/html_templater/flate.c -c -I.
	ar -r libflate.a flate.o
	$(CC) ./backend/http_parser/http_parser.c -c -I.
	$(CC) -std=gnu11 -pedantic -o singlethread_https_server https_single_thread.c ./http_parser.o ./libflate.a\
		./backend/webapplication_firewall/simple_waf.c ./backend/sql/sqlthings.c ./backend/dynamic_string/sds.c ./backend/keyvalue.c \
		./backend/responser.c ./backend/requester.c -Wall -I/usr/local/include -I/home/tihi/cweb/wolfssl/wolfssl \
		-Os -pthread -L/usr/local/lib -L/home/tihi/cweb/wolfssl/wolfssl -lm -lwolfssl -lsqlite3

clean:
	rm -r ./*.a;
	rm -r ./*.o;
	rm https_server
	rm asmengine_server
	rm wss_socket
	rm singlethread_https_server
	

#with openssl generate cert
#create build dir
#create db script
#copy all files to there
#maybe chroot too
real:	
