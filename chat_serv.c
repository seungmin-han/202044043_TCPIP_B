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
//�����忡�� ����� �Լ� 
void * send_meg(char * msg, int len);
//Ŭ���̾�Ʈ�鿡�� �޽����� �����ϴ� �Լ�. 
void error_handling(char *msg);
//������ �ڵ鸵�ϴ� �Լ�. 

int clnt_cnt = 0;	
//���� ���� ���� Ŭ���̾�Ʈ�� �� 
int clnt_socks[MAX_CLNT];	
//���� ���� Ŭ���̾�Ʈ�� ���� �迭. 
pthread_mutex_t mutx;
//�����忡�� ���Ǵ� ���ؽ� ���� 


int main(int argc, char *argv[]) 
{
	int serv_sock, clnt_sock;
	// ���� ���ϰ� Ŭ���̾�Ʈ ������ ���ϵ�ũ���� 
	struct sockaddr_in serv_adr, clnt_adr;
	// ������ Ŭ���̾�Ʈ ������ ���� ����ü 
	int clnt_adr_sz;
	// Ŭ���̾�Ʈ ������ ���� ���� 
	pthread_t t_id;
	//�������� ���̵�(�ĺ���) 


	if(argc!=2)
	{
		printf("Usage : %s <port> \n", argv[0]);
		exit(1);
	}

	pthread_mutex_init(&mutx, NULL);
	serv_sock = socket(PF_INET, SOCK_STREAM, 0);
	// PF_INET: IPv4 ���ͳ� �������� ü�踦 ����ϴ�
	// SOCK_STREAM: ���� ������ ����(TCP) ���� ������ ����.(����)
	 
	memset(&serv_addr, 0, sizeof(serv_addr));
	// ���� �ּҸ� ������ ����ü�� ��������  0���� �ʱ�ȭ. 
	serv_addr.sin_family=AF_INET;
	//�ּ� ü�踦 AF_INET(IPv4)�� ����. 
	serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	//sin_addr�� ����ü�̱� ������ �� �� s_addr ���� 
	//host to network (long) �Լ��� INADDR_ANY(�� ������)�� 
	// ȣ��Ʈ ����Ʈ �������� ��Ʈ��ũ ����Ʈ ������ �ٲپ� �����Ѵ�. 
	serv_addr.sin_port = htons(atoi(argv[1]));
	// �Է��� ��Ʈ ��ȣ�� ��Ʈ�� �ٲپ�  ��Ʈ��ũ ����Ʈ ������ ����. 
	
	if(bind(serv_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) == -1)
	//���� ���Ͽ� �ּҸ� �Ҵ��Ѵ�. 
		error_handling("bind() error");
	if(listen(serv_sock,5)==-1)
	//��α׸� 5�� �ΰ� ���� ��û�� ����Ѵ�. 
		error_handling("listen() error");

	
	while(1)
	{
		clnt_adr_sz = sizeof(clnt_adr);
		//Ŭ���̾�Ʈ �ּ� ����� ����. 
		clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
		//Ŭ���̾�Ʈ ������ ������ ����ϸ� Ŭ���̾�Ʈ ������ ������ ����. 
		
		pthread_mutex_lock(&mutx);
		//���ؽ� ������ ����Ͽ� �ٸ� ���μ���(������)����
		//�Ӱ豸���� ����� �� ������ ���(lock) 
		clnt_socks[clnt_cnt++] = clnt_sock;
		//����� Ŭ���̾�Ʈ�� �����ϴ� �迭�� ����� Ŭ���̾�Ʈ ID�� ����. 
		pthread_mutex_unlock(&mutx);
		//�ٸ� ���μ���(������)���� �Ӱ豸���� ��� �����ϵ��� ��� ����(unlock) 
		
		pthread_create(&t_id, NULL, handle_clnt, (void)*&clnt_sock);
		//handle_clnt�Լ��� ������ ����.
		//������ ���̵� �����ϰ�, 
		//������ Ŭ���̾�Ʈ�� ���ϵ�ũ���͸� �Ķ���ͷ� ����. 
		pthread_detach(t_id);
		//�ش� �������� ���Ḧ ��ٸ��� �ʰ�,
		//�ش� ������ ���� ������ ��� �ڿ��� ����(�и�) 
		printf("Connected Client IP: %s \n", inet_ntoa(clnt_adr.sin_addr));	
		//��Ʈ��ũ ����Ʈ ������ Ŭ���̾�Ʈ �����Ǹ� char*(String) ���·� ���. 
	}
	close(serv_sock);
	return 0;
}

void * handle_clnt(void * arg)
{
	int clnt_sock = *((int*)arg);
	//���� ���� �Ķ���� ���� ��Ʈ������ ��ȯ
	int str_len = 0, i;
	char msg[BUF_SIZE];
	//�ؽ�Ʈ�� ���̸� ������ ������ 0���� �ʱ�ȭ
	//�޽����� ������ �迭�� ���̸� ���ۻ����� ��ŭ �ʱ�ȭ
	
	while((str_len=read(clnt_sock, msg, sizeof(msg)))!=0) 
		send_msg(msg, str_len);
	//�Ķ���ͷ� ���޹��� Ŭ���̾�Ʈ�� �޽����� read �Լ���
	//���Ͽ� msg ������ ���޹��� ���ڸ� ����.
	
	pthread_mutex_lock(&mutx);
	//�ٸ� ���μ���(������)���� ����� �� ������
	// �Ӱ� ���� ����(���(lock))
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
	// �Ӱ� ���� ��� ����(unlock) 
	close(clnt_sock);
	// Ŭ���̾�Ʈ ���� ���� ����. 
	return NULL;
}

void send_msg(char * msg, int len)
{
	int i;
	pthread_mutex_lock(&mutx);
	// �Ӱ� ���� ���(lock) 
	for(i=0;i<clnt_cnt;i++)
		write(clnt_socks[i], msg, len);
		//����� ��� Ŭ���̾�Ʈ�鿡�� �޽��� ����. 
	pthread_mutex_unlock(&mutx);
	// �Ӱ� ���� ��� ����(unlock) 
}

void error_handling(char *message) 
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
