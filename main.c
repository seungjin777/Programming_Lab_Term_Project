#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "func.h"
#include <mmsystem.h> //bgm관련 헤더
#pragma comment(lib, "winmm.lib")

//게임 관련 정의
#define WIDTH 300 //전부 픽셀로할거면 *1, 특수문자 *2
#define HEIGHT 190
#define BLANK ' '
#define MAXHP 40 //최대 체력 (배열 판정이라 특정 위치에서 *2만큼 깎임)
#define MAXBULLET 99 //총알 최대 개수
#define MAXTIME 90 //240초
#define MAXSCORE 15 //최대점수 15점, 최대점수 변경시 설명북 설명도 <<변경하기>>

// 색상 정의
#define BLACK	0
#define BLUE1	1
#define GREEN1	2
#define CYAN1	3
#define RED1	4
#define MAGENTA1 5
#define YELLOW2	6
#define GRAY1	7
#define GRAY2	8
#define SKIN	9
#define GREEN2	10
#define CYAN2	11
#define ORANGE	12
#define PINK    13
#define BROWN	14
#define WHITE	15
// 키 정의
#define ESC 0x1b
#define SPACE ' '
#define SPECIAL1 0xe0 // 특수키는 0xe0 + key 값으로 구성된다.
#define SPECIAL2 0x00 // keypad 경우 0x00 + key 로 구성된다.
#define UP2  0x48 // Up key는 0xe0 + 0x48 두개의 값이 들어온다.
#define DOWN2 0x50
#define LEFT2 0x4b
#define RIGHT2 0x4d
#define UP		'w'
#define DOWN	's'
#define LEFT	'a'
#define RIGHT	'd'
#define SHOT2   '1'
#define SHOT    'g'
#define SKILL2  '0'
#define SKILL   'f'
#define ITEM    'h'
#define ITEM2   '2'

//전역변수들
//디폴트 맵 생성, 점령자---------------------------------------------------------------
int map[HEIGHT][WIDTH] = { 0 };
int flag_player = 0; //거점 점령자 0:미점령 1:레드 2:블루
int winner = 0; //승리자 0:무승부 1:레드 2:블루
int deco_swt = 1;
int item_switch[3] = { 0 }; //아이템 출력 여부 스위치

//시간 조절 딜레이--------------------------------------------------------------------
#define DELAY 25
int keep_moving = 1; // 1:계속이동, 0:한칸씩이동.
int time_out = MAXTIME; // 제한시간
int score[2] = { 0 };
int called[2];  //p1 처음시작?
int called2[2]; //p2 처음시작?
//int frame_count = 0; // game 진행 frame count 로 속도 조절용으로 사용된다.
int p1_frame_sync = 10; // 처음 시작은 10 frame 마다 이동, 즉, 100msec 마다 이동
int p1_frame_sync_start = 0; // 
int p2_frame_sync = 10; // 처음 시작은 10 frame 마다 이동
int p2_frame_sync_start = 0;
long long frame_count = 0;

//player 정보 구조체//**************************************************************
typedef struct playerInfo {
    int move_ch; //플레이어 계속 이동 여부(1:one,0:keep)
    int oldX; int newX;     //플레이어 좌표 @@@@@좌표는 배열기준으로 생각한다!!@@@@@@@
    int oldY; int newY;
    int arrow;  //플레이어의 시선(왼:-1, 오:1)
    int shots; //남은 탄수
    int skiPow; // 스킬 게이지
    int score; //킬 횟수
    int hp; //체력(풀피: 20)정도?
    int item_slot; //플레이어의 아이템 슬롯
    //int skill_X; int skill_Y; //스킬 사용 좌표
}playerInfo;

typedef struct ballPoint {
    int ball_cheak; //발사체 on, off
    int oldX; int newX; //발사체 좌표(파이어볼)  @@@@@좌표는 배열기준으로 생각한다!!@@@@@@@
    int oldY; int newY;
    int arrow;  //발사체 방향(왼:-1, 오:1)
}ballPoint;

typedef struct pos {
    int y; int x; int val;
}pos;

pos itemxy[3] = { {61, 40, 0},{61, 260, 0},{80, 146, 0} };
playerInfo Red = { 0 }; //RED의 플레이어 좌표 선언
playerInfo Blue = { 0 }; //BLUE의 플레이어 좌표 선언
ballPoint Red_ball[MAXBULLET]; //RED의 탄막 좌표 선언
ballPoint Blue_ball[MAXBULLET]; //BLUE의 탄막 좌표 선언
//***********************************************************************************

//함수부분
//커서안보이게
void removeCursor(void) { // 커서를 안보이게 한다
    CONSOLE_CURSOR_INFO curInfo;
    GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &curInfo);
    curInfo.bVisible = 0;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &curInfo);
}

//밑에 2개함수는 뭔지 모르겠음, 창이랑 버퍼 크기 조절?, 3번째 폰트 조절 함수, 버퍼크기 변경
void ChangeScreenSize(HANDLE hnd, COORD NewSize)
{
    //HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    SMALL_RECT DisplayArea = { 0, 0, 0, 0 };
    CONSOLE_SCREEN_BUFFER_INFO SBInfo;
    GetConsoleScreenBufferInfo(hnd, &SBInfo);
    DisplayArea.Bottom = NewSize.Y;
    DisplayArea.Right = NewSize.X;
    SetConsoleWindowInfo(hnd, 1, &DisplayArea);
}
void ChangeBuffSize(HANDLE hnd, COORD NewSize)
{
    SetConsoleScreenBufferSize(hnd, NewSize);
}
int ChangefontSize(int x, int y) //폰트 사이즈 변경
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole == INVALID_HANDLE_VALUE) {
        printf("Error getting console handle\n");
        return 1;
    }

    CONSOLE_FONT_INFOEX cfi;
    cfi.cbSize = sizeof(CONSOLE_FONT_INFOEX);
    cfi.nFont = 0;
    cfi.dwFontSize.X = x;                  // 폰트 x
    cfi.dwFontSize.Y = y;                 // 폰트 y
    cfi.FontFamily = FF_DONTCARE;
    cfi.FontWeight = FW_NORMAL;
    //wcscpy(cfi.FaceName, L"Consolas");     // Choose your font, e.g., 글꼴

    if (!SetCurrentConsoleFontEx(hConsole, FALSE, &cfi)) {
        printf("Error setting console font\n");
        return 1;
    }

    printf("Console font changed to 8x16 successfully\n");
    return 0;
}

//폰트색상, 액티브버퍼 클리어, 좌표로 이동, 문자열 출력, 원하는 위치 출력(@@주의 x,y로 입력@@)
void textcolor(int fg_color, int bg_color)//색변경
{
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), fg_color | bg_color << 4);
}
void cls(int text_color, int bg_color) // 화면 지우기고 원하는 배경색으로 설정한다.
{
    char cmd[100];
    system("cls");
    // 화면 크기 강제로 조정한다.
    sprintf(cmd, "mode con: cols=%d lines=%d", WIDTH * 2, HEIGHT);
    system(cmd);
    sprintf(cmd, "COLOR %x%x", bg_color, text_color);
    system(cmd);
}
void gotoxy(int x, int y)
{
    COORD pos = { x, y };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);// WIN32API 함수입니다. 이건 알필요 없어요
}
void printxy(int x, int y, char* str) //커서이동이랑 그리기 한번에 정의한 함수??
{//보여지는 화면에 출력
    gotoxy(x, y);
    printf("%s", str);
}

//기본오브젝트 맵배열배치, 상단 기본인터페이스, 배치된 맵 출력
void map_cls() //맵 콜라이더 클리어
{

    for (int i = 0; i < HEIGHT; i++)
    {
        for (int j = 0; j < WIDTH; j++)
        {
            map[i][j] = 0;
        }
    }
}
void default_map() //맵 배열에 생성되어야할 변하지 않는 오브젝트들
{
    int x, y; //오브젝트의 시작 지점 //여기선 x 수직, y 가로 @@주의할것@@

    for (x = 45; x <= 49; x++) //1번 블럭
    {
        for (y = 65; y <= 235; y++)
            map[x][y] = 1;
    }
    for (x = 70; x <= 74; x++) //2, 3번 블럭
    {
        for (y = 22; y <= 62; y++) //2
            map[x][y] = 1;
        for (y = 238; y <= 278; y++) //3
            map[x][y] = 1;
    }
    for (x = 83; x <= 87; x++) //4, 5번 블럭
    {
        for (y = 81; y <= 131; y++) //4
            map[x][y] = 1;
        for (y = 169; y <= 219; y++) //5
            map[x][y] = 1;
    }
    for (x = 111; x <= 115; x++) //6번 블럭
    {
        for (y = 40; y <= 90; y++)
            map[x][y] = 1;
    }
    for (x = 117; x <= 121; x++) //7번 블럭
    {
        for (y = 150; y <= 270; y++)
            map[x][y] = 1;
    }
    for (x = 125; x <= 141; x++) //17번 블럭(새로 추가)
    {
        for (y = 291; y <= 295; y++)
            map[x][y] = 1;
    }
    for (x = 135; x <= 141; x++) //18번 블럭(새로 추가)
    {
        for (y = 286; y <= 290; y++)
            map[x][y] = 1;
    }
    for (x = 117; x <= 130; x++) //8번 블럭
    {
        for (y = 145; y <= 149; y++)
            map[x][y] = 1;
    }
    for (x = 128; x <= 133; x++) //19번 블럭(새로 추가)
    {
        for (y = 141; y <= 149; y++)
            map[x][y] = 1;
    }
    for (x = 138; x <= 141; x++) //20번 블럭(새로 추가)
    {
        for (y = 120; y <= 130; y++)
            map[x][y] = 1;
    }
    for (x = 142; x <= 146; x++) //9, 10 ,11번 블럭
    {
        for (y = 0; y <= 40; y++) //9
            map[x][y] = 1;
        for (y = 55; y <= 140; y++) //10
            map[x][y] = 1;
        for (y = 260; y <= 299; y++) //11
            map[x][y] = 1;
    }
    for (x = 143; x <= 173; x++) //12, 13 블럭
    {
        for (y = 36; y <= 40; y++) //12
            map[x][y] = 1;
        for (y = 260; y <= 264; y++) //13
            map[x][y] = 1;
    }
    for (x = 165; x <= 173; x++) //14 블럭
    {
        for (y = 80; y <= 90; y++) //14
            map[x][y] = 1;
    }
    for (x = 157; x <= 173; x++) //15 블럭
    {
        for (y = 197; y <= 217; y++) //15
            map[x][y] = 1;
    }
    for (x = 167; x <= 173; x++) //16 블럭
    {
        for (y = 217; y <= 222; y++) //16
            map[x][y] = 1;
    }
    for (y = 0; y < 300; y++) // ground 블럭, 테두리 블럭
    {
        for (x = 174; x < 189; x++)
            map[x][y] = 1;
        map[18][y] = 1;
    }
    for (x = 18; x < 175; x++) // ground 블럭, 테두리 블럭
    {
        map[x][0] = 1;
        map[x][1] = 1;
        map[x][2] = 1;
        map[x][3] = 1;
        map[x][296] = 1;
        map[x][297] = 1;
        map[x][298] = 1;
        map[x][299] = 1;
    }

    //거점 포인트(깃발 포인트)
    for (x = 38; x <= 45; x++) //외부 콜라이더, 삭제 방지
    {
        for (y = 145; y <= 153; y++)
            map[x][y] = 1;
    }
    for (x = 39; x <= 44; x++) //내부 콜라이더, 플레이어 인식
    {
        for (y = 146; y <= 152; y++)
            map[x][y] = 3;
    }


    //스폰, 워프 존
    for (x = 175; x <= 178; x++) //스폰 RED, 스폰 BLUE, 워프존 1-1
    {
        for (y = 5; y <= 15; y++) //RED
            map[x][y] = 5;
        for (y = 284; y <= 294; y++) //BLUE
            map[x][y] = 5;
        for (y = 146; y <= 153; y++) //워프존 1
            map[x][y] = 5;
    }
    for (x = 118; x <= 120; x++) //워프존 2
    {
        for (y = 147; y <= 152; y++)
            map[x][y] = 5;
    }

}
void default_interface() //기본 UI
{
    //아스키 아트 Banner3폰트 ,, 점수부분이 21+3칸정도 됨
    int i, j;
    int color;
    int n = 1;
    for (i = 0; i < 2; i++) //총알 수, 스킬 쿨타임, 아이템 슬롯
    {
        if (i == 0) color = PINK;
        else color = CYAN2;
        textcolor(BLACK, color);
        printxy(n, 2, " ######  ##     ##  #######  ########  ######     ");
        printxy(n, 3, "##    ## ##     ## ##     ##    ##    ##    ##  ##");
        printxy(n, 4, "##       ##     ## ##     ##    ##    ##          ");
        printxy(n, 5, " ######  ######### ##     ##    ##     ######     ");
        printxy(n, 6, "      ## ##     ## ##     ##    ##          ##    ");
        printxy(n, 7, "##    ## ##     ## ##     ##    ##    ##    ##  ##");
        printxy(n, 8, " ######  ##     ##  #######     ##     ######     ");
        ////////////////////////(53~77)숫자
        n += 76;
        printxy(n, 2, " ######  ##    ## #### ##       ##          ");
        printxy(n, 3, "##    ## ##   ##   ##  ##       ##        ##");
        printxy(n, 4, "##       ##  ##    ##  ##       ##          ");
        printxy(n, 5, " ######  #####     ##  ##       ##          ");
        printxy(n, 6, "      ## ##  ##    ##  ##       ##          ");
        printxy(n, 7, "##    ## ##   ##   ##  ##       ##        ##");
        printxy(n, 8, " ######  ##    ## #### ######## ########    ");
        /////////////////////////(123~147)숫자
        n += 70;
        printxy(n, 2, "#### ######## ######## ##     ##    ");
        printxy(n, 3, " ##     ##    ##       ###   ###  ##");
        printxy(n, 4, " ##     ##    ##       #### ####    ");
        printxy(n, 5, " ##     ##    ######   ## ### ##    ");
        printxy(n, 6, " ##     ##    ##       ##     ##    ");
        printxy(n, 7, " ##     ##    ##       ##     ##  ##");
        printxy(n, 8, "####    ##    ######## ##     ##    ");

        n += 254;
    }

    //레드, 블루 점수
    n = 203;
    {
        textcolor(BLACK, PINK);
        printxy(n, 2, "########  ######## ########     ");
        printxy(n, 3, "##     ## ##       ##     ##  ##");
        printxy(n, 4, "##     ## ##       ##     ##    ");
        printxy(n, 5, "########  ######   ##     ##    ");
        printxy(n, 6, "##   ##   ##       ##     ##    ");
        printxy(n, 7, "##    ##  ##       ##     ##  ##");
        printxy(n, 8, "##     ## ######## ########     ");

        //점수
        n += 86;
        textcolor(YELLOW2, GRAY2);
        printxy(n, 2, " ##     ##  ######  ");
        printxy(n, 3, " ##     ## ##    ## ");
        printxy(n, 4, " ##     ## ##       ");
        printxy(n, 5, " ##     ##  ######  ");
        printxy(n, 6, "  ##   ##        ## ");
        printxy(n, 7, "   ## ##   ##    ## ");
        printxy(n, 8, "    ###     ######  ");
        n += 66;
        textcolor(BLACK, CYAN2);
        printxy(n, 2, "    ########  ##       ##     ## ########");
        printxy(n, 3, "##  ##     ## ##       ##     ## ##      ");
        printxy(n, 4, "    ##     ## ##       ##     ## ##      ");
        printxy(n, 5, "    ########  ##       ##     ## ######  ");
        printxy(n, 6, "    ##     ## ##       ##     ## ##      ");
        printxy(n, 7, "##  ##     ## ##       ##     ## ##      ");
        printxy(n, 8, "    ########  ########  #######  ########");
        textcolor(BLACK, WHITE);
        //점수
    }

    textcolor(BLACK, BLACK);//검정 박스
    for (i = 199; i < 600; i += 200)
    {
        for (j = 0; j < 10; j++)
            printxy(i, j, "  ");
    }
    for (i = 0; i < 600; i++) {
        printxy(i, 0, " ");
        printxy(i, 10, " ");
    }

    textcolor(GREEN1, GRAY1);
    for (i = 201; i < 381; i += 177) //HP 텍스트 출력
    {
        printxy(i, 11, " ##     ## ########  ");
        printxy(i, 12, " ##     ## ##     ## ");
        printxy(i, 13, " ##     ## ##     ## ");
        printxy(i, 14, " ######### ########  ");
        printxy(i, 15, " ##     ## ##        ");
        printxy(i, 15, " ##     ## ##        ");
        printxy(i, 16, " ##     ## ##        ");
        printxy(i, 17, " ##     ## ##        ");
    }
    textcolor(BLACK, WHITE);
}
void DrawMap() // DrawMap()은 Hidden screen에 그려준다.
{
    static int i = 0;
    int x, y;
    cls(BLACK, WHITE);

    for (y = 0; y < HEIGHT; y++) {
        for (x = 0; x < WIDTH; x++) {
            if (map[y][x] != 0)
                gotoxy(x * 2, y);
            switch (map[y][x]) {
            case 1: //블록
                textcolor(GRAY2, GRAY2);
                printf("  ");
                break;
            case 2: //로프
                textcolor(BROWN, WHITE);
                printf("  ");
                break;
            case 3: //거점 포인트
                textcolor(PINK, PINK);
                printf("  ");
                break;
            case 4: //스폰 포인트
                textcolor(GREEN1, YELLOW2);
                printf("〓");
                break;
            case 5: //워프 존
                textcolor(MAGENTA1, MAGENTA1);
                printf("  ");
                break;
            case 98: case 99: //98:BLUE 99:RED (player_Collider)
                textcolor(BLACK, BLACK);
                printf("  ");
                break;
            case 44: //콜라이더 테스트용
                textcolor(PINK, PINK);
                printf("  ");
                break;
            }
        }
    }
    default_interface();//기본 인터페이스 생성
    //i = i ? 0 : 1; // 별이 깜빡거리게 해 준다.
}

