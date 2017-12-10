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
#define MAX_USER 50//최대 유저수 
#define MAX_ROOM 25//최대 게임방 수 
#define W_ROOM -1//대기방 번호 
#define LOGIN_P -2//로그인 요청 번호 
#define LOGIN_A -3//중복오류 번호 
#define LOGIN_F -4//서버가득참 번호 
#define CHANS 5//연결끊김 지연시간 
#define PORT 9810//포트 

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
	int G_Room_Number[10];// 게임방 번호 
	int G_Room_Access[10];// 게임방 유저수 
	int G_Room_Play[10];// 게임방 상태 
	int G_Room_Size[10];// 게임방 보드판 크기 
	int G_Room_Score[10];// 게임방 득점수 
}W_Room_t;
// 게임방
typedef struct g_room
{
	int Number;// 게임방 번호 
	int Access;//게임방 유저수 
	char User_Name[6][100];//유저 이름 
	int User_Number[6];//유저 번호 
	int Board[6][6][6];//보드판 
	int Size;//크기설정 
	int Score;//득점설정 
	int User_Score[6];//유저 득점 
	int Play;//게임상태 
	int User_Point[6][2];//유저 커서 
	int Turn;//턴 
	int Win;//게임 결과 
	int User_Item[6][3];//유저 아이템 
	int J;//점프 
	int Q;//역행 
	int K;//한번더 
	int GetItem;//아이템 얻음 
}G_Room_t;

// 전역 함수
int Server_Socket;
int Client_Socket;
int Client_Addr_Size;
struct sockaddr_in Server_Addr;
struct sockaddr_in Client_Addr;
int Option = 1;
int Working=0;
int Port = PORT;
char Key;
int Manager_Point = 0;
int Manager_User = 0;
int Manager_G_Room = 0;
User_t User[MAX_USER];
G_Room_t G_Room[MAX_ROOM];
User_t Recv_User;
W_Room_t Recv_W_Room;
G_Room_t Recv_G_Room;
int Login_a, Login_f;
int S_Access = 0;
int Temp_Number;
int Temp_Number2 = 0;

// 프로토 타입
void* Server_Thread_Run();
void Login_Hendler();
void Create_Server();
void User_Clear(int a);
void G_Room_Clear(int a);
void G_Room_User_Clear(int a, int b);
void W_Room_Hendler();
void Manager_Input();
void Main_Drow();
void Create_Client();
void Recv_Client();
void G_Room_Handler();
void G_Room_Out();
void Send_Client();
void Search_User_Connect();
void Server_Drow();
void Search_G_Room_Connect();
void G_Room_Owner();
void G_Room_Game_Handler();

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

	// 게임방 초기화
	for(a=0; a<MAX_ROOM; a++)
	{
		G_Room_Clear(a);
	}

	pthread_create(&Server_Thread, NULL, Server_Thread_Run, NULL); // 서버 쓰레드 생성

	// 관리자 입력
	Manager_Input();

	endwin(); // ncurses 종료 

	return 0;
}

// 서버 쓰레드
void* Server_Thread_Run()
{
	//난수설정
	srand(time(NULL));

	// 서버 소켓 생성 
	Create_Server();

	// 클라이언트와 실시간 통신 
	while(1)
	{
		erase(); // 지우기

		Temp_Number2 = 2; // 클라이언트가 0개일때를 위한 변수변환

		// 클라이언트 생성
		Create_Client();

		// 클라이언트의 데이터 수신
		Recv_Client();

		// 데이터 변환 
		// 클라이언트가 로그인 요청 
		Login_Hendler();
		// 클라이언트가 대기방일 때 
		W_Room_Hendler();
		// 클라이언트가 게임방일 때
		G_Room_Handler();
		
		// 클라이언트에게 데이터 송신
		Send_Client();

		// 접속자 선별
		Search_User_Connect();

		// 접속자 0인 게임방 초기화
		Search_G_Room_Connect();

		// 서버 화면 출력
		Server_Drow();

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
	User[a].Number = -1;
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
	G_Room[a].J = 0;
	G_Room[a].Q = 1;
	G_Room[a].K = 0;
	G_Room[a].GetItem = 0;
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
	G_Room[a].User_Item[b][0] = 0;
	G_Room[a].User_Item[b][1] = 0;
	G_Room[a].User_Item[b][2] = 0;
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
		//위 
		if(Recv_User.Input == 'w' && Recv_W_Room.Point_Y != 0)
		{
			Recv_W_Room.Point_Y--;
		}
		//아래 
		if(Recv_User.Input == 's' && Recv_W_Room.Point_Y != 9)
		{
			Recv_W_Room.Point_Y++;
		}
		//왼쪽 
		if(Recv_User.Input == 'a' && Recv_W_Room.Point_X != 0)
		{
			Recv_W_Room.Point_X--;
		}
		//오른쪽 
		if(Recv_User.Input == 'd')
		{
			Recv_W_Room.Point_X++;
		}
		//방 들어감 
		if(Recv_User.Input == '\n')
		{
			// 방이 6명이 아니고 게임중이 아니면 입장 
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

// 메니저 입력 
void Manager_Input()
{
	int a;

	while (1)// 유저입력 반복
	{
		if (Temp_Number2 == 0)// 클라이언트 접속이 0일 때 출력
		{
			// 테두리
			Main_Drow();

			for (a = 1; a < 79; a++)
				mvprintw(11, a, " ");
			mvprintw(11, 30, "Access Client is Zero.");
		}

		//유저 입력 
		Key = getch();

		if (Key == 'a')
		{
			Manager_Point = 0;//유저 리스트 보기 
		}
		if (Key == 'd')
		{
			Manager_Point = 1;//게임방 리스트 보기 
		}
		if (Manager_Point == 0 && Key == 'w' && Manager_User > 0)
		{
			Manager_User--;//유저리스트 위로
		}
		if (Manager_Point == 0 && Key == 's')
		{
			Manager_User++;//유저리스트 아래로
		}
		if (Manager_Point == 1 && Key == 'w' && Manager_G_Room > 0)
		{
			Manager_G_Room--;//게임방리스트 위로
		}
		if (Manager_Point == 1 && Key == 's')
		{
			Manager_G_Room++;//게임방리스트 아래로
		}
		Temp_Number2--;
		if (Temp_Number2 < 0)//클라이언트 접속이 0일때를 위한 변수변환 
			Temp_Number2 = 0;
	}
}

// 테두리
void Main_Drow()
{
	int a;

	for (a = 0; a < 80; a++)
	{
		mvprintw(0, a, "-");
		mvprintw(23, a, "-");
	}
	for (a = 1; a < 23; a++)
	{
		mvprintw(a, 0, "|");
		mvprintw(a, 79, "|");
	}
}

// 클라이언트 생성
void Create_Client()
{
	Client_Addr_Size = sizeof(Client_Addr);
	if ((Client_Socket = accept(Server_Socket, (struct sockaddr*)&Client_Addr, &Client_Addr_Size)) == -1)
	{
		endwin();
		printf("[Client_Socket : Accept() : Error.]\n");
		exit(1);
	}
}

// 클라이언트의 데이터 수신
void Recv_Client()
{
	if (recv(Client_Socket, &Recv_User, sizeof(User_t), 0) == -1)
	{
		endwin();
		printf("[Client_Socket : Recv() : Error.]\n");
		exit(1);
	}
	if (recv(Client_Socket, &Recv_W_Room, sizeof(W_Room_t), 0) == -1)
	{
		endwin();
		printf("[Client_Socket : Recv() : Error.]\n");
		exit(1);
	}
}

// 클라이언트가 게임방일 때
void G_Room_Handler()
{
	if (Recv_User.Kind >= 0)
	{
		//접속상태 초기화 
		Recv_User.Time = S_Access*CHANS;
		User[Recv_User.Number] = Recv_User;

		//게임방 상태 전달
		Recv_G_Room = G_Room[Recv_User.Kind];

		//게임방 나가기
		G_Room_Out();

		//반장용
		G_Room_Owner();

		//게임중일때
		G_Room_Game_Handler();
	}
}

//게임방 나가기
void G_Room_Out()
{
	int a;

	if (Recv_User.Input == 'o')
	{
		for (a = 0; a < 6; a++)
		{
			if (G_Room[Recv_User.Kind].User_Number[a] == Recv_User.Number)
			{
				G_Room_User_Clear(Recv_User.Kind, a);
				G_Room[Recv_User.Kind].Access--;
				Recv_User.Kind = W_ROOM;
				User[Recv_User.Number] = Recv_User;
				break;
			}
		}
	}
}

// 클라이언트에게 데이터 송신
void Send_Client()
{
	if (send(Client_Socket, &Recv_User, sizeof(User_t), 0) == -1)
	{
		endwin();
		printf("[Client_Socket : Send() : Error.]\n");
		exit(1);
	}
	if (send(Client_Socket, &Recv_W_Room, sizeof(W_Room_t), 0) == -1)
	{
		endwin();
		printf("[Client_Socket : Send() : Error.]\n");
		exit(1);
	}
	if (send(Client_Socket, &Recv_G_Room, sizeof(G_Room_t), 0) == -1)
	{
		endwin();
		printf("[Client_Socket : Send() : Error.]\n");
		exit(1);
	}
}

// 접속자 선별
void Search_User_Connect()
{
	int a, b;
	for (a = 0; a < MAX_USER; a++)
	{
		User[a].Time--;// 모든 접속시간 -1 
		if (User[a].Time == 0)// 접속시간이 0이 됐다면 연결끊음 
		{
			S_Access--;
			if (User[a].Kind != -1)// 유저가 게임방에 접속중이였다면 게임방에서도 끊음 
			{
				for (b = 0; b < 6; b++)
				{
					if (G_Room[User[a].Kind].User_Number[b] == User[a].Number)
					{
						G_Room_User_Clear(User[a].Kind, b);
						G_Room[User[a].Kind].Access--;
						break;
					}
				}
			}
			User_Clear(a);
		}
		if (User[a].Time < 0)// 접속시간이 음수이면 0으로 초기화 
		{
			User[a].Time = 0;
		}
	}
}

// 서버 화면 출력
void Server_Drow()
{
	int a;

	// 테두리
	Main_Drow();
	for (a = 1; a < 79; a++)
	{
		mvprintw(3, a, "-");
	}
	for (a = 4; a < 23; a++)
	{
		mvprintw(a, 39, "|");
	}
	for (a = 1; a < 39; a++)
	{
		mvprintw(10, a, "-");
		mvprintw(18, a, "-");
	}
	// 관리자 커서
	if (Manager_Point == 1)
	{
		mvprintw(4, 39, ">");
		mvprintw(6, 39, ">");
		mvprintw(8, 39, ">");
	}
	else
	{
		mvprintw(4, 39, "<");
		mvprintw(6, 39, "<");
		mvprintw(8, 39, "<");
	}
	// 수신 정보 
	mvprintw(1, 1, "Recv User Name : %s", Recv_User.Name);
	mvprintw(1, 39, "Recv User Number : %d", Recv_User.Number);
	mvprintw(2, 1, "Recv User Kind : %d", Recv_User.Kind);
	mvprintw(2, 39, "Recv User Input  : %c", Recv_User.Input);

	// 접속자 목록
	mvprintw(3, 0, "|[N][Access Name]------[N]-[K]-[I]");
	for (a = 0; a < 5; a++)
	{
		if (a + 5 * Manager_User > MAX_USER - 1)
		{
			mvprintw(4 + a, 1, " ");
		}
		else if (User[a + 5 * Manager_User].Number == -1)
		{
			mvprintw(4 + a, 1, "%2d.", a + 5 * Manager_User);
			mvprintw(4 + a, 4, "Blank");
		}
		else
		{
			mvprintw(4 + a, 1, "%2d.", a + 5 * Manager_User);
			mvprintw(4 + a, 4, "%s", User[a + 5 * Manager_User].Name);
			mvprintw(4 + a, 24, "%d", User[a + 5 * Manager_User].Number);
			mvprintw(4 + a, 28, "%d", User[a + 5 * Manager_User].Kind);
			mvprintw(4 + a, 32, "%c", User[a + 5 * Manager_User].Input);
		}
	}

	//서버 정보
	mvprintw(10, 1, "[Server]");
	mvprintw(11, 1, " Access User : %d", S_Access);
	mvprintw(12, 1, " Port        : %d", PORT);
	mvprintw(13, 1, " Max User    : %d", MAX_USER);
	mvprintw(14, 1, " Max G Room  : %d", MAX_ROOM);
	mvprintw(15, 1, " Manager     : P:%d U:%d G:%d", Manager_Point, Manager_User, Manager_G_Room);

	//접속중인 유저 대기방 정보
	mvprintw(18, 1, "[Wait Room of Access User]");
	mvprintw(19, 1, " Point G Room : %d", Recv_W_Room.Point_Y + Recv_W_Room.Point_X * 10);
	mvprintw(20, 1, " Point X      : %d", Recv_W_Room.Point_X);
	mvprintw(21, 1, " Point Y      : %d", Recv_W_Room.Point_Y);

	// 게임방 정보
	if (Manager_G_Room < MAX_ROOM)
	{
		mvprintw(3, 40, "[Game Room]");
		mvprintw(4, 40, " Game Room Number : %d", G_Room[Manager_G_Room].Number);
		for (a = 0; a < 6; a++)
		{
			mvprintw(5 + a, 40, " %d.User : %2d.%s S:%d I:%d%d%d", a, G_Room[Manager_G_Room].User_Number[a], G_Room[Manager_G_Room].User_Name[a], G_Room[Manager_G_Room].User_Score[a], G_Room[Manager_G_Room].User_Item[a][0], G_Room[Manager_G_Room].User_Item[a][1], G_Room[Manager_G_Room].User_Item[a][2]);
		}
		mvprintw(11, 40, " Game Play  : %d", G_Room[Manager_G_Room].Play);
		mvprintw(12, 40, " Board Size : %d*%d", G_Room[Manager_G_Room].Size, G_Room[Manager_G_Room].Size);
		mvprintw(13, 40, " Max_Socre  : %d", G_Room[Manager_G_Room].Score);
		mvprintw(14, 40, " Game Turn  : %d", G_Room[Manager_G_Room].Turn);
		mvprintw(15, 40, " Game Win   : %d", G_Room[Manager_G_Room].Win);
		mvprintw(16, 40, " Get Item   : %d", G_Room[Manager_G_Room].GetItem);
		mvprintw(17, 40, " Item J     : %d", G_Room[Manager_G_Room].J);
		mvprintw(18, 40, " Item Q     : %d", G_Room[Manager_G_Room].Q);
		mvprintw(19, 40, " Item K     : %d", G_Room[Manager_G_Room].K);
	}

	else
	{
		mvprintw(4, 40, "Not Created Game Room : %d", G_Room[Manager_G_Room].Number);
	}

	// 서버 작동중 

	if (Working < 0)
	{
		Working = S_Access * 4;
	}
	if (Working > S_Access * 3)
	{
		mvprintw(0, 0, "[Working... |]");
	}
	else if (Working > S_Access * 2)
	{
		mvprintw(0, 0, "[Working... /]");
	}
	else if (Working > S_Access * 1)
	{
		mvprintw(0, 0, "[Working... -]");
	}
	else
	{
		mvprintw(0, 0, "[Working... \\]");
	}
	Working--;

	// 서버 이름 
	mvprintw(23, 72, "[Server]");
}

// 접속자 0인 게임방 초기화
void Search_G_Room_Connect()
{
	int a;
	for (a = 0; a < MAX_ROOM; a++)
	{
		if (G_Room[a].Access == 0)
		{
			G_Room_Clear(a);
		}
	}
}

//반장용
void G_Room_Owner()
{
	int a, b, c;
	if (Recv_User.Number == G_Room[Recv_User.Kind].User_Number[0] && G_Room[Recv_User.Kind].Play == 0)
	{
		//빙고판 크기 변경
		if (Recv_User.Input == 'r')
		{
			G_Room[Recv_User.Kind].Size++;
			if (G_Room[Recv_User.Kind].Size > 6)
			{
				G_Room[Recv_User.Kind].Size = 4;
			}
		}
		//득점수 변경 
		if (Recv_User.Input == 'f')
		{
			G_Room[Recv_User.Kind].Score++;
			if (G_Room[Recv_User.Kind].Score > 6)
			{
				G_Room[Recv_User.Kind].Score = 1;
			}
		}
		//게임 시작하기
		if (Recv_User.Input == 'S')
		{
			// 게임중 표시 
			G_Room[Recv_User.Kind].Play = 1;
			//모든 접속자 보드판에 난수 입력 
			for (a = 0; a < 6; a++)
			{
				if (G_Room[Recv_User.Kind].User_Number[a] != -1)
				{
					for (b = 0; b < G_Room[Recv_User.Kind].Size; b++)
					{
						for (c = 0; c < G_Room[Recv_User.Kind].Size; c++)
						{
							G_Room[Recv_User.Kind].Board[a][b][c] = rand() % 99 + 1;
						}
					}
				}
			}
			// 첫번째 턴 랜덤 설정 
			G_Room[Recv_User.Kind].Turn = rand() % 6;
			// 아이템 상태 초기화 
			G_Room[Recv_User.Kind].J = 0;
			G_Room[Recv_User.Kind].Q = 1;
			G_Room[Recv_User.Kind].K = 0;
			// 모든 유저 아이템 상태 초기화 
			for (a = 0; a < 6; a++)
			{
				for (b = 0; b < 3; b++)
				{
					G_Room[Recv_User.Kind].User_Item[a][b] = 0;
				}
			}
			// 접속된 유저가 턴을 받도록 수정 
			while (G_Room[Recv_User.Kind].User_Number[G_Room[Recv_User.Kind].Turn] == -1)
			{
				G_Room[Recv_User.Kind].Turn++;
				if (G_Room[Recv_User.Kind].Turn > 5)
				{
					G_Room[Recv_User.Kind].Turn = 0;
				}
			}
		}
	}
}

//게임중일때
void G_Room_Game_Handler()
{
	int a, b, c;
	if (G_Room[Recv_User.Kind].Play == 1)
	{
		// 조작 입력 
		// 지금 턴이 입력자일때 
		if (G_Room[Recv_User.Kind].User_Number[G_Room[Recv_User.Kind].Turn] == Recv_User.Number)
		{
			//위 
			if (Recv_User.Input == 'w' && G_Room[Recv_User.Kind].User_Point[G_Room[Recv_User.Kind].Turn][1] != 0)
			{
				G_Room[Recv_User.Kind].User_Point[G_Room[Recv_User.Kind].Turn][1]--;
			}
			//아래 
			if (Recv_User.Input == 's' && G_Room[Recv_User.Kind].User_Point[G_Room[Recv_User.Kind].Turn][1] != G_Room[Recv_User.Kind].Size - 1)
			{
				G_Room[Recv_User.Kind].User_Point[G_Room[Recv_User.Kind].Turn][1]++;
			}
			//왼쪽 
			if (Recv_User.Input == 'a' && G_Room[Recv_User.Kind].User_Point[G_Room[Recv_User.Kind].Turn][0] != 0)
			{
				G_Room[Recv_User.Kind].User_Point[G_Room[Recv_User.Kind].Turn][0]--;
			}
			//오른쪽 
			if (Recv_User.Input == 'd'&& G_Room[Recv_User.Kind].User_Point[G_Room[Recv_User.Kind].Turn][0] != G_Room[Recv_User.Kind].Size - 1)
			{
				G_Room[Recv_User.Kind].User_Point[G_Room[Recv_User.Kind].Turn][0]++;
			}
			//아이템 1 
			if (Recv_User.Input == '1')
			{
				switch (G_Room[Recv_User.Kind].User_Item[G_Room[Recv_User.Kind].Turn][0])
				{
				case 1:
					G_Room[Recv_User.Kind].J++;
					break;
				case 2:
					G_Room[Recv_User.Kind].Q *= -1;
					break;
				case 3:
					G_Room[Recv_User.Kind].K++;
					break;
				}
				G_Room[Recv_User.Kind].User_Item[G_Room[Recv_User.Kind].Turn][0] = 0;
			}
			//아이템 2 
			if (Recv_User.Input == '2')
			{
				switch (G_Room[Recv_User.Kind].User_Item[G_Room[Recv_User.Kind].Turn][1])
				{
				case 1:
					G_Room[Recv_User.Kind].J++;
					break;
				case 2:
					G_Room[Recv_User.Kind].Q *= -1;
					break;
				case 3:
					G_Room[Recv_User.Kind].K++;
					break;
				}
				G_Room[Recv_User.Kind].User_Item[G_Room[Recv_User.Kind].Turn][1] = 0;
			}
			//아이템 3 
			if (Recv_User.Input == '3')
			{
				switch (G_Room[Recv_User.Kind].User_Item[G_Room[Recv_User.Kind].Turn][2])
				{
				case 1:
					G_Room[Recv_User.Kind].J++;
					break;
				case 2:
					G_Room[Recv_User.Kind].Q *= -1;
					break;
				case 3:
					G_Room[Recv_User.Kind].K++;
					break;
				}
				G_Room[Recv_User.Kind].User_Item[G_Room[Recv_User.Kind].Turn][2] = 0;
			}
			// 지우기 입력 
			if (Recv_User.Input == '\n' && G_Room[Recv_User.Kind].Board[G_Room[Recv_User.Kind].Turn][G_Room[Recv_User.Kind].User_Point[G_Room[Recv_User.Kind].Turn][0]][G_Room[Recv_User.Kind].User_Point[G_Room[Recv_User.Kind].Turn][1]] != 0)
			{
				// 지울 숫자를 구별, 같은 숫자를 가진 모든 보드의 숫자 지움, 다른 보드판이 지워지면 아이템 랜덤 획득 
				G_Room[Recv_User.Kind].GetItem = 0;
				Temp_Number = G_Room[Recv_User.Kind].Board[G_Room[Recv_User.Kind].Turn][G_Room[Recv_User.Kind].User_Point[G_Room[Recv_User.Kind].Turn][0]][G_Room[Recv_User.Kind].User_Point[G_Room[Recv_User.Kind].Turn][1]];
				for (a = 0; a < 6; a++)
				{
					for (b = 0; b < 6; b++)
					{
						for (c = 0; c < 6; c++)
						{
							if (G_Room[Recv_User.Kind].Board[a][b][c] == Temp_Number)
							{
								G_Room[Recv_User.Kind].Board[a][b][c] = 0;
								if (G_Room[Recv_User.Kind].User_Number[a] != Recv_User.Number)
								{
									G_Room[Recv_User.Kind].GetItem = 1;
								}
							}
						}
					}
				}
				//아이템획득 가능이면 아이템을 랜덤으로 줌 
				if (G_Room[Recv_User.Kind].GetItem == 1)
				{
					for (a = 0; a < 6; a++)
					{
						if (G_Room[Recv_User.Kind].User_Number[a] == Recv_User.Number)
						{
							for (b = 0; b < 3; b++)
							{
								if (G_Room[Recv_User.Kind].User_Item[a][b] == 0)
								{
									G_Room[Recv_User.Kind].User_Item[a][b] = rand() % 3 + 1;
									break;
								}
							}
							break;
						}
					}
				}
				//K가 0이면 다음 턴으로 넘어감, 아니면 턴은 그대로 
				if (G_Room[Recv_User.Kind].K == 0)
				{
					G_Room[Recv_User.Kind].Turn += G_Room[Recv_User.Kind].Q;
					if (G_Room[Recv_User.Kind].Turn > 5)
					{
						G_Room[Recv_User.Kind].Turn = 0;
					}
					if (G_Room[Recv_User.Kind].Turn < 0)
					{
						G_Room[Recv_User.Kind].Turn = 5;
					}
					while (G_Room[Recv_User.Kind].User_Number[G_Room[Recv_User.Kind].Turn] == -1)
					{
						G_Room[Recv_User.Kind].Turn += G_Room[Recv_User.Kind].Q;
						if (G_Room[Recv_User.Kind].Turn > 5)
						{
							G_Room[Recv_User.Kind].Turn = 0;
						}
						if (G_Room[Recv_User.Kind].Turn < 0)
						{
							G_Room[Recv_User.Kind].Turn = 5;
						}
						if (G_Room[Recv_User.Kind].Access == 0)
						{
							break;
						}
					}
					//J가 0이 아니면 0이 될때까지 다음 턴 
					while (G_Room[Recv_User.Kind].J != 0)
					{
						G_Room[Recv_User.Kind].J--;
						G_Room[Recv_User.Kind].Turn += G_Room[Recv_User.Kind].Q;
						if (G_Room[Recv_User.Kind].Turn > 5)
						{
							G_Room[Recv_User.Kind].Turn = 0;
						}
						if (G_Room[Recv_User.Kind].Turn < 0)
						{
							G_Room[Recv_User.Kind].Turn = 5;
						}
						while (G_Room[Recv_User.Kind].User_Number[G_Room[Recv_User.Kind].Turn] == -1)
						{
							G_Room[Recv_User.Kind].Turn += G_Room[Recv_User.Kind].Q;
							if (G_Room[Recv_User.Kind].Turn > 5)
							{
								G_Room[Recv_User.Kind].Turn = 0;
							}
							if (G_Room[Recv_User.Kind].Turn < 0)
							{
								G_Room[Recv_User.Kind].Turn = 5;
							}
							if (G_Room[Recv_User.Kind].Access == 0)
							{
								break;
							}
						}
					}
				}
				else
				{
					G_Room[Recv_User.Kind].K--;
				}
			}
		}

		//게임 상황
		// 다음 턴 구별 
		while (G_Room[Recv_User.Kind].User_Number[G_Room[Recv_User.Kind].Turn] == -1)
		{
			G_Room[Recv_User.Kind].Turn += G_Room[Recv_User.Kind].Q;
			if (G_Room[Recv_User.Kind].Turn > 5)
			{
				G_Room[Recv_User.Kind].Turn = 0;
			}
			if (G_Room[Recv_User.Kind].Turn < 0)
			{
				G_Room[Recv_User.Kind].Turn = 5;
			}
			if (G_Room[Recv_User.Kind].Access == 0)// 접속자가 0이면 게임 종료 
			{
				break;
			}
		}

		//득점자 선별 
		for (a = 0; a < 6; a++)
		{
			G_Room[Recv_User.Kind].User_Score[a] = 0;
		}
		for (a = 0; a < 6; a++)
		{
			if (G_Room[Recv_User.Kind].User_Number[a] != -1)
			{
				// 세로선
				for (b = 0; b < G_Room[Recv_User.Kind].Size; b++)
				{
					Temp_Number = 0;
					for (c = 0; c < G_Room[Recv_User.Kind].Size; c++)
					{
						Temp_Number += G_Room[Recv_User.Kind].Board[a][b][c];
					}
					if (Temp_Number == 0)
					{
						G_Room[Recv_User.Kind].User_Score[a]++;
					}
				}
				//가로선 
				for (b = 0; b < G_Room[Recv_User.Kind].Size; b++)
				{
					Temp_Number = 0;
					for (c = 0; c < G_Room[Recv_User.Kind].Size; c++)
					{
						Temp_Number += G_Room[Recv_User.Kind].Board[a][c][b];
					}
					if (Temp_Number == 0)
					{
						G_Room[Recv_User.Kind].User_Score[a]++;
					}
				}
				//대각선 
				Temp_Number = 0;
				for (c = 0; c < G_Room[Recv_User.Kind].Size; c++)
				{
					Temp_Number += G_Room[Recv_User.Kind].Board[a][c][c];
				}
				if (Temp_Number == 0)
				{
					G_Room[Recv_User.Kind].User_Score[a]++;
				}
				Temp_Number = 0;
				for (c = 0; c < G_Room[Recv_User.Kind].Size; c++)
				{
					Temp_Number += G_Room[Recv_User.Kind].Board[a][G_Room[Recv_User.Kind].Size - c - 1][c];
				}
				if (Temp_Number == 0)
				{
					G_Room[Recv_User.Kind].User_Score[a]++;
				}
			}
		}
		//득점 확인
		Temp_Number = 0;
		for (a = 0; a < 6; a++)// 득점이 설정 득점 이상인 사람 선별 
		{
			if (G_Room[Recv_User.Kind].User_Score[a] >= G_Room[Recv_User.Kind].Score)
			{
				Temp_Number++;
				G_Room[Recv_User.Kind].Win = a;
			}
		}
		if (Temp_Number == 1)// 득점자가 1명이면 Win
		{
			G_Room[Recv_User.Kind].Play = 0;
		}
		else if (Temp_Number > 1)// 득점자가 2명 이상이면 무승부 
		{
			G_Room[Recv_User.Kind].Win = 6;
			G_Room[Recv_User.Kind].Play = 0;
		}
		else// 아직 득점자가 없다면 초기화 
		{
			G_Room[Recv_User.Kind].Win = -1;
		}

		Recv_G_Room = G_Room[Recv_User.Kind];// 게임방 정보 수신 버퍼 
	}
}