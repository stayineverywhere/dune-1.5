#pragma once

#include <Windows.h>
#include <conio.h>

#define N_LAYER			4	// Base, Plate, Spice, Rock은 Layer0, 건물은 Layer1, 
// Unit은 Layer2, Sandstorm, SandEagle은 Layer3로 변경
#define MAP_WIDTH		80
#define MAP_HEIGHT		18
#define MAX_OBJECT		100
#define MAX_POOL_SIZE	1000

#define RESET_OBJECT	-100
#define TICK 10

#define BASE_LAYER 0
#define GROUND_LAYER 1
#define UNIT_LAYER 2
#define SKY_LAYER 3

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
	k_H, k_I, k_J, k_K, k_L, k_M, k_N,
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

typedef enum { NEUTRAL, USER, AI } USER_TYPE;
typedef enum {
	NONE, BASE, SPICE, PLATE, ROCK, DORMITORY, GARAGE, BARRACKS, SHELTER, ARENA, FACTORY,
	HARVESTER, FREMEN, SOLDIER, FIGHTER, TANK, SANDWORM, SANDEAGLE, SANDSTORM
} UNIT_TYPE;

typedef enum {
	c_none, c_wait,
	c_prod_harvestor, c_prod_soldier, c_prod_fremen, c_prod_fighter, c_produce_tank,
	c_harvest, c_move, c_patrol,
	c_build_cmd, // 서브 명령어 실행
	c_build_plate, c_build_dorm, c_build_garage,
	c_build_barracks, c_build_shelter, c_build_arena, c_build_factory
} COMMAND_TYPE;

typedef struct {
	USER_TYPE type;			// 사용자 또는 AI, 문자로 구분하기 어려울때를 위하여 미리 할당
	UNIT_TYPE unit;
	POSITION pos;			// 현재 위치 (position)
	POSITION orig;			// harvester의 경우 base 위치, patrol의 경우 orig와 dest를 왕복
	POSITION dest;			// 목적지 (destination)
	int dir;				// patrol이나 harvest의 경우 움직이는 방향, 0 : orig, 1: dest
	COMMAND_TYPE cmd;

	int size;				// 크기
	int spice;				// 스파이스를 채굴한 경우 소지된 양을 포함
	char repr;				// 화면에 표시할 문자 (representation)
	int consumed_time;		// 건물에서 unit을 생성하는데 걸리는 시간, unit 마다 생성 시간이 다름
	int move_period;		// 몇 ms 마다 한 칸 움직이는지 를 뜻함
	int next_move_time;	// 다음에 움직일 시간
	int cost;				// 건설/생산 비용
	int population;			// 인구수
	int attack_power;		// 공격력
	int attack_period;		// 공격주기
	int next_attack_time;
	int strength;			// 체력/내구도
	int vision;				// 시야

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
#define FG_GRREN	2
#define FG_CYAN		3	// AQUA
#define FG_RED		4
#define FG_MAGENTA	5	// MAGENTA
#define FG_BROWN	6
#define FG_LIGHTGRAY	7
#define FG_DARKGRAY	8
#define FG_LIGHTBLUE	9
#define FG_LIGHTGREEN	10
#define FG_LIGHTCYAN	11
#define FG_LIGHTRED	12
#define FG_PINK		13
#define FG_YELLOW	14
#define FG_WHITE	15

#define BG_BLACK	0
#define BG_BLUE		(1<<4)
#define BG_GRREN	(2<<4)
#define BG_CYAN		(3<<4)	// AQUA
#define BG_RED		(4<<4)
#define BG_MAGENTA	(5<<4)	// MAGENTA
#define BG_BROWN	(6<<4)
#define BG_LIGHTGRAY	(7<<4)
#define BG_DARKGRAY	(8<<4)
#define BG_LIGHTBLUE	(9<<4)
#define BG_LIGHTGREEN	(10<<4)
#define BG_LIGHTCYAN	(11<<4)
#define BG_LIGHTRED	(12<<4)
#define BG_PINK		(13<<4)
#define BG_YELLOW	(14<<4)
#define BG_WHITE	(15<<4)


// display.c
extern HANDLE hStdOut, hHiddenBuffer;
extern COORD screenSize;
extern CHAR_INFO* frameData;
extern SMALL_RECT rectResourceMesg, rectMap, rectSysMesg, rectStatusMsg, rectCommand;

extern WORD reverseAttribute(WORD attr);

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
extern void display(RESOURCE resource, char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH], CURSOR cursor, int selectedObj, int clock);
extern WORD _getTextAttribute();
extern WORD getTextAttribute();
extern WORD _setTextAttribute(WORD wAttribute);
extern WORD setTextAttribute(WORD wAttribute);
extern void clearBuffer();
extern void fillLine(int x, int y, int length, wchar_t ch, WORD attr);
extern void clearConsole();
extern void initBuffers();
extern void flushBuffer();

