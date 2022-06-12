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
//�����忡�� ����� �Լ� (�޽��� ������ �Լ�) 
void * recv_msg(void * arg);
//�����忡�� ����� �Լ� (�޽��� �޴� �Լ�)
void error_handling(char * arg);
//������ �ڵ鸵�ϴ� �Լ�. 

char name[NAME_SIZE] = "[DEFAULT]";
// ä�ÿ� ������ Ŭ���̾�Ʈ�� �̸� ���� �⺻ ������ ����. 
char msg[BUF_SIZE];
// �޽����� ���� ���� 

int main(int argc, char *argv[]) 
{
	int sock;
	// Ŭ���̾�Ʈ ������ ���ϵ�ũ���� 
	struct sockaddr_in serv_addr;
	// ���� �ּ��� ������ ���� ����ü 
	pthread_t snd_thread, rcv_thread;
	// �޽����� ������ ���� ������ ���� 
	void * thread_return;
	// �������� ��ȯ ���� ������ ����. 
	
	if(argc!=4) 
	{
		printf("Usage: %s <IP> <port> <name> \n", argv[0]);
		exit(1);			
	}  
	
	sprintf(name, "[%s]", argv[3]);
	sock = socket(PF_INET, SOCK_STREAM, 0);
	// PF_INET: IPv4 ���ͳ� �������� ü�踦 ����ϴ�
	// SOCK_STREAM: ���� ������ ����(TCP) ���� ������ ����.(����)
	
	memset(&serv_addr, 0, sizeof(serv_addr));
	// ���� �ּҸ� ������ ����ü�� ��������  0���� �ʱ�ȭ. 
	serv_addr.sin_family = AF_INET;
	//�ּ� ü�踦 AF_INET(IPv4)�� ����. 
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
	//sin_addr�� ����ü�̱� ������ �� �� s_addr ����
	//�Է¹��� IP(���ڿ�)�� inet_addr �Լ��� ���� 
	// �򿣵�� �����  32��Ʈ ���� ������ �ʱ�ȭ. 
	serv_addr.sin_port = htons(atoi(argv[2]));
	// �Է��� ��Ʈ ��ȣ�� ��Ʈ�� �ٲپ�  ��Ʈ��ũ ����Ʈ ������ ����. 	
	
	if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1)
		error_handling("connect() error");
	//������ ���� ��û 
	
	pthread_create(&snd_thread, NULL, send_msg, (void*)&sock);
	pthread_create(&rcv_thread, NULL, recv_msg, (void*)&sock);
	// �޽����� �ְ� �޴� �����带 ����(�� 1���� �� 2��)
	//���� �Ű������δ� sock, ������ �Լ��� send_msg, �޴� �Լ��� recv_msg 
	pthread_join(snd_thread, &thread_return);
	pthread_join(rcv_thread, &thread_return);
	// ���� ������ �������� ������ ��ٸ���.
	// ��ȯ�Ǹ� �ڿ��� �ݳ�, �޴� �������� ���� ���.
	// ��ȯ�Ǹ� �ڿ� �ݳ�. 
	close(sock);
	//���� ���� 
	return 0;
}

void * send_msg(void * arg)
{
	int sock=*((int*)arg);
	//���޹��� Ŭ���̾�Ʈ ���� ��ũ���͸� int ������ ��ȯ 
	char name_msg[NAME_SIZE+BUF_SIZE];
	//���� ���ڸ� ������ �迭 
	while(1) 
	{
		fgets(msg, BUF_SIZE, stdin);
		//�Է��� ���ڸ� msg�� ���� 
		if(!strcmp(msg, "q\n") || !strcmp(msg, "Q\n"))
		{
			close(sock);
			exit(0);
			//q�� Q�� �ԷµǸ� ���� ����. 
		}
		sprintf(name_msg, "%s %s", name, msg);
		write(sock, name_msg, strlen(name_msg));
		//name_msg�� Ŭ���̾�Ʈ�� �̸��� �Է��� �޽����� ����ȭ�Ͽ� �ʱ�ȭ.
		// ���� �������� name_msg�� ������ ����. 
	}
	return NULL;
}

void * recv_msg(void * arg)
{
	int sock=*((int*)arg);
	//���޹��� Ŭ���̾�Ʈ ���� ��ũ���͸� int ������ ��ȯ 
	char name_msg[NAME_SIZE+BUF_SIZE];
	//���� ���ڸ� ������ �迭 
	int str_len;
	while(1) 
	{
		str_len = read(sock, name_msg, NAME_SIZE+BUF_SIZE-1);
		//���� ���� ������ ���̸� ����. 
		if(str_len==-1)
			return (void*)-1;
		name_msg[str_len] = 0;
		fputs(name_msg, stdout);
		//���� ���� ���ڸ� ���. 
	}
	return NULL;
}

void error_handling(char *msg)
{
	fputs(msg, stderr);
	fputc("\n", stderr);
	exit(1);
}
