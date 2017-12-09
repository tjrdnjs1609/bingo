//클라이언트 소스코드

//헤더파일 모음 
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
#define TIME 100000//접속 지연 
#define W_ROOM -1//대기바 번호
#define LOGIN_P -2//로그인 요청 번호
#define LOGIN_A -3//중복오류 번호 
#define LOGIN_F -4//서버가득참 번호 
#define DEBUG 0//디버그 
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

// 전역 변수
int Client_Socket;
struct sockaddr_in Client_Addr;
int Port = PORT;
char Send_Name[256];
User_t My_User;
W_Room_t My_W_Room;
G_Room_t My_G_Room;
int Temp_Number=0;

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

		mvprintw( 3,1,"                      ||||||||||||||||||||||||||||||||||                      ");
		mvprintw( 4,1,"    ,############  ,###|||||||||||||||||||||||||||||||||       ,### ,###      ");
		mvprintw( 5,1,"    ;############  ;###        ||||        ||||                ;### ;###      ");
		mvprintw( 6,1,"    ;###''''';###  ''''        ||||        ||||                ;### ;###      ");
		mvprintw( 7,1,"    ;###     ;###  ;###|||||||||||||||||||||||||||||||||       ;### ;###      ");
		mvprintw( 8,1,"    ;############# ;###|,###########|,###########|,########### ;### ;###      ");
		mvprintw( 9,1,"    ;############# ;###|;########### ;########### ;########### ;### ;###      ");
		mvprintw(10,1,"    ;###'''''';### ;###|;###'''';### ;###'''';### ;###'''';### ;### ;###      ");
		mvprintw(11,1,"    ;###      ;### ;###|;###   |;### ;###  ||;### ;###||  ;### '''' ''''      ");
		mvprintw(12,1,"    ;############# ;###|;###   |;### ;########### ;########### ,### ,###      ");
		mvprintw(13,1,"    ;############# ;###|;###   |;### ;########### ;########### ;### ;###      ");
		mvprintw(14,1,"    '''''''''''''' ''''|''''||||'''' '''''''';###|'''''''''''' '''' ''''      ");
		mvprintw(15,1,"                      ||||  |||||||  ,###  ||;###|  ||||                      ");
		mvprintw(16,1,"                      ||||           ;###########   ||||                      ");
		mvprintw(17,1,"                      ||||           ;###########   ||||                      ");
		mvprintw(18,1,"                      |||||||||||||||''''''''''''|||||||                      ");
		mvprintw(19,1,"                      ||||||||||||||||||||||||||||||||||                      ");

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
		mvprintw(20, 30, "Name : ");
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
			mvprintw(21, 30, "Accessed Name.");
		}
		else if(My_User.Kind == LOGIN_F)
		{
			mvprintw(21, 30, "Server is Full.");
		}
		else
		{
			mvprintw(21, 30, "Error.");
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

		// 깜밖임용 
		if(Temp_Number < 0)
		{
			Temp_Number = 10;
		}
		else
		{
			Temp_Number--;
		}

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
			//게임방리스트 작성 
			for(a=0; a<10; a++)
			{
				//없는 방이면 빈칸 출력 , 있는방이면 정보 출력 
				if(My_W_Room.G_Room_Number[a] == -1)
					mvprintw(2+a, 15, " ");
				else
					mvprintw(2+a, 15, "[%2dRoom : %d/6 Access : %d*%d Size : %d Score : %s ]", My_W_Room.G_Room_Number[a],  My_W_Room.G_Room_Access[a], My_W_Room.G_Room_Size[a], My_W_Room.G_Room_Size[a], My_W_Room.G_Room_Score[a], (My_W_Room.G_Room_Play[a] == 0)? "Wait" : "Play");
			}
			//게임방 커서 
			mvprintw(My_W_Room.Point_Y+2, 15, "@");
			mvprintw(My_W_Room.Point_Y+2, 64, "@");

			//리스트 페이지
			mvprintw(12, 37, "< %d >", My_W_Room.Point_X+1);

			//대기방 화면 출력  
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

			mvprintw(15, 17, "[1][2][3]                     1:use item1    ");
			mvprintw(16, 17, "   [w]    [r]  [o]            2:use item2    ");
			mvprintw(17, 17, "[a][s][d] [f]  [Enter]        3:use item3    ");

			mvprintw(19, 17, "w:up   s:down  r:change size  o:out game room");
			mvprintw(20, 17, "a:left d:right f:change score S:game start");
			mvprintw(21, 17, "Enter:join game room, erase number");
		}

		// 게임방일 때
		if(My_User.Kind >= 0)
		{
			//게임방 정보 
			mvprintw(0, 15, "[%2dRoom : %d/6 Access : %d*%d Size : %d Score : %s ]", My_G_Room.Number, My_G_Room.Access, My_G_Room.Size, My_G_Room.Size, My_G_Room.Score, (My_G_Room.Play == 0)? "Wait" : "Play");
			
			//1~3번째 유저 이름, 보드판, 득점, 아이템 출력 
			for(a=0; a<3; a++)
			{
				//유저 이름 
				mvprintw(1, 1+a*20, "%s", My_G_Room.User_Name[a]);
				for(b=0; b<6; b++)
				{
					for(c=0; c<6; c++)
					{
						//보드판 커서 
						if(My_G_Room.User_Point[a][0] == c && My_G_Room.User_Point[a][1] == b)
						{
							mvprintw(2+b, 2+a*20+c*3-1, "[");
							mvprintw(2+b, 2+a*20+c*3+2, "]");
						}
						if(b >= My_G_Room.Size || c >= My_G_Room.Size)
						{
							mvprintw(2+b, 2+a*20+c*3, "  ");//쓰지않는 보드판은 빈칸 
						}
						else if(My_G_Room.Board[a][c][b] == 0)
						{
							mvprintw(2+b, 2+a*20+c*3, "--");//0은 지운칸 
						}
						else if(My_G_Room.User_Number[a] == My_User.Number)//0이 아니고 자신의 보드판이면 숫자 출력
						{
							if(My_G_Room.Board[a][c][b] == My_G_Room.Board[a][My_G_Room.User_Point[a][0]][My_G_Room.User_Point[a][1]])
							{
								if(Temp_Number < 5)
								{
									mvprintw(2+b, 2+a*20+c*3, "%2d", My_G_Room.Board[a][c][b]);
								}
								else
								{
									mvprintw(2+b, 2+a*20+c*3, " ");
								}
							}
							else
							{
								mvprintw(2+b, 2+a*20+c*3, "%2d", My_G_Room.Board[a][c][b]); 
							}
						}
						else
						{
							mvprintw(2+b, 2+a*20+c*3, "[]");//다른유저 보드판은 0빼고 가림 
						}
					}
				}
				//득점, 아이템 
				mvprintw(8, 1+a*20, "Score : %d", My_G_Room.User_Score[a]);
				mvprintw(9, 1+a*20, "Item  : ");
				for(b=0; b<3; b++)
				{
					switch(My_G_Room.User_Item[a][b])
					{
					case 0:
						mvprintw(9, 9+a*20+b*4, "[ ]");//빈칸 
						break;
					case 1:
						mvprintw(9, 9+a*20+b*4, "[J]");//점프 
						break;
					case 2:
						mvprintw(9, 9+a*20+b*4, "[Q]");//역행 
						break;
					case 3:
						mvprintw(9, 9+a*20+b*4, "[K]");//한번더 
						break;
					}
				}
			}

			// 6~4번째 유저 정보 출력, 위외 같음 
			for(a=3; a<6; a++)
			{
				mvprintw(14, 1+(a-3)*20, "%s", My_G_Room.User_Name[8-a]);
				for(b=0; b<6; b++)
				{
					for(c=0; c<6; c++)
					{
						if(My_G_Room.User_Point[8-a][0] == c && My_G_Room.User_Point[8-a][1] == b)
						{
							mvprintw(15+b, 2+(a-3)*20+c*3-1, "[");
							mvprintw(15+b, 2+(a-3)*20+c*3+2, "]");
						}
						if(b >= My_G_Room.Size || c >= My_G_Room.Size)
						{
							mvprintw(15+b, 2+(a-3)*20+c*3, "  ");
						}
						else if(My_G_Room.Board[8-a][c][b] == 0)
						{
							mvprintw(15+b, 2+(a-3)*20+c*3, "--");
						}
						else if(My_G_Room.User_Number[8-a] == My_User.Number)
						{
							if(My_G_Room.Board[8-a][c][b] == My_G_Room.Board[8-a][My_G_Room.User_Point[8-a][0]][My_G_Room.User_Point[8-a][1]])
							{
								if(Temp_Number < 5)
								{
									mvprintw(15+b, 2+(a-3)*20+c*3, "%2d", My_G_Room.Board[8-a][c][b]);
								}
								else
								{
									mvprintw(15+b, 2+(a-3)*20+c*3, " ");
								}
							}
							else
							{
								mvprintw(15+b, 2+(a-3)*20+c*3, "%2d", My_G_Room.Board[8-a][c][b]); 
							}
						}
						else
						{
							mvprintw(15+b, 2+(a-3)*20+c*3, "[]");
						}
					}
				}
				mvprintw(21, 1+(a-3)*20, "Score : %d", My_G_Room.User_Score[8-a]);
				mvprintw(22, 1+(a-3)*20, "Item  : ");
				for(b=0; b<3; b++)
				{
					switch(My_G_Room.User_Item[8-a][b])
					{
					case 0:
						mvprintw(22, 9+(a-3)*20+b*4, "[ ]");
						break;
					case 1:
						mvprintw(22, 9+(a-3)*20+b*4, "[J]");
						break;
					case 2:
						mvprintw(22, 9+(a-3)*20+b*4, "[Q]");
						break;
					case 3:
						mvprintw(22, 9+(a-3)*20+b*4, "[K]");
						break;
					}
				}
			}

			// 게임방 테두리
			for(a=1; a<23; a++)
			{
				mvprintw(a, 20, "|");
				mvprintw(a, 40, "|");
				mvprintw(a, 60, "|");
			}
			for(a=1; a<60; a++)
			{
				mvprintw(10, a, "-");
				mvprintw(11, a, " ");
				mvprintw(12, a, " ");
				mvprintw(13, a, "-");
			}

			// 아이템 설명
			mvprintw(1, 61, "[Item]");
			mvprintw(2, 61, " J: jump turn.");
			mvprintw(3, 61, " Q: reverse turn.");
			mvprintw(4, 61, " K: one more.");

			mvprintw(6, 61, "[Game Key]");
			mvprintw(7, 61, " w: up.");
			mvprintw(8, 61, " s: down.");
			mvprintw(9, 61, " a: left.");
			mvprintw(10, 61, " d: right.");
			mvprintw(11, 61, " enter:");
			mvprintw(12, 61, "  erase number.");
			mvprintw(13, 61, " 1: use item1.");
			mvprintw(14, 61, " 2: use item2.");
			mvprintw(15, 61, " 3: use item3.");
			mvprintw(16, 61, " o: game room out.");

			mvprintw(18, 61, "[Owner]");
			mvprintw(19, 61, " S: start game.");
			mvprintw(20, 61, " r: change size.");
			mvprintw(21, 61, " f: change score.");

			//시스템 메세지
			if(My_G_Room.Play == 1)
			{
				mvprintw(11, 22, "%s\t Turn!!", My_G_Room.User_Name[My_G_Room.Turn]);//다음턴 
			}
			else if(My_G_Room.Win == 6)
			{
				mvprintw(11, 27, "Drow!!");//무승부 
			}
			else if(My_G_Room.Win >= 0 && My_G_Room.Win <=5)
			{
				mvprintw(11, 22, "%s\t Win!!", My_G_Room.User_Name[My_G_Room.Win]);//이김 
			}
			else
			{
				mvprintw(11, 26, " ");//그외 
			}

			if(My_G_Room.Play == 1)
			{
				if(My_G_Room.J != 0)
				{
					mvprintw(11, 6, " Jump*%d!! ", My_G_Room.J);//점프아이템 사용 
				}
				if(My_G_Room.K != 0)
				{
					mvprintw(12, 6, " More*%d!! ", My_G_Room.K);//한번더 사용 
				}
				if(My_G_Room.Q == 1)
				{
					mvprintw(11, 46, "  .-->  ");//순행 
					mvprintw(12, 46, "  <--'  ");
				}
				else
				{
					mvprintw(11, 46, "  <--.  ");//역행 
					mvprintw(12, 46, "  '-->  ");
				}
				if(My_G_Room.GetItem == 1)
				{
					mvprintw(12, 26, "Item Get!!");//아이템 얻음 
				}
			}

			// 방장 표시
			if(My_G_Room.Play == 0)
			{
				mvprintw(10, 6, "[ Owner ]");
			}

			// 턴 표시
			if(My_G_Room.Play == 1)
			{
			switch(My_G_Room.Turn)
			{
				case 0:
					mvprintw(10, 6, "[Turn!!]");
					break;
				case 1:
					mvprintw(10, 26, "[Turn!!]");
					break;
				case 2:
					mvprintw(10, 46, "[Turn!!]");
					break;
				case 3:
					mvprintw(13, 46, "[Turn!!]");
					break;
				case 4:
					mvprintw(13, 26, "[Turn!!]");
					break;
				case 5:
					mvprintw(13, 6, "[Turn!!]");
					break;
				}
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