// resource.c
extern void display_resource(RESOURCE resource);

// sysmesg.c
extern void add_system_fmessage(char* mesg, ...);

extern void add_system_message(char* mesg);
extern void display_system_message();

// objinfo.c
extern char* get_object_name(char repr);
extern void add_overlap_messages(SMALL_RECT rect, char* msg);
extern void display_status(int selObj);
extern void display_command(int selObj);
extern void display_desert_information();

extern void clear_messages();

// key.c
extern KEY get_key();

// cursor.c
extern void init_cursor(CURSOR* cursor);
extern void increase_cursor_size(CURSOR* cursor);
extern void decrease_cursor_size(CURSOR* cursor);
extern void display_cursor(CURSOR cursor);
extern void move_cursor(CURSOR* cursor, KEY key, int diff);
extern void update_cursor(CURSOR* cursor);


// map.c
extern void put_object(char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH]);

extern void init_map(char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH]);
extern void hakonen_map(char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH]);
extern POSITION cvt_map_position(POSITION pos);

extern void display_map(char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH]);
extern int check_empty(POSITION pos, int size);
extern int check_plate(POSITION pos);

// objects.c
extern int nobject;
extern OBJECT_POOL objectPool[MAX_POOL_SIZE];
OBJECT* copy_object(OBJECT* src);

extern OBJECT* add_object(int layer, OBJECT* obj);
extern OBJECT remove_object(int id);
extern int isPointIncluded(SMALL_RECT rect, POSITION pos);
extern int isRectOverlapped(SMALL_RECT r1, SMALL_RECT r2);
extern void build_base(USER_TYPE type, POSITION pos);
extern void build_plate(POSITION pos);
extern void build_dormitory(POSITION pos);
extern void build_garage(POSITION pos);

extern void build_barracks(POSITION pos);
extern void build_shelter(POSITION pos);
extern void build_arena(POSITION pos);
extern void build_factory(POSITION pos);
extern int add_spice(POSITION pos, int reserves);
extern void add_rock(POSITION pos, int size);
extern void add_harvester(USER_TYPE type, POSITION pos);
extern void add_worm(POSITION pos);
extern void add_eagle(POSITION pos);
extern void add_storm(POSITION pos);
extern void add_soldier(POSITION pos);
extern void add_fremen(POSITION pos);
extern OBJECT* add_fighter(POSITION pos);
extern OBJECT* add_tank(POSITION pos);
extern void free_objectPool();

extern WORD setObjectColor(char repr);

extern int check_object_select(POSITION pos);
extern void object_move();


// inouttro.c

extern void Intro();
extern void Outtro();

// objcmd.c
extern void harvest(OBJECT* obj);
extern void unit_move(OBJECT* obj);
extern void unit_patrol(OBJECT* obj);
extern COMMAND_TYPE fetch_unit_command(int selected, KEY key);
extern void invoke_unit_command(COMMAND_TYPE cmd, int selected, POSITION pos);
extern void execute_unit_command();
extern void show_building_command();
extern COMMAND_TYPE fetch_build_command(KEY key);
extern void invoke_build_command(COMMAND_TYPE cmd, POSITION pos);
extern void execute_build_command();
extern void cancel_build_command(int selected);
extern int is_pos_command(COMMAND_TYPE command);

// engine.c
extern void update_cursor_info(CURSOR* cursor);