//상호작용 관련 함수들(워프, 순간이동)
void warp(int player)
{
    int x = 0, y = 0; //워프존의 좌표
    int* nx = NULL;
    int* ny = NULL; //이동할 좌표
    int n = 0; //랜덤 워프존 인덱스
    if (player == 1)
    {
        x = Red.oldX; //그대로
        y = Red.oldY + 12 + 2; //플레이어 발 + 2아래
        nx = &Red.newX;
        ny = &Red.newY;
    }
    else if (player == 2)
    {
        x = Blue.oldX; //그대로
        y = Blue.oldY + 12 + 2; //플레이어 발 + 2아래
        nx = &Blue.newX;
        ny = &Blue.newY;
    }

    if ((5 <= x && x <= 20) && y >= 175) //레드 스폰일때
    {
        *nx = 17;
        *ny = 125;
    }
    else if ((279 <= x && x <= 294) && y >= 175) //블루 스폰일때
    {
        *nx = 277;
        *ny = 125;
    }

    if ((142 <= x && x <= 153) && y >= 175)//워프존 1을 밟았을때
    {
        n = rand() % 2; //상단 1-1, 1-2 워프존 중 랜덤으로 소환
        switch (n)
        {
        case 0: //1-1 워프존
            *nx = 70; *ny = 28;
            break;
        case 1: //1-2 워프존
            *nx = 230; *ny = 28;
            break;
        }
    }
    else if ((142 <= x && x <= 152) && y >= 118)//워프존 2를 밟았을때
    {
        n = rand() % 6; //1-1, 1-2, 2-1, 2-2, 2-3, 2-4 워프존 중 랜덤으로 소환
        switch (n)
        {
        case 0: //1-1 워프존
            *nx = 70; *ny = 28;
            break;
        case 1: //1-2 워프존
            *nx = 230; *ny = 28;
            break;
        case 2: //2-1 워프존
            *nx = 27; *ny = 55;
            break;
        case 3: //2-2 워프존
            *nx = 263; *ny = 55;
            break;
        case 4: //2-3 워프존
            *nx = 86; *ny = 68;
            break;
        case 5: //2-4 워프존
            *nx = 209; *ny = 68;
            break;
        }
    }
}
void checkFlag(int player) //깃발 쟁탈시 색변경
{
    int color = 0;
    int x = 145 * 2;
    int y = 38;
    if (player == 1) //레드가 점령
    {
        flag_player = 1;
        color = RED1;

        textcolor(RED1, RED1);
        printxy(224, 11, "                                                          ");
        printxy(224, 12, "                                                          ");
        printxy(224, 13, "                                                          ");
        printxy(224, 14, "                                                          ");
        printxy(224, 15, "                                                          ");
        printxy(224, 15, "                                                          ");
        printxy(224, 16, "                                                          ");
        printxy(224, 17, "                                                          ");
        textcolor(GRAY1, GRAY1);
        printxy(316, 11, "                                                            ");
        printxy(316, 12, "                                                            ");
        printxy(316, 13, "                                                            ");
        printxy(316, 14, "                                                            ");
        printxy(316, 15, "                                                            ");
        printxy(316, 15, "                                                            ");
        printxy(316, 16, "                                                            ");
        printxy(316, 17, "                                                            ");
    }
    else if (player == 2) //블루가 점령
    {
        flag_player = 2;
        color = BLUE1;

        textcolor(GRAY1, GRAY1);
        printxy(224, 11, "                                                          ");
        printxy(224, 12, "                                                          ");
        printxy(224, 13, "                                                          ");
        printxy(224, 14, "                                                          ");
        printxy(224, 15, "                                                          ");
        printxy(224, 15, "                                                          ");
        printxy(224, 16, "                                                          ");
        printxy(224, 17, "                                                          ");
        textcolor(BLUE1, BLUE1);
        printxy(316, 11, "                                                            ");
        printxy(316, 12, "                                                            ");
        printxy(316, 13, "                                                            ");
        printxy(316, 14, "                                                            ");
        printxy(316, 15, "                                                            ");
        printxy(316, 15, "                                                            ");
        printxy(316, 16, "                                                            ");
        printxy(316, 17, "                                                            ");
    }
    else //미점령
    {
        flag_player = 0;
        color = GRAY1;

        textcolor(GRAY1, GRAY1);
        printxy(224, 11, "                                                          ");
        printxy(224, 12, "                                                          ");
        printxy(224, 13, "                                                          ");
        printxy(224, 14, "                                                          ");
        printxy(224, 15, "                                                          ");
        printxy(224, 15, "                                                          ");
        printxy(224, 16, "                                                          ");
        printxy(224, 17, "                                                          ");
        textcolor(GRAY1, GRAY1);
        printxy(316, 11, "                                                            ");
        printxy(316, 12, "                                                            ");
        printxy(316, 13, "                                                            ");
        printxy(316, 14, "                                                            ");
        printxy(316, 15, "                                                            ");
        printxy(316, 15, "                                                            ");
        printxy(316, 16, "                                                            ");
        printxy(316, 17, "                                                            ");
    }

    //깃발 그리기
    textcolor(WHITE, WHITE); printxy(x, y + 0, "                  ");

    textcolor(WHITE, WHITE); printxy(x, y + 1, "    ");
    textcolor(BROWN, BROWN); printxy(x + 4, y + 1, "  ");
    textcolor(color, color); printxy(x + 6, y + 1, "    ");
    textcolor(WHITE, WHITE); printxy(x + 10, y + 1, "        ");

    textcolor(WHITE, WHITE); printxy(x, y + 2, "    ");
    textcolor(BROWN, BROWN); printxy(x + 4, y + 2, "  ");
    textcolor(color, color); printxy(x + 6, y + 2, "        ");
    textcolor(WHITE, WHITE); printxy(x + 14, y + 2, "    ");

    textcolor(WHITE, WHITE); printxy(x, y + 3, "    ");
    textcolor(BROWN, BROWN); printxy(x + 4, y + 3, "  ");
    textcolor(color, color); printxy(x + 6, y + 3, "          ");
    textcolor(WHITE, WHITE); printxy(x + 16, y + 3, "  ");

    textcolor(WHITE, WHITE); printxy(x, y + 4, "    ");
    textcolor(BROWN, BROWN); printxy(x + 4, y + 4, "  ");
    textcolor(color, color); printxy(x + 6, y + 4, "      ");
    textcolor(WHITE, WHITE); printxy(x + 12, y + 4, "      ");

    textcolor(WHITE, WHITE); printxy(x, y + 5, "    ");
    textcolor(BROWN, BROWN); printxy(x + 4, y + 5, "  ");
    textcolor(WHITE, WHITE); printxy(x + 6, y + 5, "            ");

    textcolor(WHITE, WHITE); printxy(x, y + 6, "    ");
    textcolor(BROWN, BROWN); printxy(x + 4, y + 6, "  ");
    textcolor(WHITE, WHITE); printxy(x + 6, y + 6, "            ");

    textcolor(BLACK, BLACK); printxy(x, y + 7, "                  ");
    textcolor(BLACK, WHITE);

}
void use_skill(int player)
{
    int x = 0, rmX = 0;
    int y = 0, rmY = 0;
    int arrow = 0, rmArrow = 0;
    int color1 = 0, color2 = 0;
    long long this_frame_count = 0; //속도 조절용
    int skill_frame_sync = 0; //스킬 프레임
    int hit = 0, enemy = 0; //적군 적중
    int i = 0, n = 0;

    if (player == 1) //RED
    {
        arrow = rmArrow = Red.arrow;
        x = rmX = (Red.newX + 3) + (arrow * 5);
        y = rmY = Red.newY + 1;
        color1 = RED1; color2 = ORANGE;
        enemy = 98;
    }
    else if (player == 2) //BLUE
    {
        arrow = rmArrow = Blue.arrow;
        x = rmX = (Blue.newX + 3) + (arrow * 5);
        y = rmY = Blue.newY + 1;
        color1 = BLUE1; color2 = CYAN1;
        enemy = 99;
    }

    //스킬 구현 부분
    this_frame_count = 0;
    skill_frame_sync = 20000;
    while (1) //기충전
    {
        textcolor(color2, color1);
        if (this_frame_count % skill_frame_sync == 0) //기충전 잠깐 시간 멈추고 발사!!!
        {
            printxy(x * 2 + arrow * (n + 2), y + n, " ▼ ");
            printxy(x * 2 + arrow * (n + 2), y + 9 - n, " ▲ "); // X 표식을 그리고 스킬 발동!
            n += 2;
        }
        this_frame_count++;
        if (n >= 11)
        {
            break;
        }
    }

    while (1)//발사
    {
        if (map[y + 0][x] != 1 && map[y + 3][x] != 1 &&
            map[y + 6][x] != 1 && map[y + 9][x] != 1)
        {
            textcolor(color2, color1); printxy(x * 2, y + 0, "◆");
            textcolor(color1, color2); printxy(x * 2, y + 1, "  ");
            textcolor(color1, color2); printxy(x * 2, y + 2, "  ");
            textcolor(color2, color1); printxy(x * 2, y + 3, "◆");
            textcolor(color1, color2); printxy(x * 2, y + 4, "  ");
            textcolor(color1, color2); printxy(x * 2, y + 5, "  ");
            textcolor(color2, color1); printxy(x * 2, y + 6, "◆");
            textcolor(color1, color2); printxy(x * 2, y + 7, "  ");
            textcolor(color1, color2); printxy(x * 2, y + 8, "  ");
            textcolor(color2, color1); printxy(x * 2, y + 9, "◆");

            if (map[y + 0][x] == enemy || map[y + 3][x] == enemy ||
                map[y + 6][x] == enemy || map[y + 9][x] == enemy)
            {
                hit = 1;
            }
        }
        else
        {
            break;
        }
        x += arrow;
    }
    textcolor(BLACK, WHITE);

    //스킬 적중시 상대 체력 (*)
    if (player == 1 && hit)
    {
        Blue.hp = 0;
        print_hp(2);
    }
    else if (player == 2 && hit)
    {
        Red.hp = 0;
        print_hp(1);
    }

    //출력된 스킬 지우기
    this_frame_count = 0;
    skill_frame_sync = 25000;
    while (1)
    {
        if (this_frame_count % skill_frame_sync == 0) //천천히 지움
        {
            if (map[rmY - 1][rmX] != 1 && map[rmY + 0][rmX] != 1 && map[rmY + 1][rmX] != 1 &&
                map[rmY + 2][rmX] != 1 && map[rmY + 3][rmX] != 1 && map[rmY + 4][rmX] != 1 &&
                map[rmY + 5][rmX] != 1 && map[rmY + 6][rmX] != 1 && map[rmY + 7][rmX] != 1 &&
                map[rmY + 8][rmX] != 1 && map[rmY + 9][rmX] != 1 && map[rmY + 10][rmX] != 1)
            {
                printxy(rmX * 2, rmY - 1, "  ");
                printxy(rmX * 2, rmY + 0, "  ");
                printxy(rmX * 2, rmY + 1, "  ");
                printxy(rmX * 2, rmY + 2, "  ");
                printxy(rmX * 2, rmY + 3, "  ");
                printxy(rmX * 2, rmY + 4, "  ");
                printxy(rmX * 2, rmY + 5, "  ");
                printxy(rmX * 2, rmY + 6, "  ");
                printxy(rmX * 2, rmY + 7, "  ");
                printxy(rmX * 2, rmY + 8, "  ");
                printxy(rmX * 2, rmY + 9, "  ");
                printxy(rmX * 2, rmY + 10, "  ");
                rmX += arrow;
            }
            else
            {
                break;
            }
        }
        this_frame_count++;
    }
}

//player관련 함수들
void Character_drow(int y, int x, int player) //캐릭터 그림 13x6
{
    int color = 0;
    int arrow = 0;
    if (player == 1)
    {
        color = RED1;
        arrow = Red.arrow;
    }
    else if (player == 2)
    {
        color = BLUE1;
        arrow = Blue.arrow;
    }

    x *= 2;// 버퍼 출력이기 때문에 *2 해야함
    if (arrow == -1) //왼쪽을 바라보는
    {
        textcolor(WHITE, color); printxy(x + 6, y, "●");
        textcolor(color, color); printxy(x + 4, y + 1, "    ");
        textcolor(color, color); printxy(x + 2, y + 2, "        ");
        textcolor(YELLOW2, BLACK); printxy(x, y + 3, "    ◆      ");
        textcolor(BLACK, SKIN); printxy(x + 2, y + 4, "●  ●  ");
        textcolor(GRAY1, GRAY1); printxy(x + 2, y + 5, "  ");
        textcolor(ORANGE, SKIN); printxy(x + 4, y + 5, "ε  ");
        textcolor(GRAY1, GRAY1); printxy(x + 8, y + 5, "  ");

        textcolor(color, color); printxy(x + 1, y + 6, "   ");
        textcolor(GRAY1, GRAY1); printxy(x + 3, y + 6, "      ");
        textcolor(color, color); printxy(x + 9, y + 6, "  ");

        textcolor(color, color); printxy(x, y + 7, "    ");
        textcolor(GRAY1, GRAY1); printxy(x + 4, y + 7, "    ");
        textcolor(color, color); printxy(x + 8, y + 7, "    ");

        textcolor(SKIN, SKIN); printxy(x, y + 8, "  ");
        textcolor(color, color); printxy(x + 2, y + 8, "    ");
        textcolor(GRAY1, GRAY1); printxy(x + 6, y + 8, "  ");
        textcolor(color, color); printxy(x + 8, y + 8, "    ");

        textcolor(color, color); printxy(x + 2, y + 9, "        ");
        textcolor(SKIN, SKIN); printxy(x + 10, y + 9, "  ");
        textcolor(color, color); printxy(x + 2, y + 10, "        ");
        textcolor(color, color); printxy(x + 2, y + 11, "        ");
        textcolor(BROWN, BROWN); printxy(x + 2, y + 12, "        ");
        textcolor(BLACK, WHITE);
    }
    else if (arrow == 1) //오른쪽을 바라보는
    {
        textcolor(WHITE, color); printxy(x + 4, y, "●");
        textcolor(color, color); printxy(x + 4, y + 1, "    ");
        textcolor(color, color); printxy(x + 2, y + 2, "        ");
        textcolor(YELLOW2, BLACK); printxy(x, y + 3, "      ◆    ");
        textcolor(BLACK, SKIN); printxy(x + 2, y + 4, "  ●  ●");
        textcolor(GRAY1, GRAY1); printxy(x + 2, y + 5, "  ");
        textcolor(ORANGE, SKIN); printxy(x + 4, y + 5, "  ３");
        textcolor(GRAY1, GRAY1); printxy(x + 8, y + 5, "  ");

        textcolor(color, color); printxy(x + 1, y + 6, "   ");
        textcolor(GRAY1, GRAY1); printxy(x + 3, y + 6, "      ");
        textcolor(color, color); printxy(x + 9, y + 6, "  ");

        textcolor(color, color); printxy(x, y + 7, "    ");
        textcolor(GRAY1, GRAY1); printxy(x + 4, y + 7, "    ");
        textcolor(color, color); printxy(x + 8, y + 7, "    ");

        textcolor(color, color); printxy(x, y + 8, "    ");
        textcolor(GRAY1, GRAY1); printxy(x + 4, y + 8, "  ");
        textcolor(color, color); printxy(x + 6, y + 8, "    ");
        textcolor(SKIN, SKIN); printxy(x + 10, y + 8, "  ");

        textcolor(SKIN, SKIN); printxy(x, y + 9, "  ");
        textcolor(color, color); printxy(x + 2, y + 9, "        ");

        textcolor(color, color); printxy(x + 2, y + 10, "        ");
        textcolor(color, color); printxy(x + 2, y + 11, "        ");
        textcolor(BROWN, BROWN); printxy(x + 2, y + 12, "        ");
        textcolor(BLACK, WHITE);
    }
}
void Character_collider(int y, int x, int num) //캐릭터(RED or BLUE) 콜라이더 13x6
{
    int i, j;
    for (i = 0; i < 13; i++)
    {
        for (j = 0; j < 6; j++)
        {
            map[y + i][x + j] = num;
        }
    }
    //DrawMap(); //콜라이더 잘 생성되는지 테스트용    
}
void erasestar(int y, int x) //잔상, 콜라이더 제거 함수
{
    x *= 2;
    for (int i = 0; i < 13; i++)
        printxy(x, y + i, "            ");
    //캐릭터 콜라이더 삭제하는거 작성해야함
    //캐릭터 콜라이더 13x6
    x /= 2;
    int k, j;
    for (k = 0; k < 13; k++)
    {
        for (j = 0; j < 6; j++)
        {
            map[y + k][x + j] = 0;
        }
    }
    //DrawMap(); //콜라이더 잘 제거되는지 테스트용

}
void player1(unsigned char ch)//player_RED(1)
{
    static int jump_top;
    static int jump_mode = 0;//점프 상태 관리 변수 0:Not Jump, 1:올라가는중, 2:내려가는중
    int move_flag = 0;
    static unsigned char last_ch = 0;

    if (called[0] == 0) { // 처음 또는 Restart
        //oldx = 104, oldy = 150, newx = 104, newy = 150; //구조체 사용했으므로 버려짐
        Red.oldX = Red.newX = 17, Red.oldY = Red.newY = 155;
        Character_drow(Red.oldY, Red.oldX, 1);
        called[0] = 1;
        last_ch = 0;
        ch = 0;
    }
    if (keep_moving && ch == 0)
        ch = last_ch;
    last_ch = ch;

    //점프 모드 구현//////////////////////////////////////////////////////////////////
    if (ch == UP && jump_mode == 0) //지면에 있어야하고 up키가 눌러지면
    {
        jump_mode = 1;
        jump_top = Red.oldY - 15; //현 위치에 점프 높이<<<<<점프 높이 조절>>>>
    }
    if (jump_mode == 0) //점프중이 아닐때
    {
        //좌, 우로 움직임
        switch (ch)
        {
        case LEFT:
            if (map[Red.oldY][Red.oldX - 1] != 1 &&
                map[Red.oldY + 2][Red.oldX - 1] != 1 &&
                map[Red.oldY + 5][Red.oldX - 1] != 1 &&
                map[Red.oldY + 8][Red.oldX - 1] != 1 &&
                map[Red.oldY + 12][Red.oldX - 1] != 1
                )
                Red.newX = Red.oldX - 1;
            else {
                Red.newX = Red.oldX;
                last_ch = LEFT;
            }
            move_flag = 1;
            break;
        case RIGHT:
            if (map[Red.oldY][Red.oldX + 1 + 5] != 1 &&
                map[Red.oldY + 2][Red.oldX + 1 + 5] != 1 &&
                map[Red.oldY + 5][Red.oldX + 1 + 5] != 1 &&
                map[Red.oldY + 8][Red.oldX + 1 + 5] != 1 &&
                map[Red.oldY + 12][Red.oldX + 1 + 5] != 1
                )
                Red.newX = Red.oldX + 1;
            else {
                Red.newX = Red.oldX;
                last_ch = RIGHT;
            }
            move_flag = 1;
            break;
        case DOWN: //워프존 발동
            if (map[Red.oldY + 12 + 2][Red.oldX + 0] == 5 ||
                map[Red.oldY + 12 + 2][Red.oldX + 2] == 5 ||
                map[Red.oldY + 12 + 2][Red.oldX + 3] == 5 ||
                map[Red.oldY + 12 + 2][Red.oldX + 5] == 5) //발바닥 + 2아래 워프존일때
            {
                warp(1);
                move_flag = 1;
            }
            else if (map[Red.oldY + 12 + 2][Red.oldX + 0] == 3 || //점령 포인트라면
                map[Red.oldY + 12 + 2][Red.oldX + 2] == 3 ||
                map[Red.oldY + 12 + 2][Red.oldX + 3] == 3 ||
                map[Red.oldY + 12 + 2][Red.oldX + 5] == 3)
            {
                checkFlag(1);
            }
            break;
        }
        Red.move_ch = 0; //1한번만 움직임, 0계속 움직임
        //블럭 지형이 아니라면 떨어짐
        if (map[Red.oldY + 1 + 12][Red.oldX] != 1 &&
            map[Red.oldY + 1 + 12][Red.oldX + 2] != 1 &&
            map[Red.oldY + 1 + 12][Red.oldX + 3] != 1 &&
            map[Red.oldY + 1 + 12][Red.oldX + 5] != 1)
        {
            jump_mode = 2;
            //newx = oldx + 1; //중력 구현
            last_ch = DOWN;
            Red.move_ch = 0; //계속 떨어짐
        }

    }
    else if (jump_mode == 1) //올라가는 중
    {
        //점프 범위나, 천장까지 올라감
        if (Red.oldY > jump_top &&
            map[Red.oldY - 1][Red.oldX] != 1 && //전부다 할 필욘 없음 양 끝과 중간 정도만
            map[Red.oldY - 1][Red.oldX + 2] != 1 &&
            map[Red.oldY - 1][Red.oldX + 3] != 1 &&
            map[Red.oldY - 1][Red.oldX + 5] != 1) // charicter collider =! 1 (마지막은 배열의 정보)
        {
            switch (ch) { //점프 중 좌우 움직임 가능
            case LEFT:
                if (map[Red.oldY][Red.oldX - 1] != 1 &&
                    map[Red.oldY + 2][Red.oldX - 1] != 1 &&
                    map[Red.oldY + 5][Red.oldX - 1] != 1 &&
                    map[Red.oldY + 8][Red.oldX - 1] != 1 &&
                    map[Red.oldY + 12][Red.oldX - 1] != 1) {
                    Red.newX = Red.oldX - 1;
                }
                else {
                    Red.newX = Red.oldX;
                    last_ch = LEFT;
                }
                move_flag = 1;
                break;
            case RIGHT:
                if (map[Red.oldY][Red.oldX + 1 + 5] != 1 &&
                    map[Red.oldY + 2][Red.oldX + 1 + 5] != 1 &&
                    map[Red.oldY + 5][Red.oldX + 1 + 5] != 1 &&
                    map[Red.oldY + 8][Red.oldX + 1 + 5] != 1 &&
                    map[Red.oldY + 12][Red.oldX + 1 + 5] != 1) {
                    Red.newX = Red.oldX + 1;
                }
                else {
                    Red.newX = Red.oldX;
                    last_ch = RIGHT;
                }
                move_flag = 1;
                break;
            }
            ////
            Red.newY = Red.oldY - 1;// 위로 이동
            move_flag = 1;// 출력할거얌
        }
        else
        {
            //newx = oldx + 1; 
            last_ch = DOWN;
            jump_mode = 2;
        }

        Red.move_ch = 0; // 계속 움직임
    }
    else if (jump_mode == 2) // 내려가는 중
    {
        //바닥을 만날때 까지 내려감
        //좌우 허용
        switch (ch)
        {
        case LEFT:
            if (map[Red.oldY][Red.oldX - 1] != 1 &&
                map[Red.oldY + 2][Red.oldX - 1] != 1 &&
                map[Red.oldY + 5][Red.oldX - 1] != 1 &&
                map[Red.oldY + 8][Red.oldX - 1] != 1 &&
                map[Red.oldY + 12][Red.oldX - 1] != 1) {
                Red.newX = Red.oldX - 1; //2로 설정하면 떨어질때 한번더 왼쪽 느르면 더 빨라짐(재밌음) 근데 버그있음
            }
            else {
                Red.newX = Red.oldX;
                last_ch = LEFT;
            }
            move_flag = 1;
            break;
        case RIGHT:
            if (map[Red.oldY][Red.oldX + 1 + 5] != 1 &&
                map[Red.oldY + 2][Red.oldX + 1 + 5] != 1 &&
                map[Red.oldY + 5][Red.oldX + 1 + 5] != 1 &&
                map[Red.oldY + 8][Red.oldX + 1 + 5] != 1 &&
                map[Red.oldY + 12][Red.oldX + 1 + 5] != 1) {
                Red.newX = Red.oldX + 1; //떨어질때 한번더 오른쪽 느르면 더 빨라짐(재밌음)
            }
            else {
                Red.newX = Red.oldX;
                last_ch = RIGHT;
            }
            move_flag = 1;
            break;
        }
        if (map[Red.oldY + 1 + 12][Red.oldX] != 1 &&
            map[Red.oldY + 1 + 12][Red.oldX + 2] != 1 &&
            map[Red.oldY + 1 + 12][Red.oldX + 3] != 1 &&
            map[Red.oldY + 1 + 12][Red.oldX + 5] != 1)
        {
            Red.newY = Red.oldY + 1;
            //last_ch = DOWN; //이거 빼도 될까?
            move_flag = 1;// 출력할거얌
        }
        else {
            //newx = oldx;
            jump_mode = 0;

        }
        Red.move_ch = 0; // 계속 움직임
    }
    /////////////////////////////////////////////////////////////////////////////////
    if (move_flag) {
        hit_item(Red.newY, Red.newX, 99); //아이템 정보(hit정보 함수 내부 판단)
        erasestar(Red.oldY, Red.oldX); // 마지막 위치의 * 를 지우고
        textcolor(WHITE, BLACK);
        Character_drow(Red.newY, Red.newX, 1);
        Character_collider(Red.newY, Red.newX, 99); //99:red 콜라이더 생성
        textcolor(BLACK, WHITE);
        Red.oldX = Red.newX; // 마지막 위치를 기억한다.
        Red.oldY = Red.newY;
    }
}
void player2(unsigned char ch)//player_BLUE(2)
{
    static int jump_top;
    static int jump_mode = 0;//점프 상태 관리 변수 0:Not Jump, 1:올라가는중, 2:내려가는중
    int move_flag = 0;
    static unsigned char last_ch = 0;

    if (called2[0] == 0) { // 처음 또는 Restart
        //oldx = 104, oldy = 150, newx = 104, newy = 150; //구조체 사용했으므로 버려짐
        Blue.oldX = Blue.newX = 277, Blue.oldY = Blue.newY = 155;
        Character_drow(Blue.oldY, Blue.oldX, 2);
        called2[0] = 1;
        last_ch = 0;
        ch = 0;
    }
    if (keep_moving && ch == 0)
        ch = last_ch;
    last_ch = ch;

    //점프 모드 구현//////////////////////////////////////////////////////////////////
    if (ch == UP2 && jump_mode == 0) //지면에 있어야하고 up키가 눌러지면
    {
        jump_mode = 1;
        jump_top = Blue.oldY - 15; //현 위치에 점프 높이<<<<<점프 높이 조절>>>>
    }
    if (jump_mode == 0) //점프중이 아닐때
    {
        //좌, 우로 움직임
        switch (ch)
        {
        case LEFT2:
            if (map[Blue.oldY][Blue.oldX - 1] != 1 &&
                map[Blue.oldY + 2][Blue.oldX - 1] != 1 &&
                map[Blue.oldY + 5][Blue.oldX - 1] != 1 &&
                map[Blue.oldY + 8][Blue.oldX - 1] != 1 &&
                map[Blue.oldY + 12][Blue.oldX - 1] != 1
                )
                Blue.newX = Blue.oldX - 1;
            else {
                Blue.newX = Blue.oldX;
                last_ch = LEFT2;
            }
            move_flag = 1;
            break;
        case RIGHT2:
            if (map[Blue.oldY][Blue.oldX + 1 + 5] != 1 &&
                map[Blue.oldY + 2][Blue.oldX + 1 + 5] != 1 &&
                map[Blue.oldY + 5][Blue.oldX + 1 + 5] != 1 &&
                map[Blue.oldY + 8][Blue.oldX + 1 + 5] != 1 &&
                map[Blue.oldY + 12][Blue.oldX + 1 + 5] != 1
                )
                Blue.newX = Blue.oldX + 1;
            else {
                Blue.newX = Blue.oldX;
                last_ch = RIGHT2;
            }
            move_flag = 1;
            break;
        case DOWN2: //워프존 발동
            if (map[Blue.oldY + 12 + 2][Blue.oldX + 0] == 5 ||
                map[Blue.oldY + 12 + 2][Blue.oldX + 2] == 5 ||
                map[Blue.oldY + 12 + 2][Blue.oldX + 3] == 5 ||
                map[Blue.oldY + 12 + 2][Blue.oldX + 5] == 5) //발바닥 + 2아래가 워프존일때
            {
                warp(2);
                move_flag = 1;
            }
            else if (map[Blue.oldY + 12 + 2][Blue.oldX + 0] == 3 || //점령 포인트라면
                map[Blue.oldY + 12 + 2][Blue.oldX + 2] == 3 ||
                map[Blue.oldY + 12 + 2][Blue.oldX + 3] == 3 ||
                map[Blue.oldY + 12 + 2][Blue.oldX + 5] == 3)
            {
                checkFlag(2);
            }
            break;
        }
        Blue.move_ch = 0; //1한번만 움직임, 0계속 움직임
        //블럭 지형이 아니라면 떨어짐
        if (map[Blue.oldY + 1 + 12][Blue.oldX] != 1 &&
            map[Blue.oldY + 1 + 12][Blue.oldX + 2] != 1 &&
            map[Blue.oldY + 1 + 12][Blue.oldX + 3] != 1 &&
            map[Blue.oldY + 1 + 12][Blue.oldX + 5] != 1)
        {
            jump_mode = 2;
            //newx = oldx + 1; //중력 구현
            last_ch = DOWN2;
            Blue.move_ch = 0; //계속 떨어짐
        }

    }
    else if (jump_mode == 1) //올라가는 중
    {
        //점프 범위나, 천장까지 올라감
        if (Blue.oldY > jump_top &&
            map[Blue.oldY - 1][Blue.oldX] != 1 && //전부다 할 필욘 없음 양 끝과 중간 정도만
            map[Blue.oldY - 1][Blue.oldX + 2] != 1 &&
            map[Blue.oldY - 1][Blue.oldX + 3] != 1 &&
            map[Blue.oldY - 1][Blue.oldX + 5] != 1) // charicter collider =! 1 (마지막은 배열의 정보)
        {
            switch (ch) { //점프 중 좌우 움직임 가능
            case LEFT2:
                if (map[Blue.oldY][Blue.oldX - 1] != 1 &&
                    map[Blue.oldY + 2][Blue.oldX - 1] != 1 &&
                    map[Blue.oldY + 5][Blue.oldX - 1] != 1 &&
                    map[Blue.oldY + 8][Blue.oldX - 1] != 1 &&
                    map[Blue.oldY + 12][Blue.oldX - 1] != 1) {
                    Blue.newX = Blue.oldX - 1;
                }
                else {
                    Blue.newX = Blue.oldX;
                    last_ch = LEFT2;
                }
                move_flag = 1;
                break;
            case RIGHT2:
                if (map[Blue.oldY][Blue.oldX + 1 + 5] != 1 &&
                    map[Blue.oldY + 2][Blue.oldX + 1 + 5] != 1 &&
                    map[Blue.oldY + 5][Blue.oldX + 1 + 5] != 1 &&
                    map[Blue.oldY + 8][Blue.oldX + 1 + 5] != 1 &&
                    map[Blue.oldY + 12][Blue.oldX + 1 + 5] != 1) {
                    Blue.newX = Blue.oldX + 1;
                }
                else {
                    Blue.newX = Blue.oldX;
                    last_ch = RIGHT2;
                }
                move_flag = 1;
                break;
            }
            ////
            Blue.newY = Blue.oldY - 1;// 위로 이동
            move_flag = 1;// 출력할거얌
        }
        else
        {
            //newx = oldx + 1; 
            last_ch = DOWN2;
            jump_mode = 2;
        }

        Blue.move_ch = 0; // 계속 움직임
    }
    else if (jump_mode == 2) // 내려가는 중
    {
        //바닥을 만날때 까지 내려감
        //좌우 허용
        switch (ch)
        {
        case LEFT2:
            if (map[Blue.oldY][Blue.oldX - 1] != 1 &&
                map[Blue.oldY + 2][Blue.oldX - 1] != 1 &&
                map[Blue.oldY + 5][Blue.oldX - 1] != 1 &&
                map[Blue.oldY + 8][Blue.oldX - 1] != 1 &&
                map[Blue.oldY + 12][Blue.oldX - 1] != 1) {
                Blue.newX = Blue.oldX - 1; //2로 설정하면 떨어질때 한번더 왼쪽 느르면 더 빨라짐(재밌음) 근데 버그있음
            }
            else {
                Blue.newX = Blue.oldX;
                last_ch = LEFT2;
            }
            move_flag = 1;
            break;
        case RIGHT2:
            if (map[Blue.oldY][Blue.oldX + 1 + 5] != 1 &&
                map[Blue.oldY + 2][Blue.oldX + 1 + 5] != 1 &&
                map[Blue.oldY + 5][Blue.oldX + 1 + 5] != 1 &&
                map[Blue.oldY + 8][Blue.oldX + 1 + 5] != 1 &&
                map[Blue.oldY + 12][Blue.oldX + 1 + 5] != 1) {
                Blue.newX = Blue.oldX + 1; //떨어질때 한번더 오른쪽 느르면 더 빨라짐(재밌음)
            }
            else {
                Blue.newX = Blue.oldX;
                last_ch = RIGHT2;
            }
            move_flag = 1;
            break;
        }
        if (map[Blue.oldY + 1 + 12][Blue.oldX] != 1 &&
            map[Blue.oldY + 1 + 12][Blue.oldX + 2] != 1 &&
            map[Blue.oldY + 1 + 12][Blue.oldX + 3] != 1 &&
            map[Blue.oldY + 1 + 12][Blue.oldX + 5] != 1)
        {
            Blue.newY = Blue.oldY + 1;
            //last_ch = DOWN; //이거 빼도 될까?
            move_flag = 1;// 출력할거얌
        }
        else {
            //newx = oldx;
            jump_mode = 0;

        }
        Blue.move_ch = 0; // 계속 움직임
    }
    /////////////////////////////////////////////////////////////////////////////////
    if (move_flag) {
        hit_item(Blue.newY, Blue.newX, 98);
        erasestar(Blue.oldY, Blue.oldX); // 마지막 위치의 * 를 지우고
        textcolor(WHITE, BLACK);
        Character_drow(Blue.newY, Blue.newX, 2);
        Character_collider(Blue.newY, Blue.newX, 98); //99:red 콜라이더 생성
        textcolor(BLACK, WHITE);
        Blue.oldX = Blue.newX; // 마지막 위치를 기억한다.
        Blue.oldY = Blue.newY;
    }
}
void init_hp(int player)
{
    int x = 0, y = 11; //출력 좌표
    int i;
    if (player == 1) //p1의 체력 정보
    {
        Red.hp = MAXHP; //풀피 회복
        x = 0;
    }
    else if (player == 2) //p2의 체력 정보
    {
        Blue.hp = MAXHP; //풀피 회복
        x = 400;
    }
    for (i = 0; i < 200; i++)
    {
        textcolor(GREEN1, GREEN1); printxy(x + i, y + 0, " ");
        textcolor(GREEN1, GREEN1); printxy(x + i, y + 1, " ");
        textcolor(GREEN1, GREEN1); printxy(x + i, y + 2, " ");
        textcolor(GREEN1, GREEN1); printxy(x + i, y + 3, " ");
        textcolor(GREEN1, GREEN1); printxy(x + i, y + 4, " ");
        textcolor(GREEN1, GREEN1); printxy(x + i, y + 5, " ");
        textcolor(GREEN1, GREEN1); printxy(x + i, y + 6, " ");
        textcolor(BLACK, WHITE);
    }
}
void print_hp(int player) //플레이어 남은 체력 출력
{   //초록바를 초기화 시키기 보단 회색바을 깎는게 나을듯 너무 출력량이 많아져 프로그램 동작 느려짐 
    //위 방법 사용시 주의할점 >> @@리스폰or힐 할때마다 체력바 텍스트배경 초록으로 변경해주기@@
    int x, y = 11; //출력 좌표
    int i;
    if (player == 1 && Red.hp >= 0) //p1의 체력 정보
    {
        x = 195;
        for (i = 0; i < MAXHP - Red.hp; i++)
        {
            textcolor(GREEN1, GRAY1); printxy(x, y + 0, "     ");
            textcolor(GREEN1, GRAY1); printxy(x, y + 1, "     ");
            textcolor(GREEN1, GRAY1); printxy(x, y + 2, "     ");
            textcolor(GREEN1, GRAY1); printxy(x, y + 3, "     ");
            textcolor(GREEN1, GRAY1); printxy(x, y + 4, "     ");
            textcolor(GREEN1, GRAY1); printxy(x, y + 5, "     ");
            textcolor(GREEN1, GRAY1); printxy(x, y + 6, "     ");
            x -= 5;
        }
    }
    else if (player == 2 && Blue.hp >= 0) //p2의 체력 정보
    {
        x = 400;
        for (i = 0; i < MAXHP - Blue.hp; i++)
        {
            textcolor(GREEN1, GRAY1); printxy(x, y + 0, "     ");
            textcolor(GREEN1, GRAY1); printxy(x, y + 1, "     ");
            textcolor(GREEN1, GRAY1); printxy(x, y + 2, "     ");
            textcolor(GREEN1, GRAY1); printxy(x, y + 3, "     ");
            textcolor(GREEN1, GRAY1); printxy(x, y + 4, "     ");
            textcolor(GREEN1, GRAY1); printxy(x, y + 5, "     ");
            textcolor(GREEN1, GRAY1); printxy(x, y + 6, "     ");
            x += 5;
        }
    }
    textcolor(BLACK, WHITE);
}
void print_skipow(int player) //스킬게이지 충전량 출력
{
    int x = 0, y = 2;
    int n = 0;
    if (player == 1)
    {
        x = 122;
        n = Red.skiPow;
    }
    else if (player == 2)
    {
        x = 522;
        n = Blue.skiPow;
    }

    switch (n)
    {
    case 0:
        textcolor(BLACK, BLACK);
        printxy(x, 2, "                      ");
        printxy(x, 3, "                      ");
        printxy(x, 4, "                        ");
        printxy(x, 5, "                        ");
        printxy(x, 6, "                        ");
        printxy(x, 7, "                      ");
        printxy(x, 8, "                      ");

        textcolor(GREEN1, WHITE);
        printxy(x + 1, 3, "      ");
        printxy(x + 1, 4, "      ");
        printxy(x + 1, 5, "      ");
        printxy(x + 1, 6, "      ");
        printxy(x + 1, 7, "      ");
        //textcolor(GREEN1, GREEN1);
        printxy(x + 8, 3, "      ");
        printxy(x + 8, 4, "      ");
        printxy(x + 8, 5, "      ");
        printxy(x + 8, 6, "      ");
        printxy(x + 8, 7, "      ");
        //textcolor(GREEN1, GREEN1);
        printxy(x + 15, 3, "      ");
        printxy(x + 15, 4, "      ");
        printxy(x + 15, 5, "        ");
        printxy(x + 15, 6, "      ");
        printxy(x + 15, 7, "      ");
        break;
    case 1:
        textcolor(BLACK, BLACK);
        printxy(x, 2, "                      ");
        printxy(x, 3, "                      ");
        printxy(x, 4, "                        ");
        printxy(x, 5, "                        ");
        printxy(x, 6, "                        ");
        printxy(x, 7, "                      ");
        printxy(x, 8, "                      ");

        textcolor(GREEN1, GREEN1);
        printxy(x + 1, 3, "      ");
        printxy(x + 1, 4, "      ");
        printxy(x + 1, 5, "      ");
        printxy(x + 1, 6, "      ");
        printxy(x + 1, 7, "      ");
        textcolor(GREEN1, WHITE);
        printxy(x + 8, 3, "      ");
        printxy(x + 8, 4, "      ");
        printxy(x + 8, 5, "      ");
        printxy(x + 8, 6, "      ");
        printxy(x + 8, 7, "      ");
        textcolor(GREEN1, WHITE);
        printxy(x + 15, 3, "      ");
        printxy(x + 15, 4, "      ");
        printxy(x + 15, 5, "        ");
        printxy(x + 15, 6, "      ");
        printxy(x + 15, 7, "      ");
        break;
    case 2:
        textcolor(BLACK, BLACK);
        printxy(x, 2, "                      ");
        printxy(x, 3, "                      ");
        printxy(x, 4, "                        ");
        printxy(x, 5, "                        ");
        printxy(x, 6, "                        ");
        printxy(x, 7, "                      ");
        printxy(x, 8, "                      ");

        textcolor(GREEN1, GREEN1);
        printxy(x + 1, 3, "      ");
        printxy(x + 1, 4, "      ");
        printxy(x + 1, 5, "      ");
        printxy(x + 1, 6, "      ");
        printxy(x + 1, 7, "      ");
        textcolor(GREEN1, GREEN1);
        printxy(x + 8, 3, "      ");
        printxy(x + 8, 4, "      ");
        printxy(x + 8, 5, "      ");
        printxy(x + 8, 6, "      ");
        printxy(x + 8, 7, "      ");
        textcolor(GREEN1, WHITE);
        printxy(x + 15, 3, "      ");
        printxy(x + 15, 4, "      ");
        printxy(x + 15, 5, "        ");
        printxy(x + 15, 6, "      ");
        printxy(x + 15, 7, "      ");
        break;
    default:
        textcolor(BLACK, BLACK);
        printxy(x, 2, "                      ");
        printxy(x, 3, "                      ");
        printxy(x, 4, "                        ");
        printxy(x, 5, "                        ");
        printxy(x, 6, "                        ");
        printxy(x, 7, "                      ");
        printxy(x, 8, "                      ");

        textcolor(GREEN1, GREEN1);
        printxy(x + 1, 3, "      ");
        printxy(x + 1, 4, "      ");
        printxy(x + 1, 5, "      ");
        printxy(x + 1, 6, "      ");
        printxy(x + 1, 7, "      ");
        textcolor(GREEN1, GREEN1);
        printxy(x + 8, 3, "      ");
        printxy(x + 8, 4, "      ");
        printxy(x + 8, 5, "      ");
        printxy(x + 8, 6, "      ");
        printxy(x + 8, 7, "      ");
        textcolor(GREEN1, GREEN1);
        printxy(x + 15, 3, "      ");
        printxy(x + 15, 4, "      ");
        printxy(x + 15, 5, "        ");
        printxy(x + 15, 6, "      ");
        printxy(x + 15, 7, "      ");

        //번개 그리기 (완충)
        textcolor(YELLOW2, YELLOW2);
        printxy(x + 10, 3, " ");
        printxy(x + 9, 4, "   ");
        textcolor(YELLOW2, GREEN1); printxy(x + 12, 4, " ");
        textcolor(YELLOW2, YELLOW2); printxy(x + 13, 4, "  ");
        printxy(x + 8, 5, "      ");
        printxy(x + 7, 6, "  ");
        textcolor(YELLOW2, GREEN1); printxy(x + 9, 6, " ");
        textcolor(YELLOW2, YELLOW2); printxy(x + 10, 6, "   ");
        printxy(x + 11, 7, " ");
        break;
    }
    textcolor(BLACK, WHITE);
}

