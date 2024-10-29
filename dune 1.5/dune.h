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
	int spice; 			// 현재 보유한 스파이스
	int spice_max; 		// 스파이스 최대 저장량
	int population; 	// 현재 인구 수
	int population_max; // 수용 가능한 인구 수
} RESOURCE;

typedef struct {
	POSITION pos;		// 현재 위치 (position)
	POSITION dest;		// 목적지 (destination)
	int size;			// 크기
	char repr;			// 화면에 표시할 문자 (representation)
	int  move_period;	// 몇 ms 마다 한 칸 움직이는지 를 뜻함
	int  next_move_time;	// 다음에 움직일 시간
	int nblock;			// sand word의 경우, block수가 증가할 수 있음. 그때 사용
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
extern CHAR_INFO* frameData;
extern SMALL_RECT rectResourceMesg, rectMap, rectSysMesg, rectStatusMsg, rectCommand;

extern void cursorOff();
extern void cursorOn();
extern void _gotoXY(HANDLE handle, int x, int y);
extern void gotoXY(int x, int y);
extern CHAR_INFO _getCharXY(HANDLE handle, int x, int y);
extern CHAR_INFO getCharXY(int x, int y);
extern void _putCharXY(HANDLE handle, int x, int y, wchar_t ch);
extern void putCharXY(int x, int y, wchar_t ch);
extern void _putStringXY(HANDLE handle, int x, int y, char* str);
extern void putStringXY(int x, int y, char* str);
extern void init_parameters();
extern void display(RESOURCE resource, char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH], CURSOR cursor, int selectedObj);
extern WORD _getTextAttribute();
extern WORD getTextAttribute();
extern WORD _setTextAttribute(WORD wAttribute);
extern WORD setTextAttribute(WORD wAttribute);
extern void clearBuffer();
extern void fillLine(int x, int y, int length, wchar_t ch, WORD attr);
extern void clearConsole();
extern void initBuffers();
extern void flushBuffer();


// inouttro.c

extern void Intro();
extern void Outtro();