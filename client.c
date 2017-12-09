#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <pthread.h>
#include <stdio.h>

// 클라이언트 설정
#define TIME 100000
#define W_ROOM -1
#define LOGIN_P -2
#define LOGIN_A -3
#define LOGIN_F -4
#define DEBUG 0

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

// 전역 변수
int Client_Socket;
struct sockaddr_in Client_Addr;
int Port = 9000;
char Send_Name[256];
User_t My_User;
W_Room_t My_W_Room;
G_Room_t My_G_Room;

// 프로토 타입
void* Client_Thread_Run();
void Create_Client();
void My_User_Clear();
void My_W_Room_Clear();

int main(void)
{
	pthread_t Client_Thread;// 클라이언트 쓰레드
	int a;// for문용 

	// ncurses 실행
	initscr();

	My_W_Room_Clear();

	// 유저 로그인
	while(1)
	{
		// 로그인창 출력 

		// 디버그용 
		if(DEBUG == 1)
		{
			mvprintw(0, 0, "My_User_Name : %s", My_User.Name);
			mvprintw(1, 0, "My_User_Numb : %d", My_User.Number);
			mvprintw(2, 0, "My_User_Kind : %d", My_User.Kind);
			mvprintw(3, 0, "My_User_Inpu : %c", My_User.Input);
		}

		mvprintw(23, 72, "[CLient]");
		refresh();

		// 로그인 화면 출력 
		// 테두리
		for(a=0; a<80; a++)
		{
			mvprintw(0, a, "-");
			mvprintw(23, a, "-");
		}
		for(a=1; a<23; a++)
		{
			mvprintw(a, 0, "|");
			mvprintw(a, 79, "|");
		}

		// 로그인 이름 입력
		My_User_Clear();
		mvprintw(10, 30, "Name : ");
		scanw(" %[^\n]s", My_User.Name);

		// 화면 지우기 
		erase();

		// 로그인 상태 : 로그인 요청
		My_User.Kind = LOGIN_P;

		// 클라이언트 생성
		Create_Client();

		// 로그인 데이터 송신
		if(send(Client_Socket, &My_User, sizeof(User_t), 0) == -1)
		{
			endwin();
			printf("[Client_Socket : Send() : Error.]\n");
			exit(1);
		}
		if(send(Client_Socket, &My_W_Room, sizeof(W_Room_t), 0) == -1)
		{
			endwin();
			printf("[Client_Socket : Recv() : Error.]\n");
			exit(1);
		}

		// 로그인 데이터 수신
		if(recv(Client_Socket, &My_User, sizeof(User_t), 0) == -1)
		{
			endwin();
			printf("[Client_Socket : Recv() : Error.]\n");
			exit(1);
		}
		if(recv(Client_Socket, &My_W_Room, sizeof(W_Room_t), 0) == -1)
		{
			endwin();
			printf("[Client_Socket : Recv() : Error.]\n");
			exit(1);
		}
		if(recv(Client_Socket, &My_G_Room, sizeof(G_Room_t), 0) == -1)
		{
			endwin();
			printf("[Client_Socket : Recv() : Error.]\n");
			exit(1);
		}

		// 클라이언트 종료
		close(Client_Socket);

		// 로그인 성공시 루프 탈출 : 실패시 안내문 출력 후 재 로그인
		if(My_User.Kind == W_ROOM)
		{
			break;
		}
		else if(My_User.Kind == LOGIN_A)
		{
			mvprintw(11, 30, "Accessed Name.");
		}
		else if(My_User.Kind == LOGIN_F)
		{
			mvprintw(11, 30, "Server is Full.");
		}
		else
		{
			mvprintw(11, 30, "Error.");
		}
	}

	// 서버 실시간 접속
	pthread_create(&Client_Thread, NULL, Client_Thread_Run, NULL);
	
	// 로그인 성공 후 유저 입력
	while(1)
	{
		My_User.Input = getch();
	}

	// ncurses종료
	endwin();

	return 0;
}

// 클라이언트 쓰레드
void* Client_Thread_Run()
{
	int a, b, c;

	while(1)
	{
		erase(); // 화면 지우기

		// 클라이언트 생성
		Create_Client();

		// 서버에 데이터 송신
		if(send(Client_Socket, &My_User, sizeof(User_t), 0) == -1)
		{
			endwin();
			printf("[Client_Socket : Send() : Error.]\n");
			exit(1);
		}
		if(send(Client_Socket, &My_W_Room, sizeof(W_Room_t), 0) == -1)
		{
			endwin();
			printf("[Client_Socket : Send() : Error.]\n");
			exit(1);
		}

		// 서버의 데이터 수신
		if(recv(Client_Socket, &My_User, sizeof(User_t), 0) == -1)
		{
			endwin();
			printf("[Client_Socket : Recv() : Error.]\n");
			exit(1);
		}
		if(recv(Client_Socket, &My_W_Room, sizeof(W_Room_t), 0) == -1)
		{
			endwin();
			printf("[Client_Socket : Recv() : Error.]\n");
			exit(1);
		}
		if(recv(Client_Socket, &My_G_Room, sizeof(G_Room_t), 0) == -1)
		{
			endwin();
			printf("[Client_Socket : Recv() : Error.]\n");
			exit(1);
		}

		// 데이터에 따른 화면 출력
		// 테두리
		for(a=0; a<80; a++)
		{
			mvprintw(0, a, "-");
			mvprintw(23, a, "-");
		}
		for(a=1; a<23; a++)
		{
			mvprintw(a, 0, "|");
			mvprintw(a, 79, "|");
		}

		// 대기방일 때
		if(My_User.Kind == W_ROOM)
		{
			for(a=0; a<10; a++)
			{
				if(My_W_Room.G_Room_Number[a] == -1)
					mvprintw(2+a, 15, " ");
				else
					mvprintw(2+a, 15, "[%2dRoom : %d/6 Access : %d*%d Size : %d Score : %s ]", My_W_Room.G_Room_Number[a],  My_W_Room.G_Room_Access[a], My_W_Room.G_Room_Size[a], My_W_Room.G_Room_Size[a], My_W_Room.G_Room_Score[a], (My_W_Room.G_Room_Play[a] == 0)? "Wait" : "Play");
			}
			mvprintw(My_W_Room.Point_Y+2, 15, "@");
			mvprintw(My_W_Room.Point_Y+2, 64, "@");

			for(a=1; a<23; a++)
			{
				mvprintw(a, 13, "|");
				mvprintw(a, 66, "|");
			}

			for(a=14; a<65; a++)
			{
				mvprintw(13, a, "-");
			}

			mvprintw(0, 34, "[Wait Room]");
			mvprintw(13, 34, "[Game Key]");

			mvprintw(16, 29, "   [w]    [r]  [o]");
			mvprintw(17, 29, "[a][s][d] [f]  [Enter]");

			mvprintw(19, 17, "w:up   s:down  r:change size  o:out game room");
			mvprintw(20, 17, "a:left d:right f:change score ");
			mvprintw(21, 17, "Enter:join game room, erase number");
		}

		// 게임방일 때
		if(My_User.Kind >= 0)
		{
			mvprintw(0, 0, "[ %2dRoom : %d/6 Access : %d*%d Size : %d Score : %s ]", My_G_Room.Number, My_G_Room.Access, My_G_Room.Size, My_G_Room.Size, My_G_Room.Score, (My_G_Room.Play == 0)? "Wait" : "Play");
			
			for(a=0; a<3; a++)
			{
				mvprintw(2, 2+a*20, "%s", My_G_Room.User_Name[a]);
				for(b=0; b<6; b++)
				{
					for(c=0; c<6; c++)
					{
						if(My_G_Room.User_Point[a][0] == c && My_G_Room.User_Point[a][1] == b)
						{
							mvprintw(3+b, 3+a*20+c*3-1, "[");
							mvprintw(3+b, 3+a*20+c*3+2, "]");
						}
						if(b >= My_G_Room.Size || c >= My_G_Room.Size)
						{
							mvprintw(3+b, 3+a*20+c*3, "  ");
						}
						else if(My_G_Room.Board[a][c][b] == 0)
						{
							mvprintw(3+b, 3+a*20+c*3, "--");
						}
						else if(My_G_Room.User_Number[a] == My_User.Number)
						{
							mvprintw(3+b, 3+a*20+c*3, "%2d", My_G_Room.Board[a][c][b]);
						}
						else
						{
							mvprintw(3+b, 3+a*20+c*3, "[]");
						}
					}
				}
				mvprintw(9, 2+a*20, "Score : %d", My_G_Room.User_Score[a]);
			}

			for(a=3; a<6; a++)
			{
				mvprintw(12, 2+(a-3)*20, "%s", My_G_Room.User_Name[8-a]);
				for(b=0; b<6; b++)
				{
					for(c=0; c<6; c++)
					{
						if(My_G_Room.User_Point[8-a][0] == c && My_G_Room.User_Point[8-a][1] == b)
						{
							mvprintw(13+b, 3+(a-3)*20+c*3-1, "[");
							mvprintw(13+b, 3+(a-3)*20+c*3+2, "]");
						}
						if(b >= My_G_Room.Size || c >= My_G_Room.Size)
						{
							mvprintw(13+b, 3+(a-3)*20+c*3, "  ");
						}
						else if(My_G_Room.Board[8-a][c][b] == 0)
						{
							mvprintw(13+b, 3+(a-3)*20+c*3, "--");
						}
						else if(My_G_Room.User_Number[8-a] == My_User.Number)
						{
							mvprintw(13+b, 3+(a-3)*20+c*3, "%2d", My_G_Room.Board[8-a][c][b]);
						}
						else
						{
							mvprintw(13+b, 3+(a-3)*20+c*3, "[]");
						}
					}
				}
				mvprintw(19, 2+(a-3)*20, "Score : %d", My_G_Room.User_Score[8-a]);
			}
			//시스템 메세지
			if(My_G_Room.Play == 1)
			{
				mvprintw(22, 2, "[System] : %s Turn", My_G_Room.User_Name[My_G_Room.Turn]);
			}
			else if(My_G_Room.Win == 6)
			{
				mvprintw(22, 2, "[System] : Drow!!");
			}
			else if(My_G_Room.Win >= 0 && My_G_Room.Win <=5)
			{
				mvprintw(22, 2, "[System] : %s Win!!", My_G_Room.User_Name[My_G_Room.Win]);
			}
			else
			{
				mvprintw(22, 2, "[System] : ");
			}
		}

		// 디버그용
		if(DEBUG == 1)
		{
			mvprintw(0, 0, "My_User_Name : %s", My_User.Name);
			mvprintw(1, 0, "My_User_Numb : %d", My_User.Number);
			mvprintw(2, 0, "My_User_Kind : %d", My_User.Kind);
			mvprintw(3, 0, "My_User_Inpu : %c", My_User.Input);
			mvprintw(4, 0, "My_W_Room_Point_X : %d My_W_Room_Point_Y : %d", My_W_Room.Point_X, My_W_Room.Point_Y);
		}

		mvprintw(23, 72, "[CLient]");
		refresh();

		// 클라이언트 삭제
		close(Client_Socket);

		// 유저 입력 초기화
		My_User.Input = ' ';

		// 접속 지연
		usleep(TIME);
	}
}

void Create_Client()
{
	// 클라이언트 생성
	if((Client_Socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		endwin();
		printf("[Client_Socket : Socket() : Error.]\n");
		exit(1);
	}

	memset(&Client_Addr, 0, sizeof(Client_Addr));
	Client_Addr.sin_family = AF_INET;
	Client_Addr.sin_port = htons(Port);
	Client_Addr.sin_addr.s_addr = htonl(INADDR_ANY);

	// 서버에 연결
	if(connect(Client_Socket, (struct sockaddr*)&Client_Addr, sizeof(Client_Addr)) == -1)
	{
		endwin();
		printf("[Connect() : Error.]\n");
		exit(1);
	}
}

// 유저 초기화
void My_User_Clear()
{
	strcpy(My_User.Name, "");
	My_User.Number = 0;
	My_User.Kind = 0;
	My_User.Input = ' ';
	My_User.Time = 0;
}

// 대기방 초기화
void My_W_Room_Clear()
{
	int a;
	My_W_Room.Point_X = 0;
	My_W_Room.Point_Y = 0;
	for(a=0; a<10; a++)
	{
		My_W_Room.G_Room_Number[a] = -1;
		My_W_Room.G_Room_Access[a] = -1;
		My_W_Room.G_Room_Play[a] = -1;
		My_W_Room.G_Room_Size[a] = -1;
		My_W_Room.G_Room_Score[a] = -1;
	}
}
