/*made by Tihamer Darai*/
#include "./backend/keyvalue.h"
#include "./backend/requester.h"
#include "./backend/responser.h"
#include "./backend/controller.c"
#include <signal.h>
#include <pthread.h>

typedef struct th_arg {
    int* th_done;
    pthread_t* th_num;
} th_arg;

//int is_main(void)
//{
//  return getpid() == gettid();
//}

void  INThandler(int sig)
{
     char  c;
	//if (is_main){
     signal(sig, SIG_IGN);
     printf("You hitted Ctrl-C\n");
     c = getchar();
     //if (c == 'y' || c == 'Y'){
		for(int i=0; i<table.route_count; i++) sdsfree(table.routes[i].url);
		  globalfree_cache();
          //system("fuser -k 54321/tcp"); //TODO not a clear shutdown...should kill all thread  with kill syscall
      //    }
     getchar(); // Get new line character
     //}
}
void test_responser(http_request hrq){
	
sds response;

if (check_route(hrq.url)!=0) {
	response = do_route(&hrq);
	}
else{
	sds response_body = initdir_for_static_files(hrq.url);
	response = adddefaultheaders(hrq);
	printf("%s\n",response);
	response = sdscatsds(response,response_body);
    sdsfree(response_body);
	}

	register char* rsireg asm("rsi");
	register int   rdxreg asm("rdx");
	rdxreg = sdslen(response);
	rsireg = response;
    client_to_rdi();
    sdsfree(response);
}

http_request test_requester(void){
	sds req_str=sdsempty();
	char* raw_http_request[2048];
	give_me_socket();
	register int   mysocket asm("rdi");
	int siz = read(mysocket, raw_http_request, 2048);
	req_str=sdscatlen(req_str,raw_http_request, siz);
	memset(raw_http_request, 0, 2048);

	http_request req = create_request(req_str);
	sdsfree(req_str);
	return req;
		}

void *threadjob(void* p){
	pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; //initialize mutex for this thread
	th_arg *thargptr = (th_arg *)p;
	pthread_t *connfd_thread=thargptr->th_num;
	int *done=thargptr->th_done;

	//TODO multithread is useless because of this lock, we need to lock just write and read
	//TODO I gues it should crash without lock, but I can't do this.
	//pthread_mutex_lock(&mutex);
	http_request req = test_requester();
	test_responser(req);
	//pthread_mutex_unlock(&mutex);

	requestfree(req);
	
	closesock();
	*done = -1;
	//close(*connfd_thread);
	//fflush(stdout);
	pthread_exit(NULL);
	}
#define THREADNUMBER 100
int main(){
	printf("%s\n", "Server run on 54321");
	pthread_t threads[THREADNUMBER]; //100 posible threads, just to be safe
	int thread_count = 0;
	int connfd[THREADNUMBER];
	int thread_done[THREADNUMBER];
	for (int i=0; i<THREADNUMBER; i++)thread_done[i] = -1; // -1 means task ended successfully
	signal(SIGINT, INThandler);
	signal(SIGPIPE, SIG_IGN); // ignore broken pipe signal
	while (1) {
	
	extern int server_asm();
	initcode();
	_listen();

	//init the routes from the controller.c
	controllercall();
	globalinit_cache();
	while(1){
		//_accept();
		//http_request req = test_requester();
		//test_responser(req);
		//sdsfree(req.req_type);
		//sdsfree(req.url);
		//sdsfree(req.http_version);
		//closesock();
	while(thread_count<THREADNUMBER){
		if (thread_done[thread_count]!=-1){connfd[thread_count]--; continue; }; //skip long processes 
		if (thread_done[thread_count]==-2) pthread_join(threads[thread_count], NULL); // kill if the long process still run in next round
		connfd[thread_count]=_accept();

		th_arg tharg;
		tharg.th_done=&thread_done[thread_count];
		tharg.th_num=&threads[thread_count];

		pthread_create(&threads[thread_count], NULL, threadjob, &tharg);		//create a thread and receive data
		pthread_join(threads[thread_count], NULL);   //join the finished thread and continue
		thread_count++;
		printf("thread_count:%d\n",thread_count);
	}
	
	thread_count=0;
	}
}
	return 0;
}