//발사체 관련 함수
void shot_red(int n) //red의 발사체 (파이어볼) @@@@@@@@@@@@@@@@콜라이더랑 디자인 추가해야함
{
    int x = Red_ball[n].newX;
    int y = Red_ball[n].newY; //수직은 변동 없음
    int ox = Red_ball[n].oldX;
    int oy = Red_ball[n].oldY; //수직은 변동 없음

    if (map[y + 0][x] != 1 && map[y + 0][x + 3] != 1 && map[y + 0][x + 7] != 1 &&
        map[y + 1][x] != 1 && map[y + 1][x + 3] != 1 && map[y + 1][x + 7] != 1 &&
        map[y + 2][x] != 1 && map[y + 2][x + 3] != 1 && map[y + 2][x + 7] != 1 &&
        map[y + 3][x] != 1 && map[y + 3][x + 3] != 1 && map[y + 3][x + 7] != 1)
    {
        Red_ball[n].ball_cheak = 1; //다음 벽이 아니면 O

        Red_ball[n].oldX = Red_ball[n].newX;
        Red_ball[n].newX = Red_ball[n].oldX + (Red_ball[n].arrow * 5);
        //(5.30. 교수님께서 쓰지말라하신 부분)
        //발사체 3칸씩 이동, 캐릭터의 크기는 6칸이기 때문에 상관없음 어떤위치에서도 적중 가능
    }
    else
    {
        Red_ball[n].ball_cheak = 0; //다음 벽이면 X (통과못함)

        Red_ball[n].newX = NULL; //0으로 숨김
    }

    if (map[y + 0][x + 4] == 98 ||
        map[y + 1][x + 4] == 98 ||
        map[y + 2][x + 4] == 98 ||
        map[y + 3][x + 4] == 98) //발사체의 중간 부분이 적군과 만나면
    {
        Blue.hp--; //블루 적중시 체력 1 감소
        print_hp(2);
        Red_ball[n].ball_cheak = 0; //적중하면 출력 x
        Red_ball[n].newX = NULL; //0으로 숨김
    }

    //잔상 지운다
    if (map[oy + 0][ox] != 1 && map[oy + 0][ox + 3] != 1 && map[oy + 0][ox + 7] != 1 &&
        map[oy + 1][ox] != 1 && map[oy + 1][ox + 3] != 1 && map[oy + 1][ox + 7] != 1 &&
        map[oy + 2][ox] != 1 && map[oy + 2][ox + 3] != 1 && map[oy + 2][ox + 7] != 1 &&
        map[oy + 3][ox] != 1 && map[oy + 3][ox + 3] != 1 && map[oy + 3][ox + 7] != 1)
    {
        for (int i = oy; i < oy + 4; i++)
        {
            printxy(ox * 2, i, "                ");
        }
    }

    //발사체 그린다
    if (Red_ball[n].ball_cheak) //체크된 경우
    {
        x *= 2;//배열아님 콘솔 출력 x2
        if (Red_ball[n].arrow == 1)//오른쪽 방향 파이어볼
        {
            textcolor(RED1, RED1); printxy(x + 6, y, "        ");
            //
            textcolor(RED1, RED1); printxy(x + 2, y + 1, "    ");
            textcolor(ORANGE, ORANGE); printxy(x + 6, y + 1, "      ");
            textcolor(YELLOW2, YELLOW2); printxy(x + 12, y + 1, "  ");
            textcolor(RED1, RED1); printxy(x + 14, y + 1, "  ");
            //
            textcolor(RED1, RED1); printxy(x, y + 2, "    ");
            textcolor(ORANGE, ORANGE); printxy(x + 4, y + 2, "    ");
            textcolor(YELLOW2, YELLOW2); printxy(x + 8, y + 2, "    ");
            textcolor(RED1, RED1); printxy(x + 12, y + 2, "    ");
            //
            textcolor(RED1, RED1); printxy(x + 6, y + 3, "        ");
        }
        else
        {
            textcolor(RED1, RED1); printxy(x + 2, y, "        ");
            //
            textcolor(RED1, RED1); printxy(x, y + 1, "    ");
            textcolor(YELLOW2, YELLOW2); printxy(x + 4, y + 1, "    ");
            textcolor(ORANGE, ORANGE); printxy(x + 8, y + 1, "    ");
            textcolor(RED1, RED1); printxy(x + 12, y + 1, "    ");
            //
            textcolor(RED1, RED1); printxy(x, y + 2, "  ");
            textcolor(YELLOW2, YELLOW2); printxy(x + 2, y + 2, "  ");
            textcolor(ORANGE, ORANGE); printxy(x + 4, y + 2, "      ");
            textcolor(RED1, RED1); printxy(x + 10, y + 2, "    ");
            //
            textcolor(RED1, RED1); printxy(x + 2, y + 3, "        ");
        }
        textcolor(BLACK, WHITE);
        x /= 2;
    }
}
void shot_blue(int n)
{
    int x = Blue_ball[n].newX;
    int y = Blue_ball[n].newY; //수직은 변동 없음
    int ox = Blue_ball[n].oldX;
    int oy = Blue_ball[n].oldY; //수직은 변동 없음

    if (map[y + 0][x] != 1 && map[y + 0][x + 3] != 1 && map[y + 0][x + 7] != 1 &&
        map[y + 1][x] != 1 && map[y + 1][x + 3] != 1 && map[y + 1][x + 7] != 1 &&
        map[y + 2][x] != 1 && map[y + 2][x + 3] != 1 && map[y + 2][x + 7] != 1 &&
        map[y + 3][x] != 1 && map[y + 3][x + 3] != 1 && map[y + 3][x + 7] != 1)
    {
        Blue_ball[n].ball_cheak = 1; //다음 벽이 아니면 O

        Blue_ball[n].oldX = Blue_ball[n].newX;
        Blue_ball[n].newX = Blue_ball[n].oldX + (Blue_ball[n].arrow * 5);
        //(5.30. 교수님께서 쓰지말라하신 부분)
        //발사체 3칸씩 이동, 캐릭터의 크기는 6칸이기 때문에 상관없음 어떤위치에서도 적중 가능
    }
    else
    {
        Blue_ball[n].ball_cheak = 0; //다음 벽이면 X (통과못함)

        Blue_ball[n].newX = NULL; //0으로 숨김
    }

    if (map[y + 0][x + 4] == 99 ||
        map[y + 1][x + 4] == 99 ||
        map[y + 2][x + 4] == 99 ||
        map[y + 3][x + 4] == 99) //발사체의 중간 부분이 적군과 만나면
    {
        Red.hp--; //레드 적중시 체력 1 감소
        print_hp(1);
        Blue_ball[n].ball_cheak = 0; //적중하면 출력 x
        Blue_ball[n].newX = NULL; //0으로 숨김
    }

    //잔상 지운다
    if (map[oy + 0][ox] != 1 && map[oy + 0][ox + 3] != 1 && map[oy + 0][ox + 7] != 1 &&
        map[oy + 1][ox] != 1 && map[oy + 1][ox + 3] != 1 && map[oy + 1][ox + 7] != 1 &&
        map[oy + 2][ox] != 1 && map[oy + 2][ox + 3] != 1 && map[oy + 2][ox + 7] != 1 &&
        map[oy + 3][ox] != 1 && map[oy + 3][ox + 3] != 1 && map[oy + 3][ox + 7] != 1)
    {
        for (int i = oy; i < oy + 4; i++)
        {
            printxy(ox * 2, i, "                ");
        }
    }


    //발사체 그린다
    if (Blue_ball[n].ball_cheak) //체크되면
    {
        x *= 2;//배열아님 콘솔 출력 x2
        if (Blue_ball[n].arrow == 1)//오른쪽 방향 파이어볼
        {
            textcolor(BLUE1, BLUE1); printxy(x + 6, y, "        ");
            //
            textcolor(BLUE1, BLUE1); printxy(x + 2, y + 1, "    ");
            textcolor(CYAN1, CYAN1); printxy(x + 6, y + 1, "      ");
            textcolor(CYAN2, CYAN2); printxy(x + 12, y + 1, "  ");
            textcolor(BLUE1, BLUE1); printxy(x + 14, y + 1, "  ");
            //
            textcolor(BLUE1, BLUE1); printxy(x, y + 2, "    ");
            textcolor(CYAN1, CYAN1); printxy(x + 4, y + 2, "    ");
            textcolor(CYAN2, CYAN2); printxy(x + 8, y + 2, "    ");
            textcolor(BLUE1, BLUE1); printxy(x + 12, y + 2, "    ");
            //
            textcolor(BLUE1, BLUE1); printxy(x + 6, y + 3, "        ");
        }
        else
        {
            textcolor(BLUE1, BLUE1); printxy(x + 2, y, "        ");
            //
            textcolor(BLUE1, BLUE1); printxy(x, y + 1, "    ");
            textcolor(CYAN2, CYAN2); printxy(x + 4, y + 1, "    ");
            textcolor(CYAN1, CYAN1); printxy(x + 8, y + 1, "    ");
            textcolor(BLUE1, BLUE1); printxy(x + 12, y + 1, "    ");
            //
            textcolor(BLUE1, BLUE1); printxy(x, y + 2, "  ");
            textcolor(CYAN2, CYAN2); printxy(x + 2, y + 2, "  ");
            textcolor(CYAN1, CYAN1); printxy(x + 4, y + 2, "      ");
            textcolor(BLUE1, BLUE1); printxy(x + 10, y + 2, "    ");
            //
            textcolor(BLUE1, BLUE1); printxy(x + 2, y + 3, "        ");
        }
        textcolor(BLACK, WHITE);
        x /= 2;
    }
}
void shot_count(int player) //남은 파이어볼 수 출력(매개변수: 1 or 2)
{
    int a, b; //자리수 
    int x, y = 2; // 출력 좌표 //RED:52 BLUE:454 , y:2-8
    int n = 0, i;

    if (player == 1)
    {
        a = Red.shots / 10; //10의 자리
        b = Red.shots % 10; //1의 자리
        x = 53;
    }
    else
    {
        a = Blue.shots / 10; //10의 자리
        b = Blue.shots % 10; //1의 자리
        x = 453;
    }

    i = x;
    textcolor(YELLOW2, BLACK);
    for (; x < i + 11; x += 10)
    {
        if (x == i)
        {
            n = a;
        }
        else if (x == i + 10)
        {
            n = b;
        }
        switch (n)
        {
        case 0:
            printxy(x, y + 0, "  #####   ");
            printxy(x, y + 1, " ##   ##  ");
            printxy(x, y + 2, "##   # ## ");
            printxy(x, y + 3, "##  #  ## ");
            printxy(x, y + 4, "## #   ## ");
            printxy(x, y + 5, " ##   ##  ");
            printxy(x, y + 6, "  #####   ");
            break;
        case 1:
            printxy(x, y + 0, "    ##    ");
            printxy(x, y + 1, "  ####    ");
            printxy(x, y + 2, "    ##    ");
            printxy(x, y + 3, "    ##    ");
            printxy(x, y + 4, "    ##    ");
            printxy(x, y + 5, "    ##    ");
            printxy(x, y + 6, "  ######  ");
            break;
        case 2:
            printxy(x, y + 0, " #######  ");
            printxy(x, y + 1, "##     ## ");
            printxy(x, y + 2, "##     ## ");
            printxy(x, y + 3, "     ###  ");
            printxy(x, y + 4, "   ##     ");
            printxy(x, y + 5, " ##       ");
            printxy(x, y + 6, "######### ");
            break;
        case 3:
            printxy(x, y + 0, " #######  ");
            printxy(x, y + 1, "##     ## ");
            printxy(x, y + 2, "       ## ");
            printxy(x, y + 3, "  ######  ");
            printxy(x, y + 4, "       ## ");
            printxy(x, y + 5, "##     ## ");
            printxy(x, y + 6, " #######  ");
            break;
        case 4:
            printxy(x, y + 0, "    ####  ");
            printxy(x, y + 1, "   ## ##  ");
            printxy(x, y + 2, "  ##  ##  ");
            printxy(x, y + 3, " ##   ##  ");
            printxy(x, y + 4, "######### ");
            printxy(x, y + 5, "      ##  ");
            printxy(x, y + 6, "      ##  ");
            break;
        case 5:
            printxy(x, y + 0, "########  ");
            printxy(x, y + 1, "##        ");
            printxy(x, y + 2, "##        ");
            printxy(x, y + 3, "########  ");
            printxy(x, y + 4, "       ## ");
            printxy(x, y + 5, "##     ## ");
            printxy(x, y + 6, " #######  ");
            break;
        case 6:
            printxy(x, y + 0, " #######  ");
            printxy(x, y + 1, "##     ## ");
            printxy(x, y + 2, "##        ");
            printxy(x, y + 3, "########  ");
            printxy(x, y + 4, "##     ## ");
            printxy(x, y + 5, "##     ## ");
            printxy(x, y + 6, " ######   ");
            break;
        case 7:
            printxy(x, y + 0, " ######## ");
            printxy(x, y + 1, " ##    ## ");
            printxy(x, y + 2, "      ##  ");
            printxy(x, y + 3, "     ##   ");
            printxy(x, y + 4, "    ##    ");
            printxy(x, y + 5, "    ##    ");
            printxy(x, y + 6, "    ##    ");
            break;
        case 8:
            printxy(x, y + 0, " #######  ");
            printxy(x, y + 1, "##     ## ");
            printxy(x, y + 2, "##     ## ");
            printxy(x, y + 3, " #######  ");
            printxy(x, y + 4, "##     ## ");
            printxy(x, y + 5, "##     ## ");
            printxy(x, y + 6, " #######  ");
            break;
        case 9:
            printxy(x, y + 0, " #######  ");
            printxy(x, y + 1, "##     ## ");
            printxy(x, y + 2, "##     ## ");
            printxy(x, y + 3, " ######## ");
            printxy(x, y + 4, "       ## ");
            printxy(x, y + 5, "##     ## ");
            printxy(x, y + 6, " #######  ");
            break;
        }
    }
    textcolor(BLACK, WHITE);
}

//아이템 관련 함수
void drow_item(int y, int x, int ITnum) //아이템 그리기 7x7
{
    x *= 2; //x는 배열 정보기 때문(*2)
    switch (ITnum)
    {
    case 6:
        textcolor(WHITE, WHITE); printxy(x, y - 1, "              ");  //블루 포션(마나 포션)

        textcolor(WHITE, WHITE); printxy(x + 0, y + 0, "   ");
        textcolor(WHITE, BLACK); printxy(x + 3, y + 0, "   ");
        textcolor(WHITE, BROWN); printxy(x + 6, y + 0, "  ");
        textcolor(WHITE, BLACK); printxy(x + 8, y + 0, "   ");
        textcolor(WHITE, WHITE); printxy(x + 11, y + 0, "   ");

        textcolor(WHITE, WHITE); printxy(x + 0, y + 1, "    ");
        textcolor(WHITE, BLACK); printxy(x + 4, y + 1, "  ");
        textcolor(WHITE, WHITE); printxy(x + 6, y + 1, "  ");
        textcolor(WHITE, BLACK); printxy(x + 8, y + 1, "  ");
        textcolor(WHITE, WHITE); printxy(x + 10, y + 1, "    ");

        textcolor(WHITE, WHITE); printxy(x + 0, y + 2, "    ");
        textcolor(WHITE, BLACK); printxy(x + 4, y + 2, "  ");
        textcolor(WHITE, BLUE1); printxy(x + 6, y + 2, "  ");
        textcolor(WHITE, BLACK); printxy(x + 8, y + 2, "  ");
        textcolor(WHITE, WHITE); printxy(x + 10, y + 2, "    ");

        textcolor(WHITE, WHITE); printxy(x + 0, y + 3, "  ");
        textcolor(WHITE, BLACK); printxy(x + 2, y + 3, "  ");
        textcolor(WHITE, BLUE1); printxy(x + 4, y + 3, "      ");
        textcolor(WHITE, BLACK); printxy(x + 10, y + 3, "  ");
        textcolor(WHITE, WHITE); printxy(x + 12, y + 3, "  ");

        textcolor(WHITE, BLACK); printxy(x + 0, y + 4, "  ");
        textcolor(WHITE, BLUE1); printxy(x + 2, y + 4, "          ");
        textcolor(WHITE, BLACK); printxy(x + 12, y + 4, "  ");

        textcolor(WHITE, BLACK); printxy(x + 0, y + 5, "  ");
        textcolor(WHITE, BLUE1); printxy(x + 2, y + 5, "          ");
        textcolor(WHITE, BLACK); printxy(x + 12, y + 5, "  ");

        textcolor(WHITE, WHITE); printxy(x + 0, y + 6, "  ");
        textcolor(WHITE, BLACK); printxy(x + 2, y + 6, "          ");
        textcolor(WHITE, WHITE); printxy(x + 12, y + 6, "  ");

        textcolor(WHITE, WHITE); printxy(x, y + 7, "              ");
        textcolor(BLACK, WHITE);
        break;
    case 7:
        textcolor(WHITE, WHITE); printxy(x, y - 1, "              ");  //레드 포션(체력 포션)

        textcolor(WHITE, WHITE); printxy(x + 0, y + 0, "   ");
        textcolor(WHITE, BLACK); printxy(x + 3, y + 0, "   ");
        textcolor(WHITE, BROWN); printxy(x + 6, y + 0, "  ");
        textcolor(WHITE, BLACK); printxy(x + 8, y + 0, "   ");
        textcolor(WHITE, WHITE); printxy(x + 11, y + 0, "   ");

        textcolor(WHITE, WHITE); printxy(x + 0, y + 1, "    ");
        textcolor(WHITE, BLACK); printxy(x + 4, y + 1, "  ");
        textcolor(WHITE, WHITE); printxy(x + 6, y + 1, "  ");
        textcolor(WHITE, BLACK); printxy(x + 8, y + 1, "  ");
        textcolor(WHITE, WHITE); printxy(x + 10, y + 1, "    ");

        textcolor(WHITE, WHITE); printxy(x + 0, y + 2, "    ");
        textcolor(WHITE, BLACK); printxy(x + 4, y + 2, "  ");
        textcolor(WHITE, RED1); printxy(x + 6, y + 2, "  ");
        textcolor(WHITE, BLACK); printxy(x + 8, y + 2, "  ");
        textcolor(WHITE, WHITE); printxy(x + 10, y + 2, "    ");

        textcolor(WHITE, WHITE); printxy(x + 0, y + 3, "  ");
        textcolor(WHITE, BLACK); printxy(x + 2, y + 3, "  ");
        textcolor(WHITE, RED1); printxy(x + 4, y + 3, "      ");
        textcolor(WHITE, BLACK); printxy(x + 10, y + 3, "  ");
        textcolor(WHITE, WHITE); printxy(x + 12, y + 3, "  ");

        textcolor(WHITE, BLACK); printxy(x + 0, y + 4, "  ");
        textcolor(WHITE, RED1); printxy(x + 2, y + 4, "          ");
        textcolor(WHITE, BLACK); printxy(x + 12, y + 4, "  ");

        textcolor(WHITE, BLACK); printxy(x + 0, y + 5, "  ");
        textcolor(WHITE, RED1); printxy(x + 2, y + 5, "          ");
        textcolor(WHITE, BLACK); printxy(x + 12, y + 5, "  ");

        textcolor(WHITE, WHITE); printxy(x + 0, y + 6, "  ");
        textcolor(WHITE, BLACK); printxy(x + 2, y + 6, "          ");
        textcolor(WHITE, WHITE); printxy(x + 12, y + 6, "  ");

        textcolor(WHITE, WHITE); printxy(x, y + 7, "              ");
        textcolor(BLACK, WHITE);
        break;
    case 8:
        textcolor(WHITE, WHITE); printxy(x, y - 1, "              "); //스페셜 아이템 깃발

        textcolor(WHITE, YELLOW2); printxy(x + 0, y + 0, "              ");

        textcolor(WHITE, YELLOW2); printxy(x + 0, y + 1, "  ");
        textcolor(WHITE, WHITE); printxy(x + 2, y + 1, "  ");
        textcolor(WHITE, BROWN); printxy(x + 4, y + 1, "  ");
        textcolor(WHITE, GRAY2); printxy(x + 6, y + 1, "  ");
        textcolor(WHITE, WHITE); printxy(x + 8, y + 1, "    ");
        textcolor(WHITE, YELLOW2); printxy(x + 12, y + 1, "  ");

        textcolor(WHITE, YELLOW2); printxy(x + 0, y + 2, "  ");
        textcolor(WHITE, WHITE); printxy(x + 2, y + 2, "  ");
        textcolor(WHITE, BROWN); printxy(x + 4, y + 2, "  ");
        textcolor(WHITE, GRAY2); printxy(x + 6, y + 2, "    ");
        textcolor(WHITE, WHITE); printxy(x + 10, y + 2, "  ");
        textcolor(WHITE, YELLOW2); printxy(x + 12, y + 2, "  ");

        textcolor(WHITE, YELLOW2); printxy(x + 0, y + 3, "  ");
        textcolor(WHITE, WHITE); printxy(x + 2, y + 3, "  ");
        textcolor(WHITE, BROWN); printxy(x + 4, y + 3, "  ");
        textcolor(WHITE, GRAY2); printxy(x + 6, y + 3, "      ");
        textcolor(WHITE, YELLOW2); printxy(x + 12, y + 3, "  ");

        textcolor(WHITE, YELLOW2); printxy(x + 0, y + 4, "  ");
        textcolor(WHITE, WHITE); printxy(x + 2, y + 4, "  ");
        textcolor(WHITE, BROWN); printxy(x + 4, y + 4, "  ");
        textcolor(WHITE, WHITE); printxy(x + 6, y + 4, "      ");
        textcolor(WHITE, YELLOW2); printxy(x + 12, y + 4, "  ");

        textcolor(WHITE, YELLOW2); printxy(x + 0, y + 5, "  ");
        textcolor(WHITE, WHITE); printxy(x + 2, y + 5, "  ");
        textcolor(WHITE, BROWN); printxy(x + 4, y + 5, "  ");
        textcolor(WHITE, WHITE); printxy(x + 6, y + 5, "      ");
        textcolor(WHITE, YELLOW2); printxy(x + 12, y + 5, "  ");

        textcolor(WHITE, YELLOW2); printxy(x + 0, y + 6, "              ");

        textcolor(WHITE, WHITE); printxy(x, y + 7, "              ");
        textcolor(BLACK, WHITE);
        break;
    case 9:
        textcolor(WHITE, WHITE); printxy(x, y - 1, "              "); //스페셜아이템(구슬)

        textcolor(WHITE, WHITE); printxy(x + 0, y + 0, "    ");
        textcolor(WHITE, MAGENTA1); printxy(x + 4, y + 0, "      ");
        textcolor(WHITE, WHITE); printxy(x + 10, y + 0, "    ");

        textcolor(WHITE, WHITE); printxy(x + 0, y + 1, "  ");
        textcolor(WHITE, MAGENTA1); printxy(x + 2, y + 1, "      ");
        textcolor(WHITE, WHITE); printxy(x + 8, y + 1, "  ");
        textcolor(WHITE, MAGENTA1); printxy(x + 10, y + 1, "  ");
        textcolor(WHITE, WHITE); printxy(x + 12, y + 1, "  ");

        textcolor(WHITE, MAGENTA1); printxy(x + 1, y + 2, "         ");
        textcolor(WHITE, WHITE); printxy(x + 10, y + 2, "  ");
        textcolor(WHITE, MAGENTA1); printxy(x + 12, y + 2, " ");

        textcolor(WHITE, MAGENTA1); printxy(x + 1, y + 3, "             ");
        textcolor(WHITE, WHITE); printxy(x + 13, y + 3, " ");
        textcolor(WHITE, MAGENTA1); printxy(x + 1, y + 4, "             ");
        textcolor(WHITE, WHITE); printxy(x + 13, y + 4, " ");

        textcolor(WHITE, WHITE); printxy(x + 0, y + 5, "  ");
        textcolor(WHITE, MAGENTA1); printxy(x + 2, y + 5, "          ");
        textcolor(WHITE, WHITE); printxy(x + 12, y + 5, "  ");

        textcolor(WHITE, WHITE); printxy(x + 0, y + 6, "    ");
        textcolor(WHITE, MAGENTA1); printxy(x + 4, y + 6, "          ");
        textcolor(WHITE, WHITE); printxy(x + 10, y + 6, "    ");

        textcolor(WHITE, WHITE); printxy(x, y + 7, "              ");
        textcolor(BLACK, WHITE);
        break;
    default:
        break;
    }
}
void erase_item(int y, int x) //아이템 지우기
{
    x *= 2; //x는 배열 정보기 때문(*2)
    textcolor(WHITE, WHITE);
    printxy(x, y + 0, "              ");
    printxy(x, y + 1, "              ");
    printxy(x, y + 2, "              ");
    printxy(x, y + 3, "              ");
    printxy(x, y + 4, "              ");
    printxy(x, y + 5, "              ");
    printxy(x, y + 6, "              ");
    printxy(x, y + 7, "              ");

}
void enter_slot(int player, int ITnum) //어아템 슬롯에 그리기, 정보 획득
{
    if (player == 99) //RED
    {
        Red.item_slot = ITnum;
        drow_item(2, 92, ITnum);
    }
    else if (player == 98) //BLUE
    {
        Blue.item_slot = ITnum;
        drow_item(2, 292, ITnum);
    }
}
void item_spawn() //일반 아이템 생성, 콜라이더 정보 삽입
{
    int i, j;
    int ITnum; //아이템 번호
    for (i = 0; i < 3; i++)
        item_switch[i] = 1; //6개 모든 아이템 스위치 on

    //콜라이더 번호 6:장전 , 7:회복 , 8:슈퍼점프, 9:초기화(보관 불가능), 10:스킬완충(스페셜?)

    ITnum = 6 + (rand() % 2);
    for (i = itemxy[0].y; i <= itemxy[0].y + 6; i++) //0번 위치 아이템
    {
        for (j = itemxy[0].x; j <= itemxy[0].x + 6; j++)
            map[i][j] = ITnum;
    }
    drow_item(itemxy[0].y, itemxy[0].x, ITnum); //아이템 그리기
    itemxy[0].val = ITnum;

    ITnum = 6 + (rand() % 2);
    for (i = itemxy[1].y; i <= itemxy[1].y + 6; i++) //1번 위치 아이템
    {
        for (j = itemxy[1].x; j <= itemxy[1].x + 6; j++)
            map[i][j] = ITnum;
    }
    drow_item(itemxy[1].y, itemxy[1].x, ITnum); //아이템 그리기
    itemxy[1].val = ITnum;

    ITnum = 8 + (rand() % 2);
    for (i = itemxy[2].y; i <= itemxy[2].y + 6; i++) //2번 위치 아이템
    {
        for (j = itemxy[2].x; j <= itemxy[2].x + 6; j++)
            map[i][j] = ITnum;
    }
    drow_item(itemxy[2].y, itemxy[2].x, ITnum); //아이템 그리기
    itemxy[2].val = ITnum;
}
void hit_item(int y, int x, int player) //캐릭터와 아이템 접촉시 상호작용
{   //전체 아이템의 접촉 정보를 판단하고 (접촉이면 습득, 비접촉이면 패스)
    int hit = 0, i, j;
    y += 5; //캐릭터는 13x6  몸체 중앙으로 collider를 옮김

    //일반 아이템
    if (map[y][x] == 6 || map[y][x + 6] == 6) //(6)장전 아이템
    {
        enter_slot(player, 6); //플레이어 아이템 슬롯에 그리고 정보 기입  
        hit = 1;
    }
    else if (map[y][x] == 7 || map[y][x + 6] == 7) //(7)회복 아이템
    {
        enter_slot(player, 7);
        hit = 1;
    }
    else if (map[y][x] == 8 || map[y][x + 1] == 8 || map[y][x + 2] == 8 
            || map[y][x + 3] == 8 || map[y][x + 4] == 8 || map[y][x + 5] == 8) //(9)깃발 초기화 아이템(먹으면 즉시 발동, 보관x)
    {
        hit = 1;
        flag_player = 0;
        checkFlag(0);//즉시 사용
    }
    else if (map[y][x] == 9 || map[y][x + 1] == 9 || map[y][x + 2] == 9 || 
            map[y][x + 3] == 9 || map[y][x + 4] == 9 || map[y][x + 5] == 9) //(10)스킬게이지 완충 아이템(먹으면 즉시 발동, 보관x)
    {
        hit = 1;//즉시 사용
        if (player == 99)
        {
            Red.skiPow = 3;
            print_skipow(1);
        }
        else if (player == 98)
        {
            Blue.skiPow = 3;
            print_skipow(2);
        }
    }

    //콜라이더 지우고, 아이템 그림 지우기
    if (hit == 1 && (itemxy[0].x - 5 <= x && x <= itemxy[0].x + 10))
    {
        erase_item(itemxy[0].y, itemxy[0].x);
        for (i = itemxy[0].y; i <= itemxy[0].y + 6; i++) //0번 위치 아이템
        {
            for (j = itemxy[0].x; j <= itemxy[0].x + 6; j++)
                map[i][j] = 0;
        }
        item_switch[0] = 0; //아이템 스위치를 off
    }
    else if (hit == 1 && (itemxy[1].x - 5 <= x && x <= itemxy[1].x + 10))
    {
        erase_item(itemxy[1].y, itemxy[1].x);
        for (i = itemxy[1].y; i <= itemxy[1].y + 6; i++) //1번 위치 아이템
        {
            for (j = itemxy[1].x; j <= itemxy[1].x + 6; j++)
                map[i][j] = 0;
        }
        item_switch[1] = 0; //아이템 스위치를 off
    }
    else if (hit == 1 && (itemxy[2].x - 5 <= x && x <= itemxy[2].x + 10))
    {
        erase_item(itemxy[2].y, itemxy[2].x);
        for (i = itemxy[2].y; i <= itemxy[2].y + 6; i++) //2번 위치 아이템
        {
            for (j = itemxy[2].x; j <= itemxy[2].x + 6; j++)
                map[i][j] = 0;
        }
        item_switch[2] = 0; //아이템 스위치를 off
    }

}

//게임 세팅, 시간 관련 함수들
void show_time(int remain_time) //값(초), 몇의 자리수
{
    int a = remain_time / 100; //100의 자리
    int b = (remain_time % 100) / 10; //10의 자리
    int c = remain_time % 10; //1의 자리
    int x, y = 11; // 출력 좌표
    int n = 0;

    textcolor(YELLOW2, GRAY2);
    if (remain_time <= 10) textcolor(RED1, BLACK); //10초 이하면 빨강으로 강조

    for (x = 284; x < 305; x += 10)
    {
        if (x == 284)
        {
            n = a;
        }
        else if (x == 294)
        {
            n = b;

        }
        else if (x == 304)
        {
            n = c;
        }
        switch (n)
        {
        case 0:
            printxy(x, y + 0, "  #####   ");
            printxy(x, y + 1, " ##   ##  ");
            printxy(x, y + 2, "##   # ## ");
            printxy(x, y + 3, "##  #  ## ");
            printxy(x, y + 4, "## #   ## ");
            printxy(x, y + 5, " ##   ##  ");
            printxy(x, y + 6, "  #####   ");
            break;
        case 1:
            printxy(x, y + 0, "    ##    ");
            printxy(x, y + 1, "  ####    ");
            printxy(x, y + 2, "    ##    ");
            printxy(x, y + 3, "    ##    ");
            printxy(x, y + 4, "    ##    ");
            printxy(x, y + 5, "    ##    ");
            printxy(x, y + 6, "  ######  ");
            break;
        case 2:
            printxy(x, y + 0, " #######  ");
            printxy(x, y + 1, "##     ## ");
            printxy(x, y + 2, "##     ## ");
            printxy(x, y + 3, "     ###  ");
            printxy(x, y + 4, "   ##     ");
            printxy(x, y + 5, " ##       ");
            printxy(x, y + 6, "######### ");
            break;
        case 3:
            printxy(x, y + 0, " #######  ");
            printxy(x, y + 1, "##     ## ");
            printxy(x, y + 2, "       ## ");
            printxy(x, y + 3, "  ######  ");
            printxy(x, y + 4, "       ## ");
            printxy(x, y + 5, "##     ## ");
            printxy(x, y + 6, " #######  ");
            break;
        case 4:
            printxy(x, y + 0, "    ####  ");
            printxy(x, y + 1, "   ## ##  ");
            printxy(x, y + 2, "  ##  ##  ");
            printxy(x, y + 3, " ##   ##  ");
            printxy(x, y + 4, "######### ");
            printxy(x, y + 5, "      ##  ");
            printxy(x, y + 6, "      ##  ");
            break;
        case 5:
            printxy(x, y + 0, "########  ");
            printxy(x, y + 1, "##        ");
            printxy(x, y + 2, "##        ");
            printxy(x, y + 3, "########  ");
            printxy(x, y + 4, "       ## ");
            printxy(x, y + 5, "##     ## ");
            printxy(x, y + 6, " #######  ");
            break;
        case 6:
            printxy(x, y + 0, " #######  ");
            printxy(x, y + 1, "##     ## ");
            printxy(x, y + 2, "##        ");
            printxy(x, y + 3, "########  ");
            printxy(x, y + 4, "##     ## ");
            printxy(x, y + 5, "##     ## ");
            printxy(x, y + 6, " ######   ");
            break;
        case 7:
            printxy(x, y + 0, " ######## ");
            printxy(x, y + 1, " ##    ## ");
            printxy(x, y + 2, "      ##  ");
            printxy(x, y + 3, "     ##   ");
            printxy(x, y + 4, "    ##    ");
            printxy(x, y + 5, "    ##    ");
            printxy(x, y + 6, "    ##    ");
            break;
        case 8:
            printxy(x, y + 0, " #######  ");
            printxy(x, y + 1, "##     ## ");
            printxy(x, y + 2, "##     ## ");
            printxy(x, y + 3, " #######  ");
            printxy(x, y + 4, "##     ## ");
            printxy(x, y + 5, "##     ## ");
            printxy(x, y + 6, " #######  ");
            break;
        case 9:
            printxy(x, y + 0, " #######  ");
            printxy(x, y + 1, "##     ## ");
            printxy(x, y + 2, "##     ## ");
            printxy(x, y + 3, " ######## ");
            printxy(x, y + 4, "       ## ");
            printxy(x, y + 5, "##     ## ");
            printxy(x, y + 6, " #######  ");
            break;

        }
    }
    textcolor(BLACK, WHITE);
}
void show_score(int player)
{
    int a, b; //자리수 
    int x, y = 2; // 출력 좌표 //RED:237 BLUE:367 , y:2-8
    int n = 0, i;

    if (player == 1)
    {
        a = Red.score / 10; //10의 자리
        b = Red.score % 10; //1의 자리
        x = 237;
        //textcolor(RED1, GRAY1);
        textcolor(BLACK, PINK);
    }
    else
    {
        a = Blue.score / 10; //10의 자리
        b = Blue.score % 10; //1의 자리
        x = 333;
        //textcolor(BLUE1, GRAY1);
        textcolor(BLACK, CYAN2);
    }

    i = x;
    for (; x < i + 11; x += 10)
    {
        if (x == i)
        {
            n = a;
        }
        else if (x == i + 10)
        {
            n = b;
        }
        switch (n)
        {
        case 0:
            printxy(x, y + 0, "  #####   ");
            printxy(x, y + 1, " ##   ##  ");
            printxy(x, y + 2, "##   # ## ");
            printxy(x, y + 3, "##  #  ## ");
            printxy(x, y + 4, "## #   ## ");
            printxy(x, y + 5, " ##   ##  ");
            printxy(x, y + 6, "  #####   ");
            break;
        case 1:
            printxy(x, y + 0, "    ##    ");
            printxy(x, y + 1, "  ####    ");
            printxy(x, y + 2, "    ##    ");
            printxy(x, y + 3, "    ##    ");
            printxy(x, y + 4, "    ##    ");
            printxy(x, y + 5, "    ##    ");
            printxy(x, y + 6, "  ######  ");
            break;
        case 2:
            printxy(x, y + 0, " #######  ");
            printxy(x, y + 1, "##     ## ");
            printxy(x, y + 2, "##     ## ");
            printxy(x, y + 3, "     ###  ");
            printxy(x, y + 4, "   ##     ");
            printxy(x, y + 5, " ##       ");
            printxy(x, y + 6, "######### ");
            break;
        case 3:
            printxy(x, y + 0, " #######  ");
            printxy(x, y + 1, "##     ## ");
            printxy(x, y + 2, "       ## ");
            printxy(x, y + 3, "  ######  ");
            printxy(x, y + 4, "       ## ");
            printxy(x, y + 5, "##     ## ");
            printxy(x, y + 6, " #######  ");
            break;
        case 4:
            printxy(x, y + 0, "    ####  ");
            printxy(x, y + 1, "   ## ##  ");
            printxy(x, y + 2, "  ##  ##  ");
            printxy(x, y + 3, " ##   ##  ");
            printxy(x, y + 4, "######### ");
            printxy(x, y + 5, "      ##  ");
            printxy(x, y + 6, "      ##  ");
            break;
        case 5:
            printxy(x, y + 0, "########  ");
            printxy(x, y + 1, "##        ");
            printxy(x, y + 2, "##        ");
            printxy(x, y + 3, "########  ");
            printxy(x, y + 4, "       ## ");
            printxy(x, y + 5, "##     ## ");
            printxy(x, y + 6, " #######  ");
            break;
        case 6:
            printxy(x, y + 0, " #######  ");
            printxy(x, y + 1, "##     ## ");
            printxy(x, y + 2, "##        ");
            printxy(x, y + 3, "########  ");
            printxy(x, y + 4, "##     ## ");
            printxy(x, y + 5, "##     ## ");
            printxy(x, y + 6, " ######   ");
            break;
        case 7:
            printxy(x, y + 0, " ######## ");
            printxy(x, y + 1, " ##    ## ");
            printxy(x, y + 2, "      ##  ");
            printxy(x, y + 3, "     ##   ");
            printxy(x, y + 4, "    ##    ");
            printxy(x, y + 5, "    ##    ");
            printxy(x, y + 6, "    ##    ");
            break;
        case 8:
            printxy(x, y + 0, " #######  ");
            printxy(x, y + 1, "##     ## ");
            printxy(x, y + 2, "##     ## ");
            printxy(x, y + 3, " #######  ");
            printxy(x, y + 4, "##     ## ");
            printxy(x, y + 5, "##     ## ");
            printxy(x, y + 6, " #######  ");
            break;
        case 9:
            printxy(x, y + 0, " #######  ");
            printxy(x, y + 1, "##     ## ");
            printxy(x, y + 2, "##     ## ");
            printxy(x, y + 3, " ######## ");
            printxy(x, y + 4, "       ## ");
            printxy(x, y + 5, "##     ## ");
            printxy(x, y + 6, " #######  ");
            break;
        }
    }
    textcolor(BLACK, WHITE);
}
void init_Game() //게임 초기화
{
    map_cls(); // 맵 콜라이더 초기화
    default_map(); //고정 오브젝트들은 화면에 미리 뿌림
    DrawMap(); //고정 오브젝트 그리기

    Red.arrow = 1; //오른쪽을 바라보며 스폰
    Blue.arrow = -1; //왼쪽을 바라보며 스폰

    Red.shots = Blue.shots = MAXBULLET;
    shot_count(1); //RED 탄수
    shot_count(2); //BLUE 탄수

    Red.hp = Blue.hp = MAXHP; //체력 20(변경시 함수까지 변경해야함)
    init_hp(1); //RED 체력
    init_hp(2); //BLUE 체력

    //플레이어의 위치 초기화
    Red.oldX = Red.newX = 17, Red.oldY = Red.newY = 155; //레드
    Blue.oldX = Blue.newX = 277, Blue.oldY = Blue.newY = 155; //블루

    //플레이어 점수 출력
    Red.score = 0;
    Blue.score = 0;
    show_score(1);
    show_score(2);

    //스킬 게이지 초기화, 출력
    Red.skiPow = 3;
    Blue.skiPow = 3;
    print_skipow(1);
    print_skipow(2);

    //플레이어 아이템 슬롯 비우기
    Red.item_slot = 0;
    Blue.item_slot = 0;
    for (int i = 0; i < 3; i++)
        item_switch[i] = 0;

    //점령 포인트 초기화
    checkFlag(0);
    flag_player = 0;

    //승리자 초기화
    winner = 0;
}
void Game_start()
{
    textcolor(YELLOW2, BLACK); printxy(252, 91 + 0, "  ######  ########    ###    ########  ########    ##    ## ########  ##     ## #### ########  ## ");
    textcolor(YELLOW2, BLACK); printxy(252, 91 + 1, " ##    ##    ##      ## ##   ##     ##    ##       ##   ##  ##     ## ##     ##  ##     ##     ## ");
    textcolor(YELLOW2, BLACK); printxy(252, 91 + 2, " ##          ##     ##   ##  ##     ##    ##       ##  ##   ##     ## ##     ##  ##     ##     ## ");
    textcolor(YELLOW2, BLACK); printxy(252, 91 + 3, "  ######     ##    ##     ## ########     ##       #####    ########  #########  ##     ##     ## ");
    textcolor(YELLOW2, BLACK); printxy(252, 91 + 4, "       ##    ##    ######### ##   ##      ##       ##  ##   ##     ## ##     ##  ##     ##     ## ");
    textcolor(YELLOW2, BLACK); printxy(252, 91 + 5, " ##    ##    ##    ##     ## ##    ##     ##       ##   ##  ##     ## ##     ##  ##     ##        ");
    textcolor(YELLOW2, BLACK); printxy(252, 91 + 6, "  ######     ##    ##     ## ##     ##    ##       ##    ## ########  ##     ## ####    ##     ## ");
    getch();
    for (int j = 91; j < 98; j++)
    {
        for (int i = 252; i < 352; i++)
        {
            textcolor(BLACK, WHITE); printxy(i, j, " ");
        }
    }
}

//타이틀화면 관련 함수
void title_red(int x, int y)
{
    textcolor(GRAY2, GRAY2); printxy(x, y - 1, "            ");

    textcolor(GRAY2, GRAY2); printxy(x, y, "    ");
    textcolor(WHITE, RED1); printxy(x + 4, y, "●");
    textcolor(GRAY2, GRAY2); printxy(x + 6, y, "      ");

    textcolor(GRAY2, GRAY2); printxy(x, y + 1, "    ");
    textcolor(RED1, RED1); printxy(x + 4, y + 1, "    ");
    textcolor(GRAY2, GRAY2); printxy(x + 8, y + 1, "    ");

    textcolor(GRAY2, GRAY2); printxy(x, y + 2, "  ");
    textcolor(RED1, RED1); printxy(x + 2, y + 2, "        ");
    textcolor(GRAY2, GRAY2); printxy(x + 10, y + 2, "  ");

    textcolor(YELLOW2, BLACK); printxy(x, y + 3, "      ◆    ");

    textcolor(GRAY2, GRAY2); printxy(x, y + 4, "  ");
    textcolor(BLUE1, SKIN); printxy(x + 2, y + 4, "  ♥  ♥");
    textcolor(GRAY2, GRAY2); printxy(x + 10, y + 4, "  ");

    textcolor(GRAY2, GRAY2); printxy(x, y + 5, "  ");
    textcolor(GRAY1, GRAY1); printxy(x + 2, y + 5, "  ");
    textcolor(ORANGE, SKIN); printxy(x + 4, y + 5, "  ３");
    textcolor(GRAY1, GRAY1); printxy(x + 8, y + 5, "  ");
    textcolor(GRAY2, GRAY2); printxy(x + 10, y + 5, "  ");

    textcolor(WHITE, GRAY2); printxy(x, y + 6, " ");
    textcolor(RED1, RED1); printxy(x + 1, y + 6, "   ");
    textcolor(GRAY1, GRAY1); printxy(x + 3, y + 6, "      ");
    textcolor(RED1, RED1); printxy(x + 9, y + 6, "  ");
    textcolor(WHITE, GRAY2); printxy(x + 11, y + 6, " ");

    textcolor(RED1, RED1); printxy(x, y + 7, "    ");
    textcolor(GRAY1, GRAY1); printxy(x + 4, y + 7, "    ");
    textcolor(RED1, RED1); printxy(x + 8, y + 7, "    ");

    textcolor(RED1, RED1); printxy(x, y + 8, "    ");
    textcolor(GRAY1, GRAY1); printxy(x + 4, y + 8, "  ");
    textcolor(RED1, RED1); printxy(x + 6, y + 8, "    ");
    textcolor(SKIN, SKIN); printxy(x + 10, y + 8, "  ");

    textcolor(SKIN, SKIN); printxy(x, y + 9, "  ");
    textcolor(RED1, RED1); printxy(x + 2, y + 9, "        ");
    textcolor(WHITE, GRAY2); printxy(x + 10, y + 9, "  ");

    textcolor(WHITE, GRAY2); printxy(x, y + 10, "  ");
    textcolor(RED1, RED1); printxy(x + 2, y + 10, "        ");
    textcolor(WHITE, GRAY2); printxy(x + 10, y + 10, "  ");

    textcolor(WHITE, GRAY2); printxy(x, y + 11, "  ");
    textcolor(RED1, RED1); printxy(x + 2, y + 11, "        ");
    textcolor(WHITE, GRAY2); printxy(x + 10, y + 11, "  ");

    textcolor(WHITE, GRAY2); printxy(x, y + 12, "  ");
    textcolor(BROWN, BROWN); printxy(x + 2, y + 12, "        ");
    textcolor(WHITE, GRAY2); printxy(x + 10, y + 12, "  ");

    textcolor(WHITE, GRAY2); printxy(x, y + 13, "            ");
    textcolor(BLACK, GRAY2);

    /*textcolor(RED1, GRAY1);
    printxy(x, y - 1 , "                               ");
    printxy(x, y + 0 , "           /\                  ");
    printxy(x, y + 1 , "          /  \                 ");
    printxy(x, y + 2 , "         |    |                ");
    printxy(x, y + 3 , "       --:'''':--              ");
    printxy(x, y + 4 , "         : '_':                ");
    printxy(x, y + 5 , "      ___/:"":_                ");
    printxy(x, y + 6 , "   _.'     ::: '.____     ' '  ");
    printxy(x, y + 7 , "  :    /           )=>>====* . ");
    printxy(x, y + 8 , " '._.'/      _/'-'-'     '  .  ");
    printxy(x, y + 9 , "   "" _:====\                   ");
    printxy(x, y + 10, "     \\     '.                 ");
    printxy(x, y + 11, "      :       :                ");
    printxy(x, y + 12, "      /    :   \               ");
    printxy(x, y + 13, "    .'      .   :              ");
    printxy(x, y + 14, "    :      : :  :              ");
    printxy(x, y + 15, "    '--;.__:-:__:              ");
    printxy(x, y + 16, "                               ");*/

}
void title_red_ball(int x, int y)
{
    textcolor(RED1, RED1); printxy(x + 6, y, "        ");
    //
    textcolor(RED1, RED1); printxy(x + 2, y + 1, "    ");
    textcolor(ORANGE, ORANGE); printxy(x + 6, y + 1, "      ");
    textcolor(YELLOW2, YELLOW2); printxy(x + 12, y + 1, "  ");
    textcolor(RED1, RED1); printxy(x + 14, y + 1, "  ");
    //
    textcolor(RED1, RED1); printxy(x, y + 2, "    ");
    textcolor(ORANGE, ORANGE); printxy(x + 4, y + 2, "    ");
    textcolor(YELLOW2, YELLOW2); printxy(x + 8, y + 2, "    ");
    textcolor(RED1, RED1); printxy(x + 12, y + 2, "    ");
    //
    textcolor(RED1, RED1); printxy(x + 6, y + 3, "        ");
    textcolor(BLACK, WHITE);
}
void title_blue(int x, int y)
{
    textcolor(WHITE, GRAY2); printxy(x, y - 1, "            ");

    textcolor(WHITE, GRAY2); printxy(x, y, "      ");
    textcolor(WHITE, BLUE1); printxy(x + 6, y, "●");
    textcolor(WHITE, GRAY2); printxy(x + 8, y, "    ");

    textcolor(WHITE, GRAY2); printxy(x, y + 1, "    ");
    textcolor(BLUE1, BLUE1); printxy(x + 4, y + 1, "    ");
    textcolor(WHITE, GRAY2); printxy(x + 8, y + 1, "    ");

    textcolor(WHITE, GRAY2); printxy(x, y + 2, "  ");
    textcolor(BLUE1, BLUE1); printxy(x + 2, y + 2, "        ");
    textcolor(WHITE, GRAY2); printxy(x + 10, y + 2, "  ");

    textcolor(YELLOW2, BLACK); printxy(x, y + 3, "    ◆      ");

    textcolor(WHITE, GRAY2); printxy(x, y + 4, "  ");
    textcolor(RED1, SKIN); printxy(x + 2, y + 4, "♥  ♥  ");
    textcolor(WHITE, GRAY2); printxy(x + 10, y + 4, "  ");

    textcolor(WHITE, GRAY2); printxy(x, y + 5, "  ");
    textcolor(GRAY1, GRAY1); printxy(x + 2, y + 5, "  ");
    textcolor(ORANGE, SKIN); printxy(x + 4, y + 5, "ε  ");
    textcolor(GRAY1, GRAY1); printxy(x + 8, y + 5, "  ");
    textcolor(WHITE, GRAY2); printxy(x + 10, y + 5, "  ");

    textcolor(WHITE, GRAY2); printxy(x, y + 6, " ");
    textcolor(BLUE1, BLUE1); printxy(x + 1, y + 6, "   ");
    textcolor(GRAY1, GRAY1); printxy(x + 3, y + 6, "      ");
    textcolor(BLUE1, BLUE1); printxy(x + 9, y + 6, "  ");
    textcolor(WHITE, GRAY2); printxy(x + 11, y + 6, " ");

    textcolor(BLUE1, BLUE1); printxy(x, y + 7, "    ");
    textcolor(GRAY1, GRAY1); printxy(x + 4, y + 7, "    ");
    textcolor(BLUE1, BLUE1); printxy(x + 8, y + 7, "    ");

    textcolor(SKIN, SKIN); printxy(x, y + 8, "  ");
    textcolor(BLUE1, BLUE1); printxy(x + 2, y + 8, "    ");
    textcolor(GRAY1, GRAY1); printxy(x + 6, y + 8, "  ");
    textcolor(BLUE1, BLUE1); printxy(x + 8, y + 8, "    ");

    textcolor(WHITE, GRAY2); printxy(x, y + 9, "  ");
    textcolor(BLUE1, BLUE1); printxy(x + 2, y + 9, "        ");
    textcolor(SKIN, SKIN); printxy(x + 10, y + 9, "  ");

    textcolor(WHITE, GRAY2); printxy(x, y + 10, "  ");
    textcolor(BLUE1, BLUE1); printxy(x + 2, y + 10, "        ");
    textcolor(WHITE, GRAY2); printxy(x + 10, y + 10, "  ");

    textcolor(WHITE, GRAY2); printxy(x, y + 11, "  ");
    textcolor(BLUE1, BLUE1); printxy(x + 2, y + 11, "        ");
    textcolor(WHITE, GRAY2); printxy(x + 10, y + 11, "  ");

    textcolor(WHITE, GRAY2); printxy(x, y + 12, "  ");
    textcolor(BROWN, BROWN); printxy(x + 2, y + 12, "        ");
    textcolor(WHITE, GRAY2); printxy(x + 10, y + 12, "  ");

    textcolor(WHITE, GRAY2); printxy(x, y + 13, "            ");
    textcolor(BLACK, GRAY2);

    /* textcolor(BLUE1, GRAY1);
     printxy(x, y - 1 , "                               ");
     printxy(x, y + 0 , "                  /\           ");
     printxy(x, y + 1 , "                 /  \          ");
     printxy(x, y + 2 , "                |    |         ");
     printxy(x, y + 3 , "              --:'''':--       ");
     printxy(x, y + 4 , "                :'_' :         ");
     printxy(x, y + 5 , "                _:"":\___      ");
     printxy(x, y + 6 , "  ' '     ____.' :::     '._   ");
     printxy(x, y + 7 , " . *====<<=)           \    :  ");
     printxy(x, y + 8 , "  .  '     '-'-'\_      /'._.' ");
     printxy(x, y + 9 , "                  \====:_ ""    ");
     printxy(x, y + 10, "                 .'     \\     ");
     printxy(x, y + 11, "                :       :      ");
     printxy(x, y + 12, "               /   :    \      ");
     printxy(x, y + 13, "              :   .      '.    ");
     printxy(x, y + 14, "              :  : :      :    ");
     printxy(x, y + 15, "              :__:-:__.;--'    ");
     printxy(x, y + 16, "                               ");*/
}
void title_blue_ball(int x, int y)
{
    textcolor(BLUE1, BLUE1); printxy(x + 2, y, "        ");
    //
    textcolor(BLUE1, BLUE1); printxy(x, y + 1, "    ");
    textcolor(CYAN2, CYAN2); printxy(x + 4, y + 1, "    ");
    textcolor(CYAN1, CYAN1); printxy(x + 8, y + 1, "    ");
    textcolor(BLUE1, BLUE1); printxy(x + 12, y + 1, "    ");
    //
    textcolor(BLUE1, BLUE1); printxy(x, y + 2, "  ");
    textcolor(CYAN2, CYAN2); printxy(x + 2, y + 2, "  ");
    textcolor(CYAN1, CYAN1); printxy(x + 4, y + 2, "      ");
    textcolor(BLUE1, BLUE1); printxy(x + 10, y + 2, "    ");
    //
    textcolor(BLUE1, BLUE1); printxy(x + 2, y + 3, "        ");
    textcolor(BLACK, WHITE);
}
void title_book(int x, int y)
{
    textcolor(YELLOW2, BROWN);
    for (int i = y - 1; i < y + 17; i++)
    {
        for (int j = x - 2; j < x + 51; j++)
        {
            printxy(j, i, " ");
        }
    }
    textcolor(BLACK, GREEN2);
    printxy(x, y + 0, "    ___________________   ___________________    ");
    printxy(x, y + 1, ".-/|                   \ /                    |\-. ");
    printxy(x, y + 2, "||||   Player1 <RED>    :   Player2 <BLUE>   ||||");
    printxy(x, y + 3, "||||                    :                    ||||");
    printxy(x, y + 4, "||||  이동   >> WASD    :  이동   >> ↑↓↔  ||||");
    printxy(x, y + 5, "||||  발사   >> G       :  발사   >> 1       ||||");
    printxy(x, y + 6, "||||  스킬   >> F       :  스킬   >> 0       ||||");
    printxy(x, y + 7, "||||  아이템 >> H       :  아이템 >> 2       ||||");
    printxy(x, y + 8, "||||                    :                    ||||");
    printxy(x, y + 9, "||||  승리방법 1        :   승리방법2        ||||");
    printxy(x, y + 10, "||||                    :                    ||||");
    printxy(x, y + 11, "|||| 시간 종료시 까지   :   또는 15킬 먼저   ||||");
    printxy(x, y + 12, "|||| 거점 정령하면 승리 :   달성시 승리      ||||");
    printxy(x, y + 13, "||||___________________ : ___________________||||");
    printxy(x, y + 14, "||/====================\:/====================\\\\||");
    printxy(x, y + 15, "`---------------------~___~--------------------''");

    textcolor(YELLOW2, BLACK);
    printxy(x + 10, y - 3, "<<<Press Any Key to Start>>>");
}
void title_title(int x, int y)
{
    for (int i = 36; i < 114; i++) {
        for (int j = 4; j < 13; j++)
        {
            textcolor(YELLOW2, MAGENTA1); printxy(i, j, " ");
        }
    }
    //1라인
    for (int i = 0; i < 37; i++)
    {
        if (i == 4 || i == 8 || i == 12 || i == 13 ||
            i == 18 || i == 19 || i == 23 || i == 24 ||
            i == 25 || i == 28 || i == 29 || i == 31 ||
            i == 32 || i == 33 || i == 35 || i == 36 || i == 37)
        {
            textcolor(GRAY1, GRAY1);
        }
        else
        {
            textcolor(RED1, BLACK);
        }
        printxy((x + i) * 2, y, "♨");
    }
    //2라인
    for (int i = 0; i < 37; i++)
    {
        if (i == 0 || i == 6 || i == 9 || i == 12 ||
            i == 14 || i == 20 || i == 23 || i == 25 ||
            i == 28 || i == 30 || i == 34)
        {
            textcolor(BLACK, BLACK);
        }
        else
        {
            textcolor(GRAY1, GRAY1);
        }
        printxy((x + i) * 2, y + 1, "  ");
    }
    //3라인
    for (int i = 0; i < 37; i++)
    {
        if (i == 0 || i == 6 || i == 9 || i == 12 ||
            i == 14 || i == 20 || i == 23 || i == 25 ||
            i == 28 || i == 30 || i == 34)
        {
            textcolor(BLACK, BLACK);
        }
        else
        {
            textcolor(GRAY1, GRAY1);
        }
        printxy((x + i) * 2, y + 2, "  ");
    }
    //4라인
    for (int i = 0; i < 37; i++)
    {
        if (i == 4 || i == 5 || i == 7 || i == 8 ||
            i == 12 || i == 13 || i == 18 || i == 19 ||
            i == 23 || i == 24 || i == 29 || i == 31 ||
            i == 32 || i == 33 || i == 35 || i == 36)
        {
            textcolor(GRAY1, GRAY1);
        }
        else
        {
            textcolor(BLACK, BLACK);
        }
        printxy((x + i) * 2, y + 3, "  ");
    }
    //5라인
    for (int i = 0; i < 37; i++)
    {
        if (i == 0 || i == 6 || i == 9 || i == 12 ||
            i == 14 || i == 20 || i == 23 || i == 25 ||
            i == 28 || i == 30 || i == 34)
        {
            textcolor(BLACK, BLACK);
        }
        else
        {
            textcolor(GRAY1, GRAY1);
        }
        printxy((x + i) * 2, y + 4, "  ");
    }
    //6라인
    for (int i = 0; i < 37; i++)
    {
        if (i == 0 || i == 6 || i == 9 || i == 12 ||
            i == 14 || i == 20 || i == 23 || i == 25 ||
            i == 28 || i == 30 || i == 34)
        {
            textcolor(BLACK, BLACK);
        }
        else
        {
            textcolor(GRAY1, GRAY1);
        }
        printxy((x + i) * 2, y + 5, "  ");
    }
    //7라인
    for (int i = 0; i < 37; i++)
    {
        if (i == 1 || i == 2 || i == 3 || i == 4 ||
            i == 8 || i == 10 || i == 11 || i == 13 ||
            i == 18 || i == 19 || i == 23 || i == 24 ||
            i == 26 || i == 27 || i == 29 || i == 33)
        {
            textcolor(GRAY1, GRAY1);
        }
        else
        {
            textcolor(BLUE1, BLACK);
        }
        printxy((x + i) * 2, y + 6, "♨");
    }
}
void title_menu(int x, int y)
{
    textcolor(BROWN, BROWN);
    for (int i = y - 1; i < y + 17; i++)
    {
        for (int j = x - 2; j < x + 51; j++)
        {
            printxy(j, i, " ");
        }
    }
    textcolor(BLACK, GREEN2);
    for (int i = y; i < y + 16; i++)
    {
        for (int j = x; j < x + 49; j++)
        {
            printxy(j, i, " ");
        }
    }
    printxy(x + 21, y + 3, "<<메뉴>>");
    printxy(x + 14, y + 6, " START   ==☞  { ① } ");
    printxy(x + 14, y + 8, " CONTROL ==☞  { ② } ");
    printxy(x + 14, y + 10, " EXIT    ==☞  { ③ } ");
}

//종료화면 관리 함수
void print_winner(int player)
{
    if (player == 1) //레드 승리
    {
        textcolor(RED1, RED1);
        for (int i = 38; i < 110; i++)
        {
            for (int j = 7; j < 18; j++)
            {
                printxy(i, j, " ");
            }
        }
        textcolor(YELLOW2, BLACK);
        printxy(40, 8, "                                                                    ");
        printxy(40, 9, "  ########  ######## ########      ##      ##  ####  ##    ##   ##  ");
        printxy(40, 10, "  ##     ## ##       ##     ##     ##  ##  ##   ##   ###   ##   ##  ");
        printxy(40, 11, "  ##     ## ##       ##     ##     ##  ##  ##   ##   ####  ##   ##  ");
        printxy(40, 12, "  ########  ######   ##     ##     ##  ##  ##   ##   ## ## ##   ##  ");
        printxy(40, 13, "  ##   ##   ##       ##     ##     ##  ##  ##   ##   ##  ####   ##  ");
        printxy(40, 14, "  ##    ##  ##       ##     ##     ##  ##  ##   ##   ##   ###       ");
        printxy(40, 15, "  ##     ## ######## ########       ###  ###   ####  ##    ##   ##  ");
        printxy(40, 16, "                                                                    ");
    }
    else if (player == 2) //블루 승리
    {
        textcolor(BLUE1, BLUE1);
        for (int i = 35; i < 116; i++)
        {
            for (int j = 7; j < 18; j++)
            {
                printxy(i, j, " ");
            }
        }
        textcolor(YELLOW2, BLACK);
        printxy(37, 8, "                                                                             ");
        printxy(37, 9, "  ########  ##       ##     ## ########     ##      ##  ####  ##    ##   ##  ");
        printxy(37, 10, "  ##     ## ##       ##     ## ##           ##  ##  ##   ##   ###   ##   ##  ");
        printxy(37, 11, "  ##     ## ##       ##     ## ##           ##  ##  ##   ##   ####  ##   ##  ");
        printxy(37, 12, "  ########  ##       ##     ## ######       ##  ##  ##   ##   ## ## ##   ##  ");
        printxy(37, 13, "  ##     ## ##       ##     ## ##           ##  ##  ##   ##   ##  ####   ##  ");
        printxy(37, 14, "  ##     ## ##       ##     ## ##           ##  ##  ##   ##   ##   ###       ");
        printxy(37, 15, "  ########  ########  #######  ########      ###  ###   ####  ##    ##   ##  ");
        printxy(37, 16, "                                                                             ");
    }
    else //무승부
    {
        textcolor(GRAY2, GRAY2);
        for (int i = 50; i < 98; i++)
        {
            for (int j = 7; j < 18; j++)
            {
                printxy(i, j, " ");
            }
        }
        textcolor(WHITE, BLACK);
        printxy(52, 8, "                                            ");
        printxy(52, 9, "  ########  ########     ###    ##      ##  ");
        printxy(52, 10, "  ##     ## ##     ##   ## ##   ##  ##  ##  ");
        printxy(52, 11, "  ##     ## ##     ##  ##   ##  ##  ##  ##  ");
        printxy(52, 12, "  ##     ## ########  ######### ##  ##  ##  ");
        printxy(52, 13, "  ##     ## ##   ##   ##     ## ##  ##  ##  ");
        printxy(52, 14, "  ##     ## ##    ##  ##     ## ##  ##  ##  ");
        printxy(52, 15, "  ########  ##     ## ##     ##  ###  ###   ");
        printxy(52, 16, "                                            ");
    }
}
void end_menu(int x, int y)
{
    char flag[5];
    int color = 0; //레드 블루 폰트 컬러
    int winType_f = 0; //플레그 부분의 백컬러
    int winType_r = 0; // 레드 부분의 백컬러
    int winType_b = 0; // 블루 부분의 백컬러
    if (Red.score == MAXSCORE) //red가 목표킬 달성했을 경우
    {
        winType_f = GREEN2;
        winType_r = WHITE;
        winType_b = GREEN2;
    }
    else if (Blue.score == MAXSCORE) //blue가 목표킬 달성했을 경우
    {
        winType_f = GREEN2;
        winType_r = GREEN2;
        winType_b = WHITE;
    }
    else if (flag_player == 0 && Red.score > Blue.score) //미점령이고 red가 더 높은 경우
    {
        winType_f = GREEN2;
        winType_r = WHITE;
        winType_b = GREEN2;
    }
    else if (flag_player == 0 && Red.score < Blue.score) //미점령이고 blue가 더 높은 경우
    {
        winType_f = GREEN2;
        winType_r = GREEN2;
        winType_b = WHITE;
    }
    else if (flag_player == 0 && Red.score == Blue.score) //미점령, 동점인경우
    {
        winType_f = GREEN2;
        winType_r = GREEN2;
        winType_b = GREEN2;
    }
    else //점령인 경우
    {
        winType_f = WHITE;
        winType_r = GREEN2;
        winType_b = GREEN2;
    }


    if (flag_player == 1) {
        strcpy(flag, "RED");
        color = RED1;
    }
    else if (flag_player == 2) {
        strcpy(flag, "BLUE");
        color = BLUE1;
    }
    else {
        strcpy(flag, "DROW");
        color = GRAY2;
    }

    textcolor(BROWN, BROWN);
    for (int i = y - 1; i < y + 10; i++)
    {
        for (int j = x - 2; j < x + 51; j++)
        {
            printxy(j, i, " ");
        }
    }
    textcolor(BLACK, GREEN2);
    for (int i = y; i < y + 9; i++)
    {
        for (int j = x; j < x + 49; j++)
        {
            printxy(j, i, " ");
        }
    }
    textcolor(BLACK, winType_f); printxy(x + 2, y + 1, "FLAG : ");
    textcolor(color, winType_f); printf("%s", flag);
    textcolor(BLACK, winType_r); printxy(x + 16, y + 1, "RED KILLS : ");
    textcolor(RED1, winType_r); printf("%0d", Red.score);
    textcolor(BLACK, winType_b); printxy(x + 32, y + 1, "BLUE KILLS : ");
    textcolor(BLUE1, winType_b); printf("%0d", Blue.score);
    textcolor(BLACK, GREEN2); printxy(x + 14, y + 4, " REPLAY  ==☞  { ① } ");
    textcolor(BLACK, GREEN2); printxy(x + 14, y + 6, " EXIT    ==☞  { ② } ");
}

