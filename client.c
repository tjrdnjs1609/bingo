#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#define BUFF_SIZE 1024
#define PORT_NUM 9000

int main(int argc, char **argv)
{
        int client_socket;

        struct sockaddr_in server_addr;

        char buff[BUFF_SIZE];

        client_socket = socket(PF_INET, SOCK_STREAM, 0);
        if(-1 == client_socket)
        {
                perror("socket");
                exit(1);
        }

        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(PORT_NUM);
        server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

        if(-1 == connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)))
        {
                perror("connect");
                exit(1);
        }

        printf("write massge\n");  //수정 부분
        scanf(" %[^\n]s", buff);   //수정 부분
        write(client_socket, buff, BUFF_SIZE);
        read(client_socket, buff, BUFF_SIZE);
        printf("%s\n", buff);
        //close(client_socket);

        return 0;
}