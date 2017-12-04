// 서버 소스코드

// 헤더파일 모음
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <time.h>

// 서버 설정 모음
#define PORT 9000
#define MAX_USER 50
#define MAX_ROOM 25
#define CHANS 10
#define MAX_SCORE 3
#define MAX_NUMBER 99

// 구조체
typedef struct user_list
{
	char Name[20];
	int Number;
	int Room;
	int Access;
}User_List_t;
typedef struct g_room_list
{
	int Access;
	int Number;
	char User[6][20];
	int User_Number[6];
	int Board[6][5][5];
	int Score[6];
	int Turn;
	int Max_Score;
	int Max_Number;
	int Play;
	char Massage[100];
}G_Room_List_t;

// 전역 변수
char Send_Kind[3]; // 유저 상태
char Recv_Kind[3];
char Send_Name[20]; // 유저 이름
char Recv_Name[20];
char Send_Talk[256]; // 유저 대화
char Recv_Talk[256];
char Send_Numb[3];
char Recv_Numb[3];
User_List_t User_List[MAX_USER]; // 유저 리스트
G_Room_List_t G_Room_List[MAX_ROOM]; // 게임방 리스트
char W_Room_List[10][50];
int Access_User = 0;
char G_Room_List_Front[3];
int G_Room_List_Front_int;
int Wait_Access = 0;
int Access_Name;
int Input_User;
char G_Room_Board[6][6][20];
char Temp_Buffer[5][5];
char G_Room_Massage[100];
int  Temp_Win;
int Temp_Sum;
char Temp_Play[30];

// 함수 프로토타입
void *ThreadRun(); // 서버 실시간 통신쓰레드

int main(void)
{
	pthread_t Thread;
	char Input[100];

	pthread_create(&Thread, NULL, ThreadRun, NULL); // 실시간 통신 쓰레드 생성

	

	// 서버관리자용 입력기
	while(1)
	{
		scanf(" %[^\n]s", Input);
	}

	return 0;
}

void *ThreadRun()
{
	int Server_Socket; // 서버 소켓
	int Client_Socket; // 클라이언트 소켓
	struct sockaddr_in Server_Addr; // 서버 주소
	struct sockaddr_in Client_Addr; // 클라이언트 주소
	int Client_Addr_Size;
	int Option = 1;
	int Port = PORT; // 포트

	srand(time(NULL)); // 난수발생

	// 서버 소켓 생성
	if ((Server_Socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		printf("Server_Socket 생성 실패\n");
		exit(1);
	}
	setsockopt(Server_Socket, SOL_SOCKET, SO_REUSEADDR, &Option, sizeof(Option));
	memset(&Server_Addr, 0, sizeof(Server_Addr));
	Server_Addr.sin_family = AF_INET;
	Server_Addr.sin_port = htons(Port);
	Server_Addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(Server_Socket, (struct sockaddr *)&Server_Addr, sizeof(Server_Addr)) == -1)
	{
		printf("Bind 실패\n");
		exit(1);
	}

	// 서버 소켓이 클라이언트를 받아들이게 함
	if(listen(Server_Socket, 5) == -1)
	{
		printf("Listen 실패\n");
		exit(1);
	} 

	// 모든 버퍼 초기화
	strcpy(Send_Kind, "");
	strcpy(Recv_Kind, "");
	strcpy(Send_Name, "");
	strcpy(Recv_Name, "");
	strcpy(Send_Talk, "");
	strcpy(Recv_Talk, "");
	strcpy(Send_Numb, "");
	strcpy(Recv_Numb, "");
	for(int i=0; i<MAX_USER; i++)
	{
		strcpy(User_List[i].Name, "");
		User_List[i].Access = 0;
		User_List[i].Number = -1;
		User_List[i].Room = -1;
	}
	for (int i = 0; i<MAX_ROOM; i++)
	{
		G_Room_List[i].Access = 0;
		G_Room_List[i].Play = 0;
		for (int j = 0; j < 6; j++)
		{
			strcpy(G_Room_List[i].User[j], "");
			G_Room_List[i].Score[j] = 0;
			G_Room_List[i].User_Number[j] = -1;
			for (int y = 0; y < 5; y++)
			{
				for (int x = 0; x < 5; x++)
				{
					G_Room_List[i].Board[j][x][y] = 0;
				}
			}
		}
	}

	// 클라이언트 소켓을 생성하고 통신
	while(1)
	{
		// 클라이언트 소켓 생성
		Client_Addr_Size = sizeof(Client_Addr);
		if (Client_Socket = accept(Server_Socket, (struct sockaddr *)&Client_Addr, &Client_Addr_Size) == -1)
		{
			printf("Server_Socket Client_Socket Accept 실패\n");
			exit(1);
		}

		// 클라이언트 수신
		if(recv(Client_Socket, Recv_Kind, sizeof(Recv_Kind), 0) == -1) printf("Recv_Kind Fail\n");
		if(recv(Client_Socket, Recv_Name, sizeof(Recv_Name), 0) == -1) printf("Recv_Name Fail\n");
		if(recv(Client_Socket, Recv_Talk, sizeof(Recv_Talk), 0) == -1) printf("Recv_Talk Fail\n");
		if (recv(Client_Socket, Recv_Numb, sizeof(Recv_Numb), 0) == -1) printf("Recv_Numb Fail\n");
		if (recv(Client_Socket, G_Room_List_Front, sizeof(G_Room_List_Front), 0) == -1) printf("G_Room_List_Front Fail\n");

		// 화면 지우기
		system("clear");

		//정보 해석 및 변환
		strcpy(Send_Name, Recv_Name);// 변화 없는 데이터
		strcpy(Send_Talk, Recv_Talk);
		strcpy(Send_Kind, Recv_Kind);
		strcpy(Send_Numb, Recv_Numb);

		if (strcmp(Recv_Kind, "wtr") == 0)
		{
			for (int i = 0; i < MAX_USER; i++)
			{
				if (strcmp(User_List[i].Name, Recv_Name) == 0)
				{
					sprintf(Recv_Numb, "%d", i);
					break;
				}
			}
		}

		if (strcmp(Recv_Kind, "plg") == 0)
		{
			Access_Name = 0;
			Input_User = -1;
			for (int i = 0; i < MAX_USER; i++)
			{
				if (strcmp(Recv_Name, User_List[i].Name) == 0)
				{
					Access_Name = 1;
					break;
				}
			}
			for (int i = 0; i < MAX_USER; i++)
			{
				if (strcmp("", User_List[i].Name) == 0)
				{
					Input_User = i;
					break;
				}
			}
			if (Access_Name == 1)
			{
				strcpy(Send_Kind, "alg");
			}
			else if (Input_User == -1)
			{
				strcpy(Send_Kind, "flg");
			}
			else
			{
				strcpy(Send_Kind, "slg");
				Access_User++;
				strcpy(User_List[Input_User].Name, Recv_Name);
				User_List[Input_User].Access = Access_User+CHANS;
				User_List[Input_User].Number = Input_User;
				User_List[Input_User].Room = -1;
				sprintf(Send_Numb, "%d", Input_User);
				Wait_Access++;
			}
		}
		else if (strcmp(Recv_Kind, "wtr") == 0)
		{
			User_List[atoi(Recv_Numb)].Access = Access_User+CHANS;// 접속 갱신

			G_Room_List_Front_int = atoi(G_Room_List_Front);
			for (int i = 0; i < 10; i++)
			{
				if (i + G_Room_List_Front_int < MAX_ROOM)
				{
					if (G_Room_List[i + G_Room_List_Front_int].Play == 0)
					{
						sprintf(Temp_Play, "대기");
					}
					else if (G_Room_List[i + G_Room_List_Front_int].Play == 1)
					{
						sprintf(Temp_Play, "게임");
					}
					sprintf(W_Room_List[i], "%2d번방 %d/6\t%s", i + G_Room_List_Front_int, G_Room_List[i + G_Room_List_Front_int].Access, Temp_Play);
				}
				else
					sprintf(W_Room_List[i], "%s", "");
			}

			if (Recv_Talk[0] == '/')
			{
				if (strcmp(Recv_Talk, "/n") == 0)
				{
					G_Room_List_Front_int += 10;
					sprintf(G_Room_List_Front, "%d", G_Room_List_Front_int);
				}
				if (strcmp(Recv_Talk, "/r") == 0)
				{
					G_Room_List_Front_int -= 10;
					if (G_Room_List_Front_int < 0)
						G_Room_List_Front_int = 0;
					sprintf(G_Room_List_Front, "%d", G_Room_List_Front_int);
				}
				if (Recv_Talk[1] >= 48 && Recv_Talk[1] <= 48 + 9)
				{
					sprintf(Send_Kind, "%c", Recv_Talk[1]);
					if (Recv_Talk[2] <= 48 && Recv_Talk[2] >= 48 + 9)
					{
						sprintf(Send_Kind, "%c%c", Recv_Talk[1], Recv_Talk[2]);
					}
					if (G_Room_List[atoi(Send_Kind)].Access < 6 && atoi(Send_Kind) < MAX_ROOM && G_Room_List[atoi(Send_Kind)].Play == 0)
					{
						Wait_Access--;
						G_Room_List[atoi(Send_Kind)].Access++;
						G_Room_List[atoi(Send_Kind)].Number = atoi(Send_Kind);
						for (int i = 0; i < MAX_USER; i++)
						{
							if (strcmp(Send_Name, User_List[i].Name) == 0)
							{
								for (int j = 0; j < 6; j++)
								{
									if (strcmp(G_Room_List[atoi(Send_Kind)].User[j], "") == 0)
									{
										strcpy(G_Room_List[atoi(Send_Kind)].User[j], Recv_Name);
										G_Room_List[atoi(Send_Kind)].User_Number[j] = i;
										User_List[atoi(Recv_Numb)].Room = atoi(Send_Kind);
										sprintf(G_Room_List[atoi(Send_Kind)].Massage, " ");
										break;
									}
								}
								break;
							}
						}
						
					}
					else
					{
						sprintf(Send_Kind, "wtr");
					}
				}
			}
		}
		else
		{
			User_List[atoi(Recv_Numb)].Access = Access_User+CHANS;// 접속 갱신

			for (int i = 0; i < 3; i++)
			{
				if((int)strlen(G_Room_List[atoi(Recv_Kind)].User[i]) < 8)
					sprintf(G_Room_Board[i][0], "%s\t\t", G_Room_List[atoi(Recv_Kind)].User[i]);
				else
					sprintf(G_Room_Board[i][0], "%s\t", G_Room_List[atoi(Recv_Kind)].User[i]);
			}
			for (int j = 1; j < 6; j++)
			{
				for (int i = 0; i < 3; i++)
				{
					for (int y = 0; y < 5; y++)
					{
						if (G_Room_List[atoi(Recv_Kind)].Board[i][y][j-1] == 0)
							sprintf(Temp_Buffer[y], "--");
						else if (strcmp(G_Room_List[atoi(Recv_Kind)].User[i], Recv_Name) == 0)
						{
							sprintf(Temp_Buffer[y], "%2d", G_Room_List[atoi(Recv_Kind)].Board[i][y][j-1]);
						}
						else
						{
							sprintf(Temp_Buffer[y], "[]");
						}
					}
					sprintf(G_Room_Board[i][j], " %s %s %s %s %s ", Temp_Buffer[0], Temp_Buffer[1], Temp_Buffer[2], Temp_Buffer[3], Temp_Buffer[4]);
				}
			}
			for (int i = 3; i < 6; i++)
			{
				if ((int)strlen(G_Room_List[atoi(Recv_Kind)].User[i]) < 8)
					sprintf(G_Room_Board[i][0], "%s\t\t", G_Room_List[atoi(Recv_Kind)].User[i]);
				else
					sprintf(G_Room_Board[i][0], "%s\t", G_Room_List[atoi(Recv_Kind)].User[i]);
			}
			for (int j = 1; j < 6; j++)
			{
				for (int i = 3; i < 6; i++)
				{
					for (int y = 0; y < 5; y++)
					{
						if (G_Room_List[atoi(Recv_Kind)].Board[i][y][j-1] == 0)
							sprintf(Temp_Buffer[y], "--");
						else if (strcmp(G_Room_List[atoi(Recv_Kind)].User[i], Recv_Name) == 0)
						{
							sprintf(Temp_Buffer[y], "%2d", G_Room_List[atoi(Recv_Kind)].Board[i][y][j-1]);
						}
						else
						{
							sprintf(Temp_Buffer[y], "[]");
						}
					}
					sprintf(G_Room_Board[i][j], " %s %s %s %s %s ", Temp_Buffer[0], Temp_Buffer[1], Temp_Buffer[2], Temp_Buffer[3], Temp_Buffer[4]);
				}
			}
			sprintf(G_Room_Massage, "%s", G_Room_List[atoi(Recv_Kind)].Massage);

			if (strcmp(Recv_Talk, "/out") == 0)
			{
				strcpy(Send_Kind, "wtr");
				Wait_Access++;
				G_Room_List[atoi(Recv_Kind)].Access--;
				for (int i = 0; i < 6; i++)
				{
					if (strcmp(G_Room_List[atoi(Recv_Kind)].User[i], Recv_Name) == 0)
					{
						strcpy(G_Room_List[atoi(Recv_Kind)].User[i], "");
						G_Room_List[atoi(Recv_Kind)].User_Number[i] = -1;
						User_List[atoi(Recv_Numb)].Room = -1;
						for (int x = 0; x < 5; x++)
						{
							for (int y = 0; y < 5; y++)
							{
								G_Room_List[atoi(Recv_Kind)].Board[i][x][y] = 0;
							}
						}
						break;
					}
				}
			}

			// 게임 시작 옵션
			if (strcmp(Recv_Talk, "/start") == 0 && strcmp(G_Room_List[atoi(Recv_Kind)].User[0], Recv_Name) == 0 && G_Room_List[atoi(Recv_Kind)].Play == 0)
			{
				G_Room_List[atoi(Recv_Kind)].Play = 1;
				G_Room_List[atoi(Recv_Kind)].Turn = rand() % G_Room_List[atoi(Recv_Kind)].Access;
				G_Room_List[atoi(Recv_Kind)].Max_Number = MAX_NUMBER;
				G_Room_List[atoi(Recv_Kind)].Max_Score = MAX_SCORE;
				for (int i = 0; i < 6; i++)
				{
					G_Room_List[atoi(Recv_Kind)].Score[i] = 0;
					if (G_Room_List[atoi(Recv_Kind)].User_Number[i] != -1)
					{
						for (int y = 0; y < 5; y++)
						{
							for (int x = 0; x < 5; x++)
							{
								G_Room_List[atoi(Recv_Kind)].Board[i][x][y] = rand() % G_Room_List[atoi(Recv_Kind)].Max_Number + 1;
							}
						}
					}
				}
				sprintf(G_Room_List[atoi(Recv_Kind)].Massage, "빙고게임이 시작되었습니다.");
			}

			// 게임이 진행중일 때
			if (G_Room_List[atoi(Recv_Kind)].Play == 1)
			{
				// 게임중 사용가능한 옵션 부분
				if (strcmp(G_Room_List[atoi(Recv_Kind)].User[G_Room_List[atoi(Recv_Kind)].Turn], Recv_Name) == 0 && Recv_Talk[0] == '/')
				{
					Recv_Talk[0] = ' ';

					for (int i = 0; i < 6; i++)
					{
						for (int x = 0; x < 5; x++)
						{
							for (int y = 0; y < 5; y++)
							{
								if (G_Room_List[atoi(Recv_Kind)].Board[i][x][y] == atoi(Recv_Talk))
								{
									G_Room_List[atoi(Recv_Kind)].Board[i][x][y] = 0;
								}
							}
						}
					}
					G_Room_List[atoi(Recv_Kind)].Turn++;
					if (G_Room_List[atoi(Recv_Kind)].Turn > 5)
					{
						G_Room_List[atoi(Recv_Kind)].Turn = 0;
					}
				}

				while (strcmp(G_Room_List[atoi(Recv_Kind)].User[G_Room_List[atoi(Recv_Kind)].Turn], "") == 0 || G_Room_List[atoi(Recv_Kind)].User_Number[G_Room_List[atoi(Recv_Kind)].Turn] == -1)
				{
					G_Room_List[atoi(Recv_Kind)].Turn++;
					if (G_Room_List[atoi(Recv_Kind)].Turn > 5)
					{
						G_Room_List[atoi(Recv_Kind)].Turn = 0;
					}
					if (G_Room_List[atoi(Recv_Kind)].Access == 0)
					{
						G_Room_List[atoi(Recv_Kind)].Play = 0;
						break;
					}
				}
				
				for (int i = 0; i < 6; i++)
				{
					G_Room_List[atoi(Recv_Kind)].Score[i] = 0;

					for (int y = 0; y < 5; y++)
					{
						Temp_Sum = 0;
						for (int x = 0; x < 5; x++)
						{
							Temp_Sum += G_Room_List[atoi(Recv_Kind)].Board[i][x][y];
						}
						if (Temp_Sum == 0)
							G_Room_List[atoi(Recv_Kind)].Score[i]++;
					}

					for (int x = 0; x < 5; x++)
					{
						Temp_Sum = 0;
						for (int y = 0; y < 5; y++)
						{
							Temp_Sum += G_Room_List[atoi(Recv_Kind)].Board[i][x][y];
						}
						if (Temp_Sum == 0)
							G_Room_List[atoi(Recv_Kind)].Score[i]++;
					}

					Temp_Sum = 0;
					for (int j = 0; j<5; j++)
					{
						Temp_Sum += G_Room_List[atoi(Recv_Kind)].Board[i][j][j];
					}
					if (Temp_Sum == 0)
						G_Room_List[atoi(Recv_Kind)].Score[i]++;

					Temp_Sum = 0;
					for (int j = 0; j<5; j++)
					{
						Temp_Sum += G_Room_List[atoi(Recv_Kind)].Board[i][4-j][j];
					}
					if (Temp_Sum == 0)
						G_Room_List[atoi(Recv_Kind)].Score[i]++;
				}

				Temp_Sum = 0;
				Temp_Win = -1;
				for (int i = 0; i < 6; i++)
				{
					if (strcmp(G_Room_List[atoi(Recv_Kind)].User[i], "") != 0)
					{
						if (G_Room_List[atoi(Recv_Kind)].Score[i] >= G_Room_List[atoi(Recv_Kind)].Max_Score)
						{
							Temp_Sum++;
							Temp_Win = i;
						}
					}
				}

				if (Temp_Sum == 1)
				{
					sprintf(G_Room_List[atoi(Recv_Kind)].Massage, "%s님의 승리!! 득점 %d개를 먼저 모으셨습니다!!", G_Room_List[atoi(Recv_Kind)].User[Temp_Win], G_Room_List[atoi(Recv_Kind)].Max_Score);
					G_Room_List[atoi(Recv_Kind)].Play = 0;
				}
				else if (Temp_Sum > 1)
				{
					sprintf(G_Room_List[atoi(Recv_Kind)].Massage, "2명 이상의 유저가 동시에 득점 %d개를 모으셨습니다!!", G_Room_List[atoi(Recv_Kind)].Max_Score);
					G_Room_List[atoi(Recv_Kind)].Play = 0;
				}
				else
				{
					sprintf(G_Room_List[atoi(Recv_Kind)].Massage, "%s님이 숫자를 선택할 차례입니다.", G_Room_List[atoi(Recv_Kind)].User[G_Room_List[atoi(Recv_Kind)].Turn]);
				}
			}
		}

		// 클라이언트 송신
		if(send(Client_Socket, Send_Kind, sizeof(Send_Kind), 0) == -1) printf("Send_Kind Fail\n");
		if(send(Client_Socket, Send_Name, sizeof(Send_Name), 0) == -1) printf("Send_Name Fail\n");
		if(send(Client_Socket, Send_Talk, sizeof(Send_Talk), 0) == -1) printf("Send_Talk Fail\n");
		if (send(Client_Socket, Send_Numb, sizeof(Send_Numb), 0) == -1) printf("Send_Numb Fail\n");
		for (int i = 0; i < 10; i++)
			if (send(Client_Socket, W_Room_List[i], sizeof(W_Room_List[i]), 0) == -1) printf("W_Room_List Fail\n");
		if (send(Client_Socket, G_Room_List_Front, sizeof(G_Room_List_Front), 0) == -1) printf("G_Room_List_Front Fail\n");
		for(int i=0; i<6; i++)
			for (int j = 0; j<6; j++)
				if (send(Client_Socket, G_Room_Board[i][j], sizeof(G_Room_Board[i][j]), 0) == -1) printf("G_Room_Board Fail\n");
		if (send(Client_Socket, G_Room_Massage, sizeof(G_Room_Massage), 0) == -1) printf("G_Room_Massage Fail\n");

		// 확인용 출력
		printf("Send_Kind : %s\n", Send_Kind);
		printf("Send_Name : %s\n", Send_Name);
		printf("Send_Talk : %s\n", Send_Talk);
		printf("Send_Numb : %s\n", Send_Numb);
		printf("Recv_Kind : %s\n", Recv_Kind);
		printf("Recv_Name : %s\n", Recv_Name);
		printf("Recv_Talk : %s\n", Recv_Talk);
		printf("Recv_Numb : %s\n", Recv_Numb);
		printf("User : %d  W_R : %d\n", Access_User, Wait_Access);

		// 변수 초기화
		strcpy( Send_Kind, "");
		strcpy( Send_Name, "");
		strcpy( Send_Talk, "");
		strcpy( Send_Numb, "");
		strcpy( Recv_Kind, "");
		strcpy( Recv_Name, "");
		strcpy( Recv_Talk, "");
		strcpy( Recv_Numb, "");

		// 클라이언트 소켓 삭제
		close(Client_Socket);

		// 접속 끊김 분별
		for (int i = 0; i < MAX_USER; i++)
		{
			User_List[i].Access--;
			if (User_List[i].Access < 0)
				User_List[i].Access = 0;
			if (User_List[i].Access == 1)
			{
				Access_User--;
				if (User_List[i].Room != -1)
				{
					for (int j = 0; j < 6; j++)
					{
						if (strcmp(G_Room_List[User_List[i].Room].User[j], User_List[i].Name) == 0)
						{
							G_Room_List[User_List[i].Room].Access--;
							Wait_Access++;
							strcpy(G_Room_List[User_List[i].Room].User[j], "");
							G_Room_List[User_List[i].Room].User_Number[j] = -1;
							for (int x = 0; x < 5; x++)
							{
								for (int y = 0; y < 5; y++)
								{
									G_Room_List[User_List[i].Room].Board[i][x][y] = 0;
								}
							}
							break;
						}
					}
				}
				Wait_Access--;
				strcpy(User_List[i].Name, "");
				User_List[i].Number = -1;
				User_List[i].Room = -1;
			}
		}
	}

	close(Server_Socket); // 서버 소켓 삭제
}