int main(void)
{
START:
    //------------------------------------------------타이틀 화면--------------------------------------------------------//
    cls(BLACK, GRAY2);
    removeCursor();
    ChangefontSize(16, 8); //타이틀화면 폰트 사이즈
    system("mode con: cols=150 lines=40"); //타이틀화면 버퍼 사이즈

    PlaySound(TEXT("Spon2.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_LOOP); //게임 시작시 bgm재생

    srand(time(NULL));
    int low = 0; // 타이틀 캐릭터 움직임
    int title_frame = 15000; //속도
    int title_count = 0;
    title_title(19, 5);
    title_menu(50, 17);
    title_red_ball(40, 16);
    title_red_ball(34, 22);
    title_red_ball(40, 29);
    title_blue_ball(92, 16);
    title_blue_ball(99, 22);
    title_blue_ball(92, 29);
    while (1)
    {
        if (title_count % title_frame == 0) //캐릭터 움직임, 별들 번쩍거림
        {
            title_red(19, 15 + low * 1);
            low = !low;
            title_blue(119, 15 + low * 1);
            textcolor(MAGENTA1 + (rand() % 4), GRAY2); printxy(15, 2, "★");
            textcolor(MAGENTA1 + (rand() % 4), GRAY2); printxy(4, 4, "★");
            textcolor(MAGENTA1 + (rand() % 4), GRAY2); printxy(10, 10, "★");
            textcolor(MAGENTA1 + (rand() % 4), GRAY2); printxy(25, 7, "★");
            textcolor(MAGENTA1 + (rand() % 4), GRAY2); printxy(7, 30, "★");
            textcolor(MAGENTA1 + (rand() % 4), GRAY2); printxy(37, 34, "★");
            textcolor(MAGENTA1 + (rand() % 4), GRAY2); printxy(21, 38, "★");

            textcolor(MAGENTA1 + (rand() % 4), GRAY2); printxy(110, 1, "★");
            textcolor(MAGENTA1 + (rand() % 4), GRAY2); printxy(138, 2, "★");
            textcolor(MAGENTA1 + (rand() % 4), GRAY2); printxy(130, 10, "★");
            textcolor(MAGENTA1 + (rand() % 4), GRAY2); printxy(123, 7, "★");
            textcolor(MAGENTA1 + (rand() % 4), GRAY2); printxy(112, 30, "★");
            textcolor(MAGENTA1 + (rand() % 4), GRAY2); printxy(124, 34, "★");
            textcolor(MAGENTA1 + (rand() % 4), GRAY2); printxy(139, 38, "★");

            textcolor(MAGENTA1 + (rand() % 4), GRAY2); printxy(50, 36, "★");
            textcolor(MAGENTA1 + (rand() % 4), GRAY2); printxy(67, 38, "★");
            textcolor(MAGENTA1 + (rand() % 4), GRAY2); printxy(90, 36, "★");
            textcolor(MAGENTA1 + (rand() % 4), GRAY2); printxy(105, 38, "★");
        }


        if (kbhit() == 1)
        {
            char c1;
            c1 = getch(); // key 값을 읽는다
            if (c1 == '\0') // VS 의 NULL 값은 Error이다.
                continue;
            if (c1 == '1') //시작 버튼
            {
                break;
            }
            else if (c1 == '2') //설명 버튼
            {
                title_book(50, 17);
                getch();
                break;//시작하려면 아무키나
            }
            else if (c1 == '3') //종료 버튼
            {
                exit(0);
            }
        }
        title_count++;
    }
    textcolor(BLACK, WHITE);

    PlaySound(NULL, 0, 0); //게임 진입시 bgm종료
    //-------------------------------------------------게임 진입---------------------------------------------------------//
    srand(time(NULL));//랜덤 시드

    cls(BLACK, WHITE);
    ChangefontSize(3, 5); //게임 진입시 폰트 사이즈 변경
    system("mode con: cols=600 lines=190"); //게임 진입시 화면 창 변경
    init_Game();//게임 초기 세팅

    // 아무키나 입력시 시작
    Game_start();
    PlaySound(TEXT("Spon.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_LOOP); //게임 시작시 bgm재생
    //시간 세팅
    int run_time, start_time, remain_time, last_remain_time; //리플레이 하면 goto문 써야하나? 아니면 전역변수로 주면 됨
    last_remain_time = remain_time = time_out;
    start_time = time(NULL); //시작시간
    while (1) //게임 엔진
    {
        unsigned char ch;// 특수키 0xe0 을 입력받으려면 unsigned char 로 선언해야 함

        //시간 관리 부분--------------------------
        run_time = time(NULL) - start_time; //실행시간
        remain_time = time_out - run_time; //남은시간
        if (remain_time < last_remain_time) { // 시간이 변할때만 출력
            show_time(remain_time);
            last_remain_time = remain_time;
        }


        //랜덤 아이템 생성 부분--------------------
        if (run_time % 20 == 0) //아이템 콜라이더 생성, 20초마다 바뀜
        {
            item_spawn();
        }
        if (run_time % 3 == 0) //아이템 스위치on
        {
            for (int i = 0; i < 3; i++)
            {
                if (item_switch[i] == 1)
                {
                    drow_item(itemxy[i].y + deco_swt, itemxy[i].x, itemxy[i].val);
                    //deco_swt = !deco_swt; 아이템 위아래 움직이는데 별로임
                }
            }
        }
        //----------------------------------------



        if (kbhit() == 1) {  // 키보드가 눌려져 있으면
            char c1;
            c1 = getch(); // key 값을 읽는다
            if (c1 == '\0') // VS 의 NULL 값은 Error이다.
                continue;
            else
                ch = c1;
            if (ch == ESC) // ESC 누르면 프로그램 종료
                exit(0);
            if (ch == SPECIAL1) { // 만약 특수키
                // 예를 들어 UP key의 경우 0xe0 0x48 두개의 문자가 들어온다.
                ch = getch();
                // Player1은 방향키로 움직인다.

                switch (ch) {
                case UP2:
                case DOWN2:
                case LEFT2:
                case RIGHT2://player2만 방향 전환
                    player2(ch);//player1만 방향 전환
                    player1(0);
                    if (ch == LEFT2) Blue.arrow = -1; //플레이어2의 시선 왼쪽
                    if (ch == RIGHT2) Blue.arrow = 1; //플레이어2의 시선 오른쪽
                    break;
                default:// 방향 전환이 아니면
                    player1(0);
                    player2(0);
                }
            }
            else {
                // Player2은 AWSD 로 움직인다.

                switch (ch) {
                case UP:
                case DOWN:
                case LEFT:
                case RIGHT:
                    player1(ch);//player1만 방향 전환
                    player2(0);
                    if (ch == LEFT) Red.arrow = -1; //플레이어1의 시선 왼쪽
                    if (ch == RIGHT) Red.arrow = 1; //플레이어1의 시선 오른쪽
                    break;
                default: // 방향 전환이 아니면
                    player1(0);
                    player2(0);
                }
            }

            //(RED) p1 발사키를 눌렀다****************************************************************************
            if (ch == SHOT && Red.shots > 0) //발사키 오른쪽 숫자 1 눌려지면, 총알 있으면
            {
                int n = MAXBULLET - Red.shots;
                Red_ball[n].arrow = Red.arrow;
                Red_ball[n].newY = Red_ball[n].oldY = Red.newY + 5; //파이어볼 높이 조절가능(5:몸통쯤?)

                if (Red.arrow == 1) //캐릭터 방향이 오른쪽일때
                {
                    Red_ball[n].newX = Red_ball[n].oldX = (Red.newX + 4) + Red_ball[n].arrow * 2;
                }
                else if (Red.arrow == -1) //개릭터 방향이 왼쪽일때
                {
                    Red_ball[n].newX = Red_ball[n].oldX = (Red.newX - 6) + Red_ball[n].arrow * 2;
                }


                shot_red(n); //n번째 발사체 전달
                Red.shots--;
                shot_count(1); //플레이어1 남은 탄수
            }
            //(BLUE) p2 발사키를 눌렀다***************************************************************************
            if (ch == SHOT2 && Blue.shots > 0) //발사키 오른쪽 숫자 1 눌려지면, 총알 있으면
            {
                int n = MAXBULLET - Blue.shots;
                Blue_ball[n].arrow = Blue.arrow;
                Blue_ball[n].newY = Blue_ball[n].oldY = Blue.newY + 5; //파이어볼 높이 조절가능(5:몸통쯤?)

                if (Blue.arrow == 1) //캐릭터 방향이 오른쪽일때
                {
                    Blue_ball[n].newX = Blue_ball[n].oldX = (Blue.newX + 4) + Blue_ball[n].arrow * 2;
                }
                else if (Blue.arrow == -1) //개릭터 방향이 왼쪽일때
                {
                    Blue_ball[n].newX = Blue_ball[n].oldX = (Blue.newX - 6) + Blue_ball[n].arrow * 2;
                }


                shot_blue(n); //n번째 발사체 전달
                Blue.shots--;
                shot_count(2); //플레이어2 남은 탄수
            }//**************************************************************************************************
            //(RED) p1 스킬키를 눌렀다****************************************************************************
            if (ch == SKILL && Red.skiPow >= 3)
            {
                Red.skiPow = 0; //스킬 게이지 초기화
                print_skipow(1);
                use_skill(1); //스킬 발동
            }
            //(BLUE) p2 스킬키를 눌렀다***************************************************************************
            if (ch == SKILL2 && Blue.skiPow >= 3)
            {
                Blue.skiPow = 0; //스킬 게이지 초기화
                print_skipow(2);
                use_skill(2); //스킬 발동
            }
            //(RED) p1 아이템키를 눌렀다****************************************************************************
            if (ch == ITEM)
            {
                if (Red.item_slot == 6) //장전
                {
                    erase_item(2, 92);
                    Red.shots = MAXBULLET; //탄수 초기화
                    shot_count(1);
                }
                else if (Red.item_slot == 7) //회복
                {
                    erase_item(2, 92);
                    init_hp(1); //체력 초기화
                    print_hp(1);
                }
            }
            //(BLUE) p2 아이템키를 눌렀다***************************************************************************
            if (ch == ITEM2)
            {
                if (Blue.item_slot == 6) //장전
                {
                    erase_item(2, 292);
                    Blue.shots = MAXBULLET; //탄수 초기화
                    shot_count(2);
                }
                else if (Blue.item_slot == 7) //회복
                {
                    erase_item(2, 292);
                    init_hp(2); //체력 초기화
                    print_hp(2);
                }

            }
        }
        else
        {
            // keyboard 가 눌려지지 않으면 안 움직임: move_ch=1, 계속 움직임: move_ch=0
            player1(Red.move_ch);
            player2(Blue.move_ch);
        }

        //********************발사 여부가 O이면 작동*******************************************
        for (int i = 0; i < MAXBULLET; i++)
        {
            if (Red_ball[i].ball_cheak) shot_red(i);
            if (Blue_ball[i].ball_cheak) shot_blue(i);
        }
        //***********************************************************************************



        //(게임 매니저) 게임 상태정보 체크-----------------------------------------------------##
        if (Red.hp <= 0) //레드 사망
        {
            erasestar(Red.oldY, Red.oldX); //사망 시체 제거? 좀 잔인한데...
            Blue.score++; //레드사망 -> 블루점수 + 1
            show_score(2); //블루 스코어 최신화
            Red.skiPow++;//사망하면 스킬 게이지 상승
            print_skipow(1);//스킬 게이지 출력
            Red.oldX = Red.newX = 17, Red.oldY = Red.newY = 155;//리스폰
            init_hp(1); //체력 초기화
            Red.shots = MAXBULLET; //탄수 초기화
            shot_count(1);
        }

        if (Blue.hp <= 0) //블루 사망
        {
            erasestar(Blue.oldY, Blue.oldX); //사망 시체 제거? 좀 잔인한데...
            Red.score++; //블루사망 -> 레드점수 + 1
            show_score(1); //레드 스코어 최신화
            Blue.skiPow++;//사망하면 스킬 게이지 상승
            print_skipow(2);//스킬 게이지 출력
            Blue.oldX = Blue.newX = 277, Blue.oldY = Blue.newY = 155;//리스폰
            init_hp(2);//체력 초기화, 탄수 초기화
            Blue.shots = MAXBULLET; //탄수 초기화
            shot_count(2);
        }

        if (remain_time <= 0) //경기 시간 종료 (점령자 승리or 미점령시 킬 높은사람 승리)
        {
            //if 점령포인트 (0,1,2) 0:미점령 1:레드승리 2:블루승리
            //if(점령포인트 = 0) 3항 연산자 쓰면 될듯 킬수 더 높은사람 승리
            if (flag_player == 0)
            {
                if (Red.score > Blue.score) winner = 1; //레드 승리!     
                else if (Red.score < Blue.score) winner = 2; //블루 승리!
                else winner = 0; //무승부
            }
            else if (flag_player == 1) winner = 1; //레드 승리!                
            else if (flag_player == 2) winner = 2; //블루 승리!

            break; //게임을 빠져나옴 **(종료)**
        }
        if (Red.score == MAXSCORE || Blue.score == MAXSCORE) //목표 킬수 달성자 승리(킬수 조절해야할듯?)
        {
            if (Red.score > Blue.score) winner = 1; //레드 승리!     
            else if (Red.score < Blue.score) winner = 2; //블루 승리!
            else winner = 0; //무승부

            break; //게임을 빠져나옴 **(종료)**
        }
        //----------------------------------------------------------------------------------##   
        Sleep(DELAY);
        frame_count++;
    }

    PlaySound(NULL, 0, 0); //게임 종료 bgm종료
    //-------------------------------------------------종료 화면---------------------------------------------------------//
    cls(BLACK, GRAY2);
    ChangefontSize(16, 8); //종료화면 폰트 사이즈
    system("mode con: cols=150 lines=40");//종료화면 버퍼 사이즈

    print_winner(winner); //매개변수 winner
    end_menu(50, 20);

    title_red(19, 15);
    title_blue(119, 15);
    if (winner == 1)
    {
        PlaySound(TEXT("wow.wav"), NULL, SND_FILENAME | SND_ASYNC); //종료 bgm 한번 재생하고 끝
        textcolor(RED1, SKIN); printxy(19 + 2, 15 + 4, "  ♥  ♥");
        textcolor(BLACK, SKIN); printxy(119 + 2, 15 + 4, "ㅠ  ㅠ  ");
    }
    else if (winner == 2)
    {
        PlaySound(TEXT("wow.wav"), NULL, SND_FILENAME | SND_ASYNC); //종료 bgm 한번 재생하고 끝
        textcolor(BLUE1, SKIN); printxy(119 + 2, 15 + 4, "♥  ♥  ");
        textcolor(BLACK, SKIN); printxy(19 + 2, 15 + 4, "  ㅠ  ㅠ");
    }
    else  if (winner == 0)
    {
        PlaySound(TEXT("fail.wav"), NULL, SND_FILENAME | SND_ASYNC); //종료 bgm 한번 재생하고 끝
        textcolor(BLACK, SKIN); printxy(19 + 2, 15 + 4, "  ㅡ  ㅡ");
        textcolor(BLACK, SKIN); printxy(119 + 2, 15 + 4, "ㅡ  ㅡ  ");
    }


    while (1)
    {
        if (kbhit() == 1)
        {
            char c1;
            c1 = getch(); // key 값을 읽는다
            if (c1 == '\0') // VS 의 NULL 값은 Error이다.
                continue;
            if (c1 == '1') //리플레이 버튼
            {
                goto START;
            }
            else if (c1 == '2') //종료 버튼
            {
                exit(0);
            }
        }
    }

    getch();
    return 0;
}

/*---------수정일: 5/19--------------------*/
 //구조체로 구현하는게 나을듯? 
 //빨강 플레이어 위치 정보 배열에 저장(물체 접촉시 사용하기 위함)
 //콜라이더 생성함, 지우는거 해야함
 //프로그램 재정리 한번 해야할듯 배배꼬임

/*---------수정일: 5/20--------------------*/
 //콜라이더 생성 했음 지우는거 구현해야함
 //(블록이 아닌 구조물이 0이 되면 안됨 그건 아직 해결 못함)
 //콘솔크기랑 폰트 사이즈 런타임중 수정가능한지 체크하기(가능)
 //워프존 구현해야함


/*---------수정일: 5/21--------------------*/
 //워프 구현하기, (로프?)
 //발사체 함수 만들기, 아직 못함
 //대충 워프 구현은 함

/*---------수정일: 5/23--------------------*/
//발사체 만들기(미완)
//워프 수정하기(못함)

/*---------수정일: 5/24--------------------*/
//발사체 만들기(대략적으로 만듦)
//제한시간 만듦
//워프 수정하기(못함)

/*---------수정일: 5/25--------------------*/
//남은 탄수 표시하기(감소만 표시, 획득시 출력해야함)
//스킬 구현하기(스킬 인터페이스만 구현), 충돌 구현하기 0
//워프 수정하기(스폰만 수정함)

/*---------수정일: 5/26--------------------*/
//어떤 스킬할건지 정하고 구현하기
//워프구간 랜덤 구현하기(기능만 구현함 꾸며야함)
//승리기준 정하기, 점령 구현하기(승리기준 정함)
//점령하면 인터페이스에 정보 등록(완성)

/*---------수정일: 5/27~~  ----------------*/
//시험공부

/*---------수정일: 5/31--------------------*/
//교수님께서 총알 이동 n칸씩 하지말라 하심
//프레임 카운트로 속도 조절해야함
//내 프로그램은 프레임 카운트 적용이 힘듦을 알게됨
//스킬 구현하기(완성), 초기화면(X), 설명화면 만들기(X)

/*---------수정일: 6/01--------------------*/
//초기화면(완성), 설명화면 만들기(완성)
//종료화면 만들기(70% 완성) 꾸미기만 하면 될듯?
//다음시간에 아이템 추가하기

/*---------수정일: 6/01~~ ----------------*/
//시험공부

/*---------수정일: 6/05 ----------------*/
//캐릭터 바라보는 방향으로 그림(수정 함)
//타이틀화면 밋밋한 느낌(수정하면 좋을듯)
//엔딩화면도, 아이템 추가해야하고, 맵 꾸미기

/*---------수정일: 6/07 ----------------*/
//아이템 구현할 방법 구상

/*---------최종 수정: 6/12,13 ------------*/
//아이템 넣기
//맵 꾸미기
//bgm넣기



//아이템 획득 판정 아이디어
//캐릭터 콜라이더가 아이템 콜라이더를 지우면
//아이템 콜라이더를 지운 캐릭터 콜라이더 번호를 인식해서 
//아이템이 발동하게 한다
//if(아이템 좌표 바로위 == 캐릭터 콜라이더)
// {
//      아이템 지워지고
//      아이템을 먹느 캐릭터에게 능력 발동
// }
//
// 1. 비행(콜라이더 바 무한정으로 늘리기)
// 2. 꿀벌(날아가서 쏘기? 가능?)
// 3. 장전(포션)
// 4. 힐팩(먹으면 힐)
//

//https://jubrodev.tistory.com/3 
//https://ascii.co.uk/art
//참고하면 좋을 사이트


//                  /\           //
//                 /  \          //
//                |    |         //
//              --:'''':--       //
//                :'_' :         //
//                _:"":\___      //
//  ' '     ____.' :::     '._   //
// . *====<<=)           \    :  //
//  .  '     '-'-'\_      /'._.' //
//                  \====:_ ""   //
//                 .'     \\     //
//                :       :      //
//               /   :    \      //
//              :   .      '.    //
//              :  : :      :    //
//              :__:-:__.;--'    //

//           /\                  //
//          /  \                 //
//         |    |                // 
//       --:'''':--              //
//         : '_':                // 
//      ___/:"":_                //  
//   _.'     ::: '.____     ' '  // 
//  :    /           )=>>====* . // 
// '._.'/      _/'-'-'     '  .  //  
//   "" _:====\                  //  
//     \\     '.                 // 
//      :       :                // 
//      /    :   \               //
//    .'      .   :              // 
//    :      : :  :              //  
//    '--;.__:-:__:              // 



  //    ___________________   ___________________    
  //.-/|  78   ~~**~~      \ /      ~~**~~   79  |\-.
  //||||                    :                    ||||
  //||||   Dorothy asked    :   The Scarecrow    ||||
  //||||   the Scarecrow    :   answered "Some   ||||
  //||||   "How can you     :   people without   ||||
  //||||   talk if you      :   brains do an     ||||
  //||||   haven't got      :   awful lot        ||||
  //||||   a brain?"        :   of talking."     ||||
  //||||   She looked at    :   She replied,     ||||
  //||||   him puzzled.     :   "That's true."   ||||
  //||||                    :                    ||||
  //||||  The Wizard Of Oz  :    boba@wwa.com    ||||
  //||||___________________ : ___________________||||
  //||/====================\:/====================\||
  //`---------------------~___~--------------------''