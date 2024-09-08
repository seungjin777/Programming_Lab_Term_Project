#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//�Լ� �������

//Ŀ���Ⱥ��̰�
void removeCursor(void);

//�ؿ� 2���Լ��� ���� �𸣰���, â�̶� ���� ũ�� ����?, 3��° ��Ʈ ���� �Լ�, ����ũ�� ����
void ChangeScreenSize(HANDLE hnd, COORD NewSize);
void ChangeBuffSize(HANDLE hnd, COORD NewSize);
int ChangefontSize(int x, int y);

//��Ʈ����, ��Ƽ����� Ŭ����, ��ǥ�� �̵�, ���ڿ� ���, ���ϴ� ��ġ ���(@@���� x,y�� �Է�@@)
void textcolor(int fg_color, int bg_color);
void cls(int text_color, int bg_color);
void gotoxy(int x, int y);
void printxy(int x, int y, char* str);

//�⺻������Ʈ �ʹ迭��ġ, ��� �⺻�������̽�, ��ġ�� �� ���
void map_cls();
void default_map();
void default_interface();
void DrawMap();

//��ȣ�ۿ� ���� �Լ���(����, �����̵�)
void warp(int player);
void checkFlag(int player);
void use_skill(int player);

//player���� �Լ���
void Character_drow(int y, int x, int player);
void Character_collider(int y, int x, int num);
void erasestar(int y, int x);
void player1(unsigned char ch);
void player2(unsigned char ch);
void init_hp(int player);
void print_hp(int player);
void print_skipow(int player);

//�߻�ü ���� �Լ�
void shot_red(int n);
void shot_blue(int n);
void shot_count(int player);

//������ ���� �Լ�
void drow_item(int y, int x, int ITnum);
void erase_item(int y, int x);
void enter_slot(int player, int ITnum);
void item_spawn();
void hit_item(int y, int x, int player);
void use_item();

//���� ����, �ð� ���� �Լ���
void show_time(int remain_time);
void show_score(int player);
void init_Game();
void Game_start();

//Ÿ��Ʋȭ�� ���� �Լ�
void title_red(int x, int y);
void title_blue(int x, int y);
void title_book(int x, int y);
void title_title(int x, int y);
void title_menu(int x, int y);

//����ȭ�� ���� �Լ�
void print_winner(int player);
void end_menu(int x, int y);