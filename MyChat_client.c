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
	int client_socket;

	struct sockaddr_in server_addr;

	char buff[BUFF_SIZE];

	if(argc < 2)
	{
		printf("실행파일 포트번호\n");
		exit(1);
	}

	client_socket = socket(AF_INET, SOCK_STREAM, 0);
	if(-1 == client_socket)
	{
		perror("socket");
		exit(1);
	}

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(atoi(argv[1]));
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	if(-1 == connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)))
	{
		perror("connect");
		exit(1);
	}

	printf("write massge\n");

	while(1)
	{
		scanf(" %[^\n]s", buff);
		//write(client_socket, buff, BUFF_SIZE);

		if(send(client_socket, buff, BUFF_SIZE, 0) == -1)
		{
			perror("send");
			exit(1);
		}

		//read(client_socket, buff, BUFF_SIZE);

		//if(recv(client_socket, buff, BUFF_SIZE, 0) == -1)
		//{
		//	perror("recv");
		//	exit(1);
		//}

		//printf("%s\n", buff);
	}

	close(client_socket);

	return 0;
}
