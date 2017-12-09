// 헤더파일 모음
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>

// 클라이언트 설정
#define PORT 9000
#define SLEEP 500000

// 전역 변수
int Login = 0;
char Send_Kind[3];
char Recv_Kind[3];
char Send_Name[20];
char Recv_Name[20];
char Send_Talk[256];
char Recv_Talk[256];
char Send_Numb[3];
char Recv_Numb[3];
char W_Room_List[10][50];
char G_Room_List_Front[3];
char G_Room_Board[6][6][20];
char G_Room_Massage[100];

// 함수 프로토타입
void* ThreadRun();

int main(void)
{
	pthread_t Thread;

	strcpy(Send_Kind, "plg");
	
	pthread_create(&Thread, NULL, ThreadRun, NULL);

	while (1) // 로그인
	{
		printf("이름 입력 : ");
		scanf(" %[^\n]s", Send_Name);
		Login = 1;
		while (Login == 1);
		if (Login == 2)
		{
			break;
		}
	}

	while(1)
	{
		scanf(" %[^\n]s", Send_Talk);
	}

	return 0;
}

void* ThreadRun()
{
	int Client_Socket; // 클라이언트 소켓
	struct sockaddr_in Client_Addr; // 클라이언트 주소
	int Port = PORT; // 포트
	
	// 서버와 실시간 통신
	while(1)
	{
		while (Login == 0); // 로그인 준비중

		system("clear"); // 화면 지우기

		// 클라이언트 생성
		if ((Client_Socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		{
			printf("Client_Socket 생성 실패\n");
			exit(1);
		}
		memset(&Client_Addr, 0, sizeof(Client_Addr));
		Client_Addr.sin_family = AF_INET;
		Client_Addr.sin_port = htons(Port);
		Client_Addr.sin_addr.s_addr = inet_addr("127.0.0.1");
		if (connect(Client_Socket, (struct sockaddr *)&Client_Addr, sizeof(Client_Addr)) == -1)
		{
			printf("Client_Socket Connect 실패\n");
			exit(1);
		}

		// 서버 송신
		if(send(Client_Socket, Send_Kind, sizeof(Send_Kind), 0) == -1) printf("Send_Kind Fail\n");
		if(send(Client_Socket, Send_Name, sizeof(Send_Name), 0) == -1) printf("Send_Name Fail\n");
		if(send(Client_Socket, Send_Talk, sizeof(Send_Talk), 0) == -1) printf("Send_Talk Fail\n");
		if (send(Client_Socket, Send_Numb, sizeof(Send_Numb), 0) == -1) printf("Send_Numb Fail\n");
		if (send(Client_Socket, G_Room_List_Front, sizeof(G_Room_List_Front), 0) == -1) printf("G_Room_List_Front Fail\n");

		// 서버 수신
		if(recv(Client_Socket, Recv_Kind, sizeof(Recv_Kind), 0) == -1) printf("Recv_Kind Fail\n");
		if(recv(Client_Socket, Recv_Name, sizeof(Recv_Name), 0) == -1) printf("Recv_Name Fail\n");
		if(recv(Client_Socket, Recv_Talk, sizeof(Recv_Talk), 0) == -1) printf("Recv_Talk Fail\n");
		if (recv(Client_Socket, Recv_Numb, sizeof(Recv_Numb), 0) == -1) printf("Recv_Numb Fail\n");
		for (int i = 0; i<10; i++)
			if (recv(Client_Socket, W_Room_List[i], sizeof(W_Room_List[i]), 0) == -1) printf("W_Room_List Fail\n");
		if (recv(Client_Socket, G_Room_List_Front, sizeof(G_Room_List_Front), 0) == -1) printf("G_Room_List_Front Fail\n");
		for (int i = 0; i<6; i++)
			for (int j = 0; j<6; j++)
				if (recv(Client_Socket, G_Room_Board[i][j], sizeof(G_Room_Board[i][j]), 0) == -1) printf("G_Room_Board Fail\n");
		if (recv(Client_Socket, G_Room_Massage, sizeof(G_Room_Massage), 0) == -1) printf("G_Room_Massage Fail\n");

		// 확인용 출력
		/*printf("Send_Kind : %s\n", Send_Kind);
		printf("Send_Name : %s\n", Send_Name);
		printf("Send_Talk : %s\n", Send_Talk);
		printf("Send_Numb : %s\n", Send_Numb);
		printf("Recv_Kind : %s\n", Recv_Kind);
		printf("Recv_Name : %s\n", Recv_Name);
		printf("Recv_Talk : %s\n", Recv_Talk);
		printf("Recv_Numb : %s\n", Recv_Numb);
		printf("\n");*/

		// 정보 해석 및 변환
		if (strcmp(Recv_Kind, "alg") == 0) // 중복 접속
		{
			printf("이미 접속중인 이름\n");
			Login = 0;
		}
		else if (strcmp(Recv_Kind, "flg") == 0) // 서버 가득참
		{
			printf("서버가 가득참\n");
			Login = 0;
		}
		else if (strcmp(Recv_Kind, "slg") == 0) // 로그인 성공
		{
			printf("로그인 성공 : 대기방으로 이동\n");
			Login = 2;
			sprintf(Send_Numb, "%s", Recv_Numb);
			sprintf(Send_Kind, "%s","wtr");
			sprintf(G_Room_List_Front, "%s", "0");
		}
		else if (strcmp(Recv_Kind, "wtr") == 0) // 대기방
		{
			strcpy(Send_Kind, Recv_Kind);
			printf("대기방\n");
			for (int i = 0; i < 10; i++)
			{
				printf("%s\n", W_Room_List[i]);
			}
		}
		else // 게임방
		{
			strcpy(Send_Kind, Recv_Kind);
			for (int i = 0; i < 3; i++)
			{
				printf("%s", G_Room_Board[i][0]);
			}
			printf("\n");
			for (int j = 1; j < 6; j++)
			{
				for (int i = 0; i < 3; i++)
					printf("%s", G_Room_Board[i][j]);
				printf("\n");
			}
			printf("\n");
			for (int i = 3; i < 6; i++)
			{
				printf("%s", G_Room_Board[i][0]);
			}
			printf("\n");
			for (int j = 1; j < 6; j++)
			{
				for (int i = 3; i < 6; i++)
					printf("%s", G_Room_Board[i][j]);
				printf("\n");
			}
			printf("\n");
			printf("%s\n", G_Room_Massage);
		}

		// 클라이언트 삭제
		close(Client_Socket);

		// 초기화
		strcpy(Send_Talk, "");

		// 접속 대기
		usleep(SLEEP);
	}
}
