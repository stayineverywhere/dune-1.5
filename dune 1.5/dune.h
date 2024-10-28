#pragma once

#include <Windows.h>
#include <conio.h>

#define N_LAYER 2
#define MAP_WIDTH 80
#define MAP_HEIGHT 18
#define MAX_OBJECT 20

#define TICK 10

extern char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH];

typedef struct {
	int row, column;
} POSITION;

typedef struct {
	POSITION pos;
	CHAR_INFO charInfo[4];
	int size;
} CURSOR;

typedef enum {
	k_none = 0, k_escape, k_space, k_A, k_B, k_C, k_D, k_E, k_F, k_G,
	k_H, k_I, k_J, k_L, k_M, k_N,
	k_O, k_P, k_Q, k_R, k_S, k_T, k_U, k_V, k_W, k_X, k_Y, k_Z,
	k_quit,
	k_up, k_right, k_down, k_left,
	k_undef
} KEY;

typedef enum {
	d_stay = 0,
	d_up, d_right, d_down, d_left
} DIRECTION;

typedef struct {
	int spice; 			// ���� ������ �����̽�
	int spice_max; 		// �����̽� �ִ� ���差
	int population; 	// ���� �α� ��
	int population_max; // ���� ������ �α� ��
} RESOURCE;

typedef struct {
	POSITION pos;		// ���� ��ġ (position)
	POSITION dest;		// ������ (destination)
	int size;			// ũ��
	char repr;			// ȭ�鿡 ǥ���� ���� (representation)
	int  move_period;	// �� ms ���� �� ĭ �����̴��� �� ����
	int  next_move_time;	// ������ ������ �ð�
	int nblock;			// sand word�� ���, block���� ������ �� ����. �׶� ���
	POSITION* block;
} OBJECT;

typedef struct _objects {
	int layer;
	OBJECT* obj;
} OBJECT_POOL;

//
// color table

#define FG_BLACK	0
#define FG_BLUE		1


#define BG_BLUE		(1<<4)
#define BG_YELLOW	(14<<4)

// display.c
extern HANDLE hStdOut, hHiddenBuffer;
extern COORD screenSize;

// inouttro.c

extern void Intro();
extern void Outtro();