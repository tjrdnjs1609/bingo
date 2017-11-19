#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#define BUFF_SIZE 1024

int main(int argc, char **argv)
{
	int server_socket;
	int client_socket;
	int client_addr_size;

	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;

	char buff_rcv[BUFF_SIZE+5];
	char buff_snd[BUFF_SIZE+5];

	int option = 1;

	int recvnum;

	if(argc < 2)
	{
		printf("실행파일 포트번호\n");
		exit(1);
	}

	server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if(-1 == server_socket)
	{
		perror("socket");
		exit(1);
	}

	setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(atoi(argv[1]));
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if(-1 == bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)))
	{
		perror("bind");
		exit(1);
	}

	if(-1 == listen(server_socket, 5))
	{
		perror("listen");
		exit(1);
	}

	while(1)
	{
		client_addr_size = sizeof(client_addr);
		client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_size);

		if(-1 == client_socket)
		{
			perror("accept");
			exit(1);
		}

		recvnum = 1024;

		while(recvnum == 1024)
		{
			//read(client_socket, buff_rcv, BUFF_SIZE);

			if((recvnum = recv(client_socket, buff_rcv, BUFF_SIZE, 0)) == -1)
			{
				perror("recv");
				exit(1);
			}
			else if(recvnum != 1024)
			{
				printf("User Disconnected.\n");
				break;
			}

			printf("%d : %s\n", recvnum, buff_rcv);

			sprintf(buff_snd, "%d : %s", (int)strlen(buff_rcv), buff_rcv);
			//write(client_socket, buff_snd, strlen(buff_snd)+1);

			//if(send(client_socket, buff_snd, strlen(buff_snd)+1, 0) == -1)
			//{
			//	perror("send");
			//	exit(1);
			//}
		}

		close(client_socket);
	}
}
