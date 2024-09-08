#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//함수 헤더파일

//커서안보이게
void removeCursor(void);

//밑에 2개함수는 뭔지 모르겠음, 창이랑 버퍼 크기 조절?, 3번째 폰트 조절 함수, 버퍼크기 변경
void ChangeScreenSize(HANDLE hnd, COORD NewSize);
void ChangeBuffSize(HANDLE hnd, COORD NewSize);
int ChangefontSize(int x, int y);

//폰트색상, 액티브버퍼 클리어, 좌표로 이동, 문자열 출력, 원하는 위치 출력(@@주의 x,y로 입력@@)
void textcolor(int fg_color, int bg_color);
void cls(int text_color, int bg_color);
void gotoxy(int x, int y);
void printxy(int x, int y, char* str);

//기본오브젝트 맵배열배치, 상단 기본인터페이스, 배치된 맵 출력
void map_cls();
void default_map();
void default_interface();
void DrawMap();

//상호작용 관련 함수들(워프, 순간이동)
void warp(int player);
void checkFlag(int player);
void use_skill(int player);

//player관련 함수들
void Character_drow(int y, int x, int player);
void Character_collider(int y, int x, int num);
void erasestar(int y, int x);
void player1(unsigned char ch);
void player2(unsigned char ch);
void init_hp(int player);
void print_hp(int player);
void print_skipow(int player);

//발사체 관련 함수
void shot_red(int n);
void shot_blue(int n);
void shot_count(int player);

//아이템 관련 함수
void drow_item(int y, int x, int ITnum);
void erase_item(int y, int x);
void enter_slot(int player, int ITnum);
void item_spawn();
void hit_item(int y, int x, int player);
void use_item();

//게임 세팅, 시간 관련 함수들
void show_time(int remain_time);
void show_score(int player);
void init_Game();
void Game_start();

//타이틀화면 관련 함수
void title_red(int x, int y);
void title_blue(int x, int y);
void title_book(int x, int y);
void title_title(int x, int y);
void title_menu(int x, int y);

//종료화면 관리 함수
void print_winner(int player);
void end_menu(int x, int y);