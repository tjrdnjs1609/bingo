// 서버 소스코드

// 헤더파일 모음
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <pthread.h>
#include <stdio.h>

// 서버 설정
#define MAX_USER 50
#define MAX_ROOM 25
#define W_ROOM -1
#define LOGIN_P -2
#define LOGIN_A -3
#define LOGIN_F -4
#define CHANS 5
#define DEBUG 1

// 구조체
// 유저 
typedef struct user
{
	char Name[100]; // 유저 이름 
	int Number;// 유저 번호 
	int Kind;// 유저 위치
	char Input;// 유저 입력 
	int Time;// 유저 접속상태 
}User_t;
// 대기방 
typedef struct w_room
{
	int Point_X;// 가리키는 방향 X
	int Point_Y;// 가리키는 방향 Y
	int G_Room_Number[10];
	int G_Room_Access[10];
	int G_Room_Play[10];
	int G_Room_Size[10];
	int G_Room_Score[10];
}W_Room_t;
// 게임방
typedef struct g_room
{
	int Number;
	int Access;
	char User_Name[6][100];
	int User_Number[6];
	int Board[6][6][6];
	int Size;
	int Score;
	int User_Score[6];
	int Play;
	int User_Point[6][2];
	int Turn;
	int Win;
}G_Room_t;

// 전역 함수
int Server_Socket;
int Client_Socket;
int Client_Addr_Size;
struct sockaddr_in Server_Addr;
struct sockaddr_in Client_Addr;
int Option = 1;
int Working=0;
int Port = 9000;
char Key;
User_t User[MAX_USER];
G_Room_t G_Room[MAX_ROOM];
User_t Recv_User;
W_Room_t Recv_W_Room;
G_Room_t Recv_G_Room;
int Login_a, Login_f;
int S_Access = 0;
int Temp_Number;

// 프로토 타입
void* Server_Thread_Run();
void Login_Hendler();
void Create_Server();
void User_Clear(int a);
void G_Room_Clear(int a);
void G_Room_User_Clear(int a, int b);
void W_Room_Hendler();

// 메인 함수
int main(void)
{
	pthread_t Server_Thread; // 서버 쓰레드 
	int a; // for문 변수

	initscr(); // ncurses 시작

	// 유저리스트 초기화
	for(a=0; a<MAX_USER; a++)
	{
		User_Clear(a);
	}

	// 게임벙 초기화
	for(a=0; a<MAX_ROOM; a++)
	{
		G_Room_Clear(a);
	}

	pthread_create(&Server_Thread, NULL, Server_Thread_Run, NULL); // 서버 쓰레드 생성

	// 입력
	while(1)
	{
		Key = getch();
	}

	endwin(); // ncurses 종료 

	return 0;
}

