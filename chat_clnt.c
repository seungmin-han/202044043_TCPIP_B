#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#define BUF_SIZE 100
#define NAME_SIZE 20

void * send_msg(void * arg);
//스레드에서 사용할 함수 (메시지 보내는 함수) 
void * recv_msg(void * arg);
//스레드에서 사용할 함수 (메시지 받는 함수)
void error_handling(char * arg);
//에러를 핸들링하는 함수. 

char name[NAME_SIZE] = "[DEFAULT]";
// 채팅에 참여할 클라이언트의 이름 값을 기본 값으로 설정. 
char msg[BUF_SIZE];
// 메시지를 담을 변수 

int main(int argc, char *argv[]) 
{
	int sock;
	// 클라이언트 소켓의 파일디스크립터 
	struct sockaddr_in serv_addr;
	// 서버 주소의 정보를 담을 구조체 
	pthread_t snd_thread, rcv_thread;
	// 메시지를 보내고 받을 스레드 변수 
	void * thread_return;
	// 스레드의 반환 값을 저장할 변수. 
	
	if(argc!=4) 
	{
		printf("Usage: %s <IP> <port> <name> \n", argv[0]);
		exit(1);			
	}  
	
	sprintf(name, "[%s]", argv[3]);
	sock = socket(PF_INET, SOCK_STREAM, 0);
	// PF_INET: IPv4 인터넷 프로토콜 체계를 사용하는
	// SOCK_STREAM: 연결 지향형 소켓(TCP) 으로 소켓을 생성.(서버)
	
	memset(&serv_addr, 0, sizeof(serv_addr));
	// 서버 주소를 저장할 구조체의 변수들을  0으로 초기화. 
	serv_addr.sin_family = AF_INET;
	//주소 체계를 AF_INET(IPv4)로 설정. 
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
	//sin_addr은 구조체이기 때문에 그 안 s_addr 값을
	//입력받은 IP(문자열)을 inet_addr 함수를 통해 
	// 빅엔디안 방식의  32비트 정수 값으로 초기화. 
	serv_addr.sin_port = htons(atoi(argv[2]));
	// 입력한 포트 번호를 인트로 바꾸어  네트워크 바이트 순서로 저장. 	
	
	if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1)
		error_handling("connect() error");
	//서버에 연결 요청 
	
	pthread_create(&snd_thread, NULL, send_msg, (void*)&sock);
	pthread_create(&rcv_thread, NULL, recv_msg, (void*)&sock);
	// 메시지를 주고 받는 스레드를 생성(각 1개씩 총 2개)
	//전달 매개변수로는 sock, 보내는 함수는 send_msg, 받는 함수는 recv_msg 
	pthread_join(snd_thread, &thread_return);
	pthread_join(rcv_thread, &thread_return);
	// 먼저 보내는 스레드의 응답을 기다린다.
	// 반환되면 자원을 반납, 받는 스레드의 응답 대기.
	// 반환되면 자원 반납. 
	close(sock);
	//연결 종료 
	return 0;
}

void * send_msg(void * arg)
{
	int sock=*((int*)arg);
	//전달받은 클라이언트 파일 디스크립터를 int 형으로 변환 
	char name_msg[NAME_SIZE+BUF_SIZE];
	//보낼 문자를 저장할 배열 
	while(1) 
	{
		fgets(msg, BUF_SIZE, stdin);
		//입력한 문자를 msg에 저장 
		if(!strcmp(msg, "q\n") || !strcmp(msg, "Q\n"))
		{
			close(sock);
			exit(0);
			//q나 Q가 입력되면 연결 종료. 
		}
		sprintf(name_msg, "%s %s", name, msg);
		write(sock, name_msg, strlen(name_msg));
		//name_msg에 클라이언트의 이름과 입력한 메시지를 형식화하여 초기화.
		// 서버 소켓으로 name_msg의 내용을 전달. 
	}
	return NULL;
}

void * recv_msg(void * arg)
{
	int sock=*((int*)arg);
	//전달받은 클라이언트 파일 디스크립터를 int 형으로 변환 
	char name_msg[NAME_SIZE+BUF_SIZE];
	//받을 문자를 저장할 배열 
	int str_len;
	while(1) 
	{
		str_len = read(sock, name_msg, NAME_SIZE+BUF_SIZE-1);
		//전달 받은 문자의 길이를 저장. 
		if(str_len==-1)
			return (void*)-1;
		name_msg[str_len] = 0;
		fputs(name_msg, stdout);
		//전달 받은 문자를 출력. 
	}
	return NULL;
}

void error_handling(char *msg)
{
	fputs(msg, stderr);
	fputc("\n", stderr);
	exit(1);
}
