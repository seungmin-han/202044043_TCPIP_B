#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define BUF_SIZE 100
#define MAX_CLNT 256



void * handle_clnt(void * arg);
//스레드에서 사용할 함수 
void * send_meg(char * msg, int len);
//클라이언트들에게 메시지를 전달하는 함수. 
void error_handling(char *msg);
//에러를 핸들링하는 함수. 

int clnt_cnt = 0;	
//현재 접속 중인 클라이언트의 수 
int clnt_socks[MAX_CLNT];	
//접속 중인 클라이언트를 담을 배열. 
pthread_mutex_t mutx;
//스레드에서 사용되는 뮤텍스 변수 


int main(int argc, char *argv[]) 
{
	int serv_sock, clnt_sock;
	// 서버 소켓과 클라이언트 소켓의 파일디스크립터 
	struct sockaddr_in serv_adr, clnt_adr;
	// 서버와 클라이언트 정보를 담을 구조체 
	int clnt_adr_sz;
	// 클라이언트 사이즈 저장 변수 
	pthread_t t_id;
	//스레스의 아이디(식별자) 


	if(argc!=2)
	{
		printf("Usage : %s <port> \n", argv[0]);
		exit(1);
	}

	pthread_mutex_init(&mutx, NULL);
	serv_sock = socket(PF_INET, SOCK_STREAM, 0);
	// PF_INET: IPv4 인터넷 프로토콜 체계를 사용하는
	// SOCK_STREAM: 연결 지향형 소켓(TCP) 으로 소켓을 생성.(서버)
	 
	memset(&serv_addr, 0, sizeof(serv_addr));
	// 서버 주소를 저장할 구조체의 변수들을  0으로 초기화. 
	serv_addr.sin_family=AF_INET;
	//주소 체계를 AF_INET(IPv4)로 설정. 
	serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	//sin_addr은 구조체이기 때문에 그 안 s_addr 값을 
	//host to network (long) 함수로 INADDR_ANY(내 아이피)를 
	// 호스트 바이트 순서에서 네트워크 바이트 순서로 바꾸어 저장한다. 
	serv_addr.sin_port = htons(atoi(argv[1]));
	// 입력한 포트 번호를 인트로 바꾸어  네트워크 바이트 순서로 저장. 
	
	if(bind(serv_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) == -1)
	//서버 소켓에 주소를 할당한다. 
		error_handling("bind() error");
	if(listen(serv_sock,5)==-1)
	//백로그를 5로 두고 연결 요청을 대기한다. 
		error_handling("listen() error");

	
	while(1)
	{
		clnt_adr_sz = sizeof(clnt_adr);
		//클라이언트 주소 사이즈를 저장. 
		clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
		//클라이언트 소켓의 연결을 허용하며 클라이언트 정보를 변수에 저장. 
		
		pthread_mutex_lock(&mutx);
		//뮤텍스 변수를 사용하여 다른 프로세스(스레드)에서
		//임계구역을 사용할 수 없도록 잠금(lock) 
		clnt_socks[clnt_cnt++] = clnt_sock;
		//연결된 클라이언트를 저장하는 배열에 연결된 클라이언트 ID를 저장. 
		pthread_mutex_unlock(&mutx);
		//다른 프로세스(스레드)에서 임계구역을 사용 가능하도록 잠금 해제(unlock) 
		
		pthread_create(&t_id, NULL, handle_clnt, (void)*&clnt_sock);
		//handle_clnt함수로 스레드 생성.
		//스레드 아이디를 저장하고, 
		//연결한 클라이언트의 파일디스크립터를 파라미터로 전달. 
		pthread_detach(t_id);
		//해당 스레드의 종료를 기다리지 않고,
		//해당 스레드 종료 시점에 모든 자원을 해제(분리) 
		printf("Connected Client IP: %s \n", inet_ntoa(clnt_adr.sin_addr));	
		//네트워크 바이트 순서의 클라이언트 아이피를 char*(String) 형태로 출력. 
	}
	close(serv_sock);
	return 0;
}

void * handle_clnt(void * arg)
{
	int clnt_sock = *((int*)arg);
	//전달 받은 파라미터 값을 인트형으로 변환
	int str_len = 0, i;
	char msg[BUF_SIZE];
	//텍스트의 길이를 저장할 변수를 0으로 초기화
	//메시지를 저장할 배열의 길이를 버퍼사이즈 만큼 초기화
	
	while((str_len=read(clnt_sock, msg, sizeof(msg)))!=0) 
		send_msg(msg, str_len);
	//파라미터로 전달받은 클라이언트의 메시지를 read 함수를
	//통하여 msg 변수에 전달받은 문자를 저장.
	
	pthread_mutex_lock(&mutx);
	//다른 프로세스(스레드)에서 사용할 수 없도록
	// 임계 구역 설정(잠금(lock))
	for(i=0;i<clnt_cnt; i++) 
	{
		if(clnt_sock==clnt_socks[i])
		{
			while(i++<clnt_cnt-1)
				clnt_socks[i]=clnt_sock[i+1];
			break;
		}
	}
	
	clnt_cnt--;
	pthread_mutex_unlock(&mutx);
	// 임계 구역 잠금 해제(unlock) 
	close(clnt_sock);
	// 클라이언트 연결 접속 종료. 
	return NULL;
}

void send_msg(char * msg, int len)
{
	int i;
	pthread_mutex_lock(&mutx);
	// 임계 구역 잠금(lock) 
	for(i=0;i<clnt_cnt;i++)
		write(clnt_socks[i], msg, len);
		//연결된 모든 클라이언트들에게 메시지 전달. 
	pthread_mutex_unlock(&mutx);
	// 임계 구역 잠금 해제(unlock) 
}

void error_handling(char *message) 
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