// 서버 쓰레드
void* Server_Thread_Run()
{
	int a, b, c; // for문 전용

	//난수설정
	srand(time(NULL));

	// 서버 소켓 생성 
	Create_Server();

	// 클라이언트와 실시간 통신 
	while(1)
	{
		erase(); // 지우기

		// 클라이언트 생성
		Client_Addr_Size = sizeof(Client_Addr);
		if((Client_Socket = accept(Server_Socket, (struct sockaddr*)&Client_Addr, &Client_Addr_Size)) == -1)
		{
			endwin();
			printf("[Client_Socket : Accept() : Error.]\n");
			exit(1);
		}

		// 클라이언트의 데이터 수신
		if(recv(Client_Socket, &Recv_User, sizeof(User_t), 0) == -1)
		{
			endwin();
			printf("[Client_Socket : Recv() : Error.]\n");
			exit(1);
		}
		if(recv(Client_Socket, &Recv_W_Room, sizeof(W_Room_t), 0) == -1)
		{
			endwin();
			printf("[Client_Socket : Recv() : Error.]\n");
			exit(1);
		}

		// 데이터 변환 
		// 클라이언트가 로그인 요청 
		Login_Hendler();

		// 클라이언트가 대기방일 때 
		W_Room_Hendler();

		// 클라이언트가 게임방일 때
		if(Recv_User.Kind >= 0)
		{
			//접속상태 초기화 
			Recv_User.Time = S_Access*CHANS;
			User[Recv_User.Number] = Recv_User;

			//게임방 상태 전달
			Recv_G_Room = G_Room[Recv_User.Kind];

			//게임방 나가기
			if(Recv_User.Input == 'o')
			{
				for(a=0; a<6; a++)
				{
					if(G_Room[Recv_User.Kind].User_Number[a] == Recv_User.Number)
					{
						G_Room_User_Clear(Recv_User.Kind, a);
						G_Room[Recv_User.Kind].Access--;
						Recv_User.Kind = W_ROOM;
						User[Recv_User.Number] = Recv_User;
						break;
					}
				}
			}

			//반장용
			if(Recv_User.Number == G_Room[Recv_User.Kind].User_Number[0] && G_Room[Recv_User.Kind].Play == 0)
			{
				//빙고판 크기 변경
				if(Recv_User.Input == 'r')
				{
					G_Room[Recv_User.Kind].Size++;
					if(G_Room[Recv_User.Kind].Size > 6)
					{
						G_Room[Recv_User.Kind].Size = 4;
					}
				}
				//득점수 변경 
				if(Recv_User.Input == 'f')
				{
					G_Room[Recv_User.Kind].Score++;
					if(G_Room[Recv_User.Kind].Score > 6)
					{
						G_Room[Recv_User.Kind].Score = 1;
					}
				}
				//게임 시작하기
				if(Recv_User.Input == 'S')
				{
					G_Room[Recv_User.Kind].Play = 1;
					for(a=0; a<6; a++)
					{
						if(G_Room[Recv_User.Kind].User_Number[a] != -1)
						{
							for(b=0; b<G_Room[Recv_User.Kind].Size; b++)
							{
								for(c=0; c<G_Room[Recv_User.Kind].Size; c++)
								{
									G_Room[Recv_User.Kind].Board[a][b][c] = rand()%99+1;
								}
							}
						}
					}
					G_Room[Recv_User.Kind].Turn = rand()%6;
					while(G_Room[Recv_User.Kind].User_Number[G_Room[Recv_User.Kind].Turn] == -1)
					{
						G_Room[Recv_User.Kind].Turn++;
						if(G_Room[Recv_User.Kind].Turn > 5)
						{
							G_Room[Recv_User.Kind].Turn = 0;
						}
					}
				}
			}

			//게임중일때
			if(G_Room[Recv_User.Kind].Play == 1)
			{
				// 조작
				if(G_Room[Recv_User.Kind].User_Number[G_Room[Recv_User.Kind].Turn] == Recv_User.Number)
				{
					if(Recv_User.Input == 'w' && G_Room[Recv_User.Kind].User_Point[G_Room[Recv_User.Kind].Turn][1] != 0)
					{
						G_Room[Recv_User.Kind].User_Point[G_Room[Recv_User.Kind].Turn][1]--;
					}
					if(Recv_User.Input == 's' && G_Room[Recv_User.Kind].User_Point[G_Room[Recv_User.Kind].Turn][1] != G_Room[Recv_User.Kind].Size-1)
					{
						G_Room[Recv_User.Kind].User_Point[G_Room[Recv_User.Kind].Turn][1]++;
					}
					if(Recv_User.Input == 'a' && G_Room[Recv_User.Kind].User_Point[G_Room[Recv_User.Kind].Turn][0] != 0)
					{
						G_Room[Recv_User.Kind].User_Point[G_Room[Recv_User.Kind].Turn][0]--;
					}
					if(Recv_User.Input == 'd'&& G_Room[Recv_User.Kind].User_Point[G_Room[Recv_User.Kind].Turn][0] != G_Room[Recv_User.Kind].Size-1)
					{
						G_Room[Recv_User.Kind].User_Point[G_Room[Recv_User.Kind].Turn][0]++;
					}
					if(Recv_User.Input == '\n' && G_Room[Recv_User.Kind].Board[G_Room[Recv_User.Kind].Turn][G_Room[Recv_User.Kind].User_Point[G_Room[Recv_User.Kind].Turn][0]][G_Room[Recv_User.Kind].User_Point[G_Room[Recv_User.Kind].Turn][1]] != 0)
					{
						Temp_Number = G_Room[Recv_User.Kind].Board[G_Room[Recv_User.Kind].Turn][G_Room[Recv_User.Kind].User_Point[G_Room[Recv_User.Kind].Turn][0]][G_Room[Recv_User.Kind].User_Point[G_Room[Recv_User.Kind].Turn][1]];
						for(a=0; a<6; a++)
						{
							for(b=0; b<6; b++)
							{
								for(c=0; c<6; c++)
								{
									if(G_Room[Recv_User.Kind].Board[a][b][c] == Temp_Number)
									{
										G_Room[Recv_User.Kind].Board[a][b][c] = 0;
									}
								}
							}
						}
						G_Room[Recv_User.Kind].Turn++;
					}
				}

				//게임 상황
				while(G_Room[Recv_User.Kind].User_Number[G_Room[Recv_User.Kind].Turn] == -1)
				{
					G_Room[Recv_User.Kind].Turn++;
					if(G_Room[Recv_User.Kind].Turn > 5)
					{
						G_Room[Recv_User.Kind].Turn = 0;
					}
					if(G_Room[Recv_User.Kind].Access == 0)
					{
						break;
					}
				}
				for(a=0; a<6; a++)
				{
					G_Room[Recv_User.Kind].User_Score[a] = 0;
				}
				for(a=0; a<6; a++)
				{
					if(G_Room[Recv_User.Kind].User_Number[a] != -1)
					{
						for(b=0; b<G_Room[Recv_User.Kind].Size; b++)
						{
							Temp_Number = 0;
							for(c=0; c<G_Room[Recv_User.Kind].Size; c++)
							{
								Temp_Number += G_Room[Recv_User.Kind].Board[a][b][c];
							}
							if(Temp_Number == 0)
							{
								G_Room[Recv_User.Kind].User_Score[a]++;
							}
						}
						for(b=0; b<G_Room[Recv_User.Kind].Size; b++)
						{
							Temp_Number = 0;
							for(c=0; c<G_Room[Recv_User.Kind].Size; c++)
							{
								Temp_Number += G_Room[Recv_User.Kind].Board[a][c][b];
							}
							if(Temp_Number == 0)
							{
								G_Room[Recv_User.Kind].User_Score[a]++;
							}
						}
						Temp_Number = 0;
						for(c=0; c<G_Room[Recv_User.Kind].Size; c++)
						{
							Temp_Number += G_Room[Recv_User.Kind].Board[a][c][c];
						}
						if(Temp_Number == 0)
						{
							G_Room[Recv_User.Kind].User_Score[a]++;
						}
						Temp_Number = 0;
						for(c=0; c<G_Room[Recv_User.Kind].Size; c++)
						{
							Temp_Number += G_Room[Recv_User.Kind].Board[a][G_Room[Recv_User.Kind].Size-c-1][c];
						}
						if(Temp_Number == 0)
						{
							G_Room[Recv_User.Kind].User_Score[a]++;
						}
					}
				}
				//득점 확인
				Temp_Number = 0;
				for(a=0; a<6; a++)
				{
					if(G_Room[Recv_User.Kind].User_Score[a] >= G_Room[Recv_User.Kind].Score)
					{
						Temp_Number++;
						G_Room[Recv_User.Kind].Win = a;
					}
				}
				if(Temp_Number == 1)
				{
					G_Room[Recv_User.Kind].Play = 0;
				}
				else if(Temp_Number > 1)
				{
					G_Room[Recv_User.Kind].Win = 6;
					G_Room[Recv_User.Kind].Play = 0;
				}
				else
				{
					G_Room[Recv_User.Kind].Win = -1;
				}

				Recv_G_Room = G_Room[Recv_User.Kind];
			}
		}

		// 클라이언트에게 데이터 송신
		if(send(Client_Socket, &Recv_User, sizeof(User_t), 0) == -1)
		{
			endwin();
			printf("[Client_Socket : Send() : Error.]\n");
			exit(1);
		}
		if(send(Client_Socket, &Recv_W_Room, sizeof(W_Room_t), 0) == -1)
		{
			endwin();
			printf("[Client_Socket : Send() : Error.]\n");
			exit(1);
		}
		if(send(Client_Socket, &Recv_G_Room, sizeof(G_Room_t), 0) == -1)
		{
			endwin();
			printf("[Client_Socket : Send() : Error.]\n");
			exit(1);
		}

		// 접속자 선별 
		for(a=0; a<MAX_USER; a++)
		{
			User[a].Time--;
			if(User[a].Time == 0)
			{
				S_Access--;
				if(User[a].Kind != -1)
				{
					for(b=0; b<6; b++)
					{
						if(G_Room[User[a].Kind].User_Number[b] == User[a].Number)
						{
							G_Room_User_Clear(User[a].Kind, b);
							G_Room[User[a].Kind].Access--;
							break;
						}
					}
				}
				User_Clear(a);
			}
			if(User[a].Time < 0)
			{
				User[a].Time = 0;
			}
		}

		// 접속자 0인 게임방 초기화
		for(a=0; a<MAX_ROOM; a++)
		{
			if(G_Room[a].Access == 0)
			{
				 G_Room_Clear(a);
				break;
			}
		}

		// 서버 화면 출력
		
		// 서버 작동중 
		if(Working == 0)
		{
			Working = 1;
			mvprintw(0, 0, "[Working... |]");
		}
		else if(Working == 1)
		{
			Working = 2;
			mvprintw(0, 0, "[Working... /]");
		}
		else if(Working == 2)
		{
			Working = 3;
			mvprintw(0, 0, "[Working... -]");
		}
		else if(Working == 3)
		{
			Working = 0;
			mvprintw(0, 0, "[Working... \\]");
		}

		// 서버 상태 확인용 출력
		if(DEBUG == 1)
		{
			mvprintw(1, 0, "Recv_User_Name : %s", Recv_User.Name);
			mvprintw(2, 0, "Recv_User_Number : %d", Recv_User.Number);
			mvprintw(3, 0, "Recv_User_Kind : %d", Recv_User.Kind);
			mvprintw(4, 0, "Recv_User_Input : %c", Recv_User.Input);
			mvprintw(5, 0, "Login_a : %d Login_f : %d", Login_a, Login_f);
			mvprintw(6, 0, "S_Access : %d", S_Access);
			mvprintw(7, 0, "User : ");
			for(a=0; a<MAX_USER; a++)
				mvprintw(7, 10+a*3, "[%2d]", User[a].Time);
			mvprintw(8, 0, "G_Room[0]_Access : %d", G_Room[0].Access);
			mvprintw(9, 0, "G_Room[0]_User_Number : ");
			for(a=0; a<6; a++)
			{
				mvprintw(9, 25+a*3, "[%2d]", G_Room[0].User_Number[a]);
			}
			mvprintw(10, 0, "G_Room[0]_Play : %d", G_Room[0].Play);
			mvprintw(11, 0, "G_Room[0]_Win : %d", G_Room[0].Win);
			mvprintw(12, 0, "G_Room[0]_Turn : %d", G_Room[0].Turn);
			mvprintw(13, 0, "Temp_Number : %d", Temp_Number);
		}

		// 서버 이름 
		mvprintw(23, 0, "Server");

		refresh();

		// 클라이언트 삭제
		close(Client_Socket);
	}
}

// 로그인 핸들러
void Login_Hendler()
{
	int a;

	if(Recv_User.Kind == LOGIN_P)
	{
		// 공간이 있는지 확인 
		Login_f = -1;
		for(a=0; a<MAX_USER; a++)
		{
			if(strcmp(User[a].Name, "") == 0)
			{
				Login_f = a;
				break;
			}
		}
		// 이미 접속중인지 확인 
		Login_a = 0;
		for(a=0; a<MAX_USER; a++)
		{
			if(strcmp(Recv_User.Name, User[a].Name) == 0)
			{
				Login_a = 1;
				break;
			}
		}
		if(Login_a == 1)
		{
			// 이미 접속중 
			Recv_User.Kind = LOGIN_A;
		}
		else if(Login_f == -1)
		{
			// 서버가 가득참 
			Recv_User.Kind = LOGIN_F;
		}
		else
		{
			// 로그인 성공 : 대기방으로 이동 
			S_Access++;
			Recv_User.Kind = W_ROOM;
			Recv_User.Number = Login_f;
			Recv_User.Time = S_Access*CHANS;
			User[Login_f] = Recv_User;
		}
	}
}

// 서버 생성 
void Create_Server()
{
	// 서버 소켓 생성 
	if((Server_Socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		endwin();
		printf("[System] : Server_Socket : socket() : Error.\n");
		exit(1);
	}

	// 바인드 오류 방지용 
	if(setsockopt(Server_Socket, SOL_SOCKET, SO_REUSEADDR, &Option, sizeof(Option)) == -1)
	{
		endwin();
		printf("[System] : Setsockopt() : Error.\n");
		exit(1);
	}

	// 서버 설정 
	memset(&Server_Addr, 0, sizeof(Server_Addr));
	Server_Addr.sin_family = AF_INET;
	Server_Addr.sin_port = htons(Port);
	Server_Addr.sin_addr.s_addr = htonl(INADDR_ANY);

	// 서버 bind
	if(bind(Server_Socket, (struct sockaddr*)&Server_Addr, sizeof(Server_Addr)) == -1)
	{
		endwin();
		printf("[System] : Bind() : Error.\n");
		exit(1);
	}

	// 서버 연결 on
	if(listen(Server_Socket, 5) == -1)
	{
		endwin();
		printf("[Listen() : Error.]\n");
		exit(1);
	}
}

// 유저 초기화
void User_Clear(int a)
{
	strcpy(User[a].Name, "");
	User[a].Number = 0;
	User[a].Kind = 0;
	User[a].Input = ' ';
	User[a].Time = 0;
}

// 게임방 초기화
void G_Room_Clear(int a)
{
	int b, c, d;
	G_Room[a].Number = a;
	G_Room[a].Access = 0;
	G_Room[a].Size = 5;
	G_Room[a].Score = 3;
	G_Room[a].Play = 0;
	G_Room[a].Turn = 0;
	G_Room[a].Win = -1;
	for(b=0; b<6; b++)
	{
		G_Room_User_Clear(a, b);
	}
}

//게임방 유저 초기화
void G_Room_User_Clear(int a, int b)
{
	int c, d;
	strcpy(G_Room[a].User_Name[b], "");
	G_Room[a].User_Number[b] = -1;
	G_Room[a].User_Score[b] = 0;
	G_Room[a].User_Point[b][0] = 0;
	G_Room[a].User_Point[b][1] = 0;
	for(c=0; c<6; c++)
	{
		for(d=0; d<6; d++)
		{
			G_Room[a].Board[b][c][d] = 0;
		}
	}
}

//대기방 핸들러
void W_Room_Hendler()
{
	int a;
	if(Recv_User.Kind == W_ROOM)
	{
		//접속상태 초기화 
		Recv_User.Time = S_Access*CHANS;
		User[Recv_User.Number] = Recv_User;
	
		// 대기방 게임방 리스트 설정 
		for(a=0; a<10; a++)
		{
			if(a+Recv_W_Room.Point_X*10 >= MAX_ROOM)
			{
				Recv_W_Room.G_Room_Number[a] = -1;
				Recv_W_Room.G_Room_Access[a] = -1;
				Recv_W_Room.G_Room_Play[a] = -1;
				Recv_W_Room.G_Room_Size[a] = -1;
				Recv_W_Room.G_Room_Score[a] = -1;
			}
			else
			{
				Recv_W_Room.G_Room_Number[a] = a+Recv_W_Room.Point_X*10;
				Recv_W_Room.G_Room_Access[a] = G_Room[a+Recv_W_Room.Point_X*10].Access;
				Recv_W_Room.G_Room_Play[a] = G_Room[a+Recv_W_Room.Point_X*10].Play;
				Recv_W_Room.G_Room_Size[a] = G_Room[a+Recv_W_Room.Point_X*10].Size;
				Recv_W_Room.G_Room_Score[a] = G_Room[a+Recv_W_Room.Point_X*10].Score;
			}
		}

		// 대기방에서의 입력 설정 
		if(Recv_User.Input == 'w' && Recv_W_Room.Point_Y != 0)
		{
			Recv_W_Room.Point_Y--;
		}
		if(Recv_User.Input == 's' && Recv_W_Room.Point_Y != 9)
		{
			Recv_W_Room.Point_Y++;
		}
		if(Recv_User.Input == 'a' && Recv_W_Room.Point_X != 0)
		{
			Recv_W_Room.Point_X--;
		}
		if(Recv_User.Input == 'd')
		{
			Recv_W_Room.Point_X++;
		}
		if(Recv_User.Input == '\n')
		{
			if(G_Room[Recv_W_Room.Point_Y+Recv_W_Room.Point_X*10].Access != 6 && G_Room[Recv_W_Room.Point_Y+Recv_W_Room.Point_X*10].Play == 0 && Recv_W_Room.Point_Y+Recv_W_Room.Point_X*10 < MAX_ROOM)
			{
				Recv_User.Kind = Recv_W_Room.Point_Y+Recv_W_Room.Point_X*10;
				User[Recv_User.Number] = Recv_User;
				for(a=0; a<6; a++)
				{
					if(G_Room[Recv_W_Room.Point_Y+Recv_W_Room.Point_X*10].User_Number[a] == -1)
					{
						strcpy(G_Room[Recv_W_Room.Point_Y+Recv_W_Room.Point_X*10].User_Name[a], Recv_User.Name);
						G_Room[Recv_W_Room.Point_Y+Recv_W_Room.Point_X*10].User_Number[a] = Recv_User.Number;
						G_Room[Recv_W_Room.Point_Y+Recv_W_Room.Point_X*10].Access++;
						G_Room[Recv_W_Room.Point_Y+Recv_W_Room.Point_X*10].Win = -1;
						break;
					}
				}
			}
		}
	}
}
