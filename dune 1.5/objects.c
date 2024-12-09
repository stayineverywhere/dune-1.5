#include "dune.h"
#include <stdlib.h>
#include <malloc.h>
#include <math.h>

// 중간 제출때 1/10로 worm, eagle, storm의 이동을 표현하였기 때문에 정상적으로 시간을 복원
#define WORM_MOVE_PERIOD 2500
#define EAGLE_MOVE_PERIOD 2000
#define STORM_MOVE_PERIOD 1500

#define STORM_OCCUR_PERIOD (50000/TICK)	// Sandstorm이 발생하는 주기, 50초
#define STORM_LAST_TIME (10000/TICK)	// Sandstorm이 존재하는 시간, 10초
int sandstorm_id = -1;

int nobject = 0;
OBJECT_POOL objectPool[MAX_POOL_SIZE];
extern RESOURCE resource;


// 각각의 object에 맞는 색을 지정
WORD setObjectColor(char repr)
{
	WORD color;
	switch (repr) {
	case '0': case'1': case'2': case'3': case'4': case'5': case'6': case'7': case'8': case'9':
		color = FG_WHITE | BG_MAGENTA;
		break;
	case 'b': case 'h': case 'S': case 'M':
		color = FG_WHITE | BG_BLUE;
		break;
	case 'B': case 'H': case 'F': case 'T':
		color = FG_WHITE | BG_RED;
		break;
	case 'E':	// SandEagle
		color = FG_RED | BG_CYAN;
		break;
	case 'O':	// SandStorm
		color = FG_RED | BG_LIGHTGRAY;
		break;
	case 'R':
		color = FG_BLACK | BG_DARKGRAY;
		break;
	case 'W':	// SandWorm 머리
		color = FG_RED | BG_BROWN;
		break;
	case 'w':	// SandWorm 꼬리
		color = FG_YELLOW | BG_BROWN;
		break;
	case 'P':
		color = FG_WHITE | BG_LIGHTGRAY;
		break;
	default:
		return 0;
	}
	return setTextAttribute(color);
}

// object pool에 object 들을 추가함
// layer별로 선언된 object를 추가함

OBJECT* copy_object(OBJECT* src)
{
	OBJECT* obj = (OBJECT*)malloc(sizeof(OBJECT));
	if (obj != NULL) {
		*obj = *src;
	}
	return obj;
}

// object pool에 object 추가하고, 추가된 object 포인터를 반환
OBJECT* add_object(int layer, OBJECT* obj)
{
	// object전체를 저장
	if (nobject >= MAX_POOL_SIZE) {
		add_system_message("Object pool is full.");
		add_system_message("MAX_POOL_SIZE를 증가해야 합니다.");
		return NULL;
	}
	// 사용자인 경우에만, reource를 검사
	else if (obj->type == USER && resource.population + obj->population > resource.population_max) {
		add_system_message("최대 인구수를 초과하였습니다.\n숙소를 더 생성하기 바랍니다.");
		add_system_fmessage("current: %d, obj: %d, max: %d",
			resource.population, obj->population, resource.population_max);
		return NULL;
	}
	objectPool[nobject].layer = layer;
	objectPool[nobject++].obj = obj;
	resource.population += obj->population;
	return obj;
}


void build_base(USER_TYPE type, POSITION pos)
{
	OBJECT obj = { 0 };
	obj.unit = BASE;
	if (type == USER) {
		obj.repr = 'b';
	}
	else
		obj.repr = 'B';
	obj.type = type;
	obj.pos = pos;
	obj.size = 2;	//크기
	obj.strength = 50; //내구도
	obj.consumed_time = 0;
	add_object(BASE_LAYER, copy_object(&obj));
}

void build_plate(POSITION pos)
{
	OBJECT obj = { 0 };
	obj.unit = PLATE;
	obj.repr = 'P';
	obj.pos = pos;
	obj.size = 2;	//크기
	obj.cost = 1; // 건설비용
	obj.strength = 0; //내구도
	obj.consumed_time = 0;
	add_object(BASE_LAYER, copy_object(&obj));
}

void build_dormitory(POSITION pos)
{
	OBJECT obj = { 0 };
	obj.unit = DORMITORY;
	obj.repr = 'D';
	obj.pos = pos;
	obj.size = 1;	//크기
	obj.cost = 2; // 건설비용
	obj.strength = 10; //내구도
	obj.consumed_time = obj.cost * 1000 / TICK;
	resource.population_max += 10; // 인구 최대치 증가시킴 (+10)
	add_object(GROUND_LAYER, copy_object(&obj));
}

void build_garage(POSITION pos)
{
	OBJECT obj = { 0 };
	obj.unit = GARAGE;
	obj.repr = 'G';
	obj.pos = pos;
	obj.size = 1;	//크기
	obj.cost = 4; // 건설비용
	obj.strength = 10; //내구도
	obj.consumed_time = obj.cost * 1000 / TICK;
	resource.spice_max += 10; // 스파이스 최대 저장량 증가 (+10)
	add_object(GROUND_LAYER, copy_object(&obj));
}

void build_barracks(POSITION pos)
{
	OBJECT obj = { 0 };
	obj.type = USER;
	obj.unit = BARRACKS;
	obj.repr = 'K';
	obj.pos = pos;
	obj.size = 1;	//크기
	obj.cost = 4; // 건설비용
	obj.strength = 20; //내구도
	obj.consumed_time = obj.cost * 1000 / TICK;
	add_object(GROUND_LAYER, copy_object(&obj));
}

void build_shelter(POSITION pos)
{
	OBJECT obj = { 0 };
	obj.type = USER;
	obj.unit = SHELTER;
	obj.repr = 'L';
	obj.pos = pos;
	obj.size = 1;	//크기
	obj.cost = 5; // 건설비용
	obj.strength = 30; //내구도
	obj.consumed_time = obj.cost * 1000 / TICK;
	add_object(GROUND_LAYER, copy_object(&obj));
}

void build_arena(POSITION pos)
{
	OBJECT obj = { 0 };
	obj.type = AI;
	obj.unit = ARENA;
	obj.repr = 'A';
	obj.pos = pos;
	obj.size = 1;	//크기
	obj.cost = 3; // 건설비용
	obj.strength = 15; //내구도
	obj.consumed_time = obj.cost * 1000 / TICK;
	add_object(GROUND_LAYER, copy_object(&obj));
}

void build_factory(POSITION pos)
{
	OBJECT obj = { 0 };
	obj.type = AI;
	obj.unit = FACTORY;
	obj.repr = 'C';
	obj.pos = pos;
	obj.size = 1;	//크기
	obj.cost = 5; // 건설비용
	obj.strength = 30; //내구도
	obj.consumed_time = obj.cost * 1000 / TICK;
	add_object(GROUND_LAYER, copy_object(&obj));
}


// 너무 많은 spice가 생성되는 것을 방지하기 위하여 현재 위치에서 [-dist, dist] 안에 spice가 없어야 생성함
int check_near_spice(POSITION pos, int dist)
{
	int row_start = max(0, pos.row - dist), row_end = min(MAP_HEIGHT - 1, pos.row + dist);
	int col_start = max(0, pos.column - dist), col_end = min(MAP_WIDTH - 1, pos.column + dist);

	for (int r = row_start; r <= row_end; r++) {
		for (int c = col_start; c <= col_end; c++) {
			// spice는 layer 0에 위치하고 있음. '1'~'9'의 값을 가지고 있음
			if (map[BASE_LAYER][r][c] >= '1' && map[BASE_LAYER][r][c] <= '9')
				return TRUE;
		}
	}
	return FALSE;
}

int add_spice(POSITION pos, int reserves)
{
	// layer 0, 1에 다른 object가 있으면 아무런 spice 생성하지 않음
	for (int l = 0; l < 2; l++) {
		if (map[l][pos.row][pos.column] != ' ' && map[l][pos.row][pos.column] != 0)
			return FALSE;
	}
	// 반경 [-5, 5]의 사각형 안에 이미 spice가 있으면 생성하지 않음
	if (check_near_spice(pos, 5))
		return FALSE;

	OBJECT obj = { 0 };
	obj.unit = SPICE;
	if (reserves < 0) {
		reserves = rand() % 9 + 1;
		add_system_fmessage("매장량 %c인 스파이스가 생성되었습니다.", reserves + '0');
	}
	obj.spice = reserves;
	obj.repr = reserves + '0';
	obj.pos = pos;
	obj.size = 1;	//크기
	add_object(BASE_LAYER, copy_object(&obj));
	return TRUE;
}

void add_rock(POSITION pos, int size)
{
	OBJECT obj = { 0 };
	obj.unit = ROCK;
	obj.repr = 'R';
	obj.pos = pos;
	obj.size = size;	//크기
	add_object(BASE_LAYER, copy_object(&obj));
}

void add_harvester(USER_TYPE type, POSITION pos)
{
	OBJECT obj = { 0 };
	obj.unit = HARVESTER;
	if (type == USER) {
		obj.repr = 'h';
		// 사용자의 base 위치
		obj.orig = (POSITION){ MAP_HEIGHT - 2, 0 };
	}
	else {
		obj.repr = 'H';
		// AI의 base 위치
		obj.orig = (POSITION){ 0, MAP_WIDTH - 2 };
	}
	obj.type = type;
	obj.pos = pos;
	obj.size = 1;	//크기
	obj.cost = 5;	// 생산비용
	obj.population = 5; // 인구수
	obj.strength = 70; //체력
	obj.move_period = 2000 / TICK; // 이동주기
	obj.next_move_time = obj.move_period;
	add_object(UNIT_LAYER, copy_object(&obj)); // layer 2에 생산
}

void add_worm(POSITION pos)
{
	OBJECT obj = { 0 };
	obj.unit = SANDWORM;
	obj.repr = 'W';
	obj.pos = pos;
	obj.size = 1;	//크기
	obj.strength = INT_MAX; // 체력, 무한대
	obj.move_period = WORM_MOVE_PERIOD / TICK; // 이동주기
	obj.next_move_time = obj.move_period;
	obj.attack_period = 10000 / TICK; //공격주기
	obj.attack_power = INT_MAX; // 공격력, 무한대
	obj.vision = INT_MAX; // 시야, 무한대
	add_object(UNIT_LAYER, copy_object(&obj)); // layer 2에 생산
}

void add_eagle(POSITION pos)
{
	OBJECT obj = { 0 };
	obj.unit = SANDEAGLE;
	obj.repr = 'E';
	obj.pos = pos;
	obj.size = 1;	//크기
	obj.strength = INT_MAX; // 체력, 무한대
	obj.move_period = EAGLE_MOVE_PERIOD / TICK; // 이동주기
	obj.next_move_time = obj.move_period;
	obj.vision = INT_MAX; // 시야, 무한대
	add_object(SKY_LAYER, copy_object(&obj)); // layer 3에 생산
}

void add_storm(POSITION pos)
{
	OBJECT obj = { 0 };
	obj.unit = SANDSTORM;
	obj.repr = 'O';
	obj.pos = pos;
	obj.size = 2;	//크기
	obj.strength = INT_MAX; // 체력, 무한대
	obj.move_period = STORM_MOVE_PERIOD / TICK; // 이동주기
	obj.next_move_time = obj.move_period;
	obj.attack_period = 10000 / TICK; //공격주기
	obj.attack_power = INT_MAX; // 공격력, 무한대
	obj.vision = INT_MAX; // 시야, 무한대
	add_object(SKY_LAYER, copy_object(&obj)); // layer 3에 생산
	sandstorm_id = nobject - 1;
}

void add_soldier(POSITION pos)
{
	OBJECT obj = { 0 };
	obj.type = USER;
	obj.unit = SOLDIER;
	obj.repr = 'S';
	obj.pos = pos;
	obj.size = 1;	//크기
	obj.cost = 1;
	obj.population = 1;
	obj.move_period = 1000 / TICK; // 이동주기
	obj.next_move_time = obj.move_period;
	obj.attack_power = 5; // 공격력, 무한대
	obj.attack_period = 800 / TICK; //공격주기
	obj.next_attack_time = obj.attack_period;
	obj.strength = 15; // 체력, 무한대
	obj.vision = 1; // 시야, 무한대
	add_object(UNIT_LAYER, copy_object(&obj)); // layer 2에 생산
}

void add_fremen(POSITION pos)
{
	OBJECT obj = { 0 };
	obj.type = USER;
	obj.unit = FREMEN;
	obj.repr = 'M';
	obj.pos = pos;
	obj.size = 1;	//크기
	obj.cost = 5;
	obj.population = 2;
	obj.move_period = 400 / TICK; // 이동주기
	obj.next_move_time = obj.move_period;
	obj.attack_power = 15; // 공격력, 무한대
	obj.attack_period = 200 / TICK; //공격주기
	obj.next_attack_time = obj.attack_period;
	obj.strength = 25; // 체력, 무한대
	obj.vision = 8; // 시야, 무한대
	add_object(UNIT_LAYER, copy_object(&obj)); // layer 2에 생산
}

OBJECT* add_fighter(POSITION pos)
{
	OBJECT obj = { 0 };
	obj.type = AI;
	obj.unit = FIGHTER;
	obj.repr = 'F';
	obj.pos = pos;
	obj.size = 1;	//크기
	obj.cost = 1;
	obj.population = 1;
	obj.move_period = 1200 / TICK; // 이동주기
	obj.next_move_time = obj.move_period;
	obj.attack_power = 6; // 공격력, 무한대
	obj.attack_period = 600 / TICK; //공격주기
	obj.next_attack_time = obj.attack_period;
	obj.strength = 10; // 체력, 무한대
	obj.vision = 1; // 시야, 무한대
	return add_object(UNIT_LAYER, copy_object(&obj)); // layer 2에 생산
}
OBJECT* add_tank(POSITION pos)
{
	OBJECT obj = { 0 };
	obj.type = AI;
	obj.unit = TANK;
	obj.repr = 'T';
	obj.pos = pos;
	obj.size = 1;	//크기
	obj.cost = 12;
	obj.population = 5;
	obj.move_period = 3000 / TICK; // 이동주기
	obj.next_move_time = obj.move_period;
	obj.attack_power = 45; // 공격력, 무한대
	obj.attack_period = 4000 / TICK; //공격주기
	obj.next_attack_time = obj.attack_period;
	obj.strength = 60; // 체력, 무한대
	obj.vision = 4; // 시야, 무한대
	return add_object(UNIT_LAYER, copy_object(&obj)); // layer 2에 생산
}

// 배열에서 object를 삭제하고, 필요하면 OBJECT 구조체를 반환
OBJECT remove_object(int id)
{
	OBJECT obj = { 0 };
	if (id < 0) return obj;
	obj = *(objectPool[id].obj);
	// object는 동적 메모리 할당되었기 때문에, free 호출
	if (objectPool[id].obj != NULL)
		free(objectPool[id].obj);
	for (int i = id; i < nobject - 1; i++) {
		objectPool[i] = objectPool[i + 1];
	}
	nobject--;
	resource.population -= obj.population;
	return obj;
}


// Bonus 1)
void worm_lengthen(OBJECT* worm)
{
	// SandWorm의 꼬리를 증가함.
	worm->nblock++;
	POSITION* block;
	if (worm->block == NULL)
		block = (POSITION*)malloc(worm->nblock * sizeof(POSITION));
	else
		block = (POSITION*)realloc(worm->block, worm->nblock * sizeof(POSITION));
	if (block != NULL) {
		worm->block = block;
	}
	// 꼬리를 증가시키는데 실패하면, 크기를 그대로 유지
	else worm->nblock--;
}

void worm_shorten(OBJECT* worm)
{
	// SandWorm의 꼬리를 증가함.
	if (worm->nblock == 0)return;
	else if (worm->nblock == 1) {
		worm->nblock = 0;
		free(worm->block);
		worm->block = NULL;
	}
	else if (worm->block != NULL) {
		worm->nblock--;
		POSITION* block = (POSITION*)realloc(worm->block, worm->nblock * sizeof(POSITION));
		if (block != NULL)
			worm->block = block;
		else
			worm->nblock++;
	}
}


// objectPool에 저장된 object에서 동적으로 생성된 메모리를 반환
void free_objectPool()
{
	for (int i = 0; i < nobject; i++) {
		OBJECT* obj = objectPool[i].obj;
		if (obj != NULL) {
			if (obj->nblock > 0 && obj->block != NULL) {
				free(obj->block);
				obj->block = NULL;
			}
			free(obj);
			objectPool[i].obj = NULL;
		}
	}
}

// 주어진 사각형과 좌표를 비교하여 겹치면 TRUE, 그렇지 않으면 FALSE를 반환
int isPointIncluded(SMALL_RECT rect, POSITION pos)
{
	if (pos.row >= rect.Top && pos.row < rect.Bottom && pos.column >= rect.Left && pos.column < rect.Right)
		return TRUE;
	else return FALSE;
}

int isRectOverlapped(SMALL_RECT r1, SMALL_RECT r2)
{
	// Right와 Bottom은 좌표가 포함되지 않기 때문에, 포함여부를 확인하기 위해서는 =이 포함되어야 함.
	if (r1.Left >= r2.Right || r1.Right <= r2.Left || r1.Top >= r2.Bottom || r1.Bottom <= r2.Top)
		return FALSE;
	else
		return TRUE;
}

// 현재 위치와 겹치는 object를 반환, 동일한 위치인 경우에는 높은 layer의 object를 반환
int get_in_object_id(POSITION pos)
{
	int max_layer = -1;
	int id = -1;
	for (int i = 0; i < nobject; i++) {
		OBJECT* obj = objectPool[i].obj;
		SMALL_RECT rect = { obj->pos.column, obj->pos.row,
							obj->pos.column + obj->size, obj->pos.row + obj->size };
		//add_system_fmessage("%c : %d, %d, %d, %d", obj->repr, obj->pos.row, obj->pos.column, pos->row, pos->column);
		if (isPointIncluded(rect, pos) && objectPool[i].layer > max_layer) {
			max_layer = objectPool[i].layer;
			id = i;
		}
	}
	return id;
}

int is_over_plate(POSITION pos)
{
	for (int i = 0; i < nobject; i++) {
		OBJECT* obj = objectPool[i].obj;
		if (obj->unit != PLATE) continue;
		SMALL_RECT rect = { obj->pos.column, obj->pos.row,
							obj->pos.column + obj->size, obj->pos.row + obj->size };
		//add_system_fmessage("%c : %d, %d, %d, %d", obj->repr, obj->pos.row, obj->pos.column, pos->row, pos->column);
		if (isPointIncluded(rect, pos)) {
			return TRUE;
		}
	}
	return FALSE;
}

// storm과 size=2인 경우에는 사각형과 사각형이 겹치는 지를 확인함
int get_overlapped_id(SMALL_RECT obj_rect)
{
	for (int i = 0; i < nobject; i++) {
		OBJECT* obj = objectPool[i].obj;
		SMALL_RECT rect = { obj->pos.column, obj->pos.row,
							obj->pos.column + obj->size, obj->pos.row + obj->size };
		if (obj->unit != SANDSTORM && obj->unit != SANDEAGLE && obj->unit != SANDSTORM &&
			isRectOverlapped(rect, obj_rect)) {
			return i;
		}
	}
	return -1;
}

// 커서는 화면의 위치를 가지고 있고, object는 map안에서의 상대적인 위치를 가지고 있기 때문에
// 보정이 필요함
int check_object_select(POSITION pos)
{
	// 스크린 좌표와 map 좌표를 보정함
	return get_in_object_id(cvt_map_position(pos));
}


int is_generalUnit(int id)
{
	if (id < 0) return FALSE;
	OBJECT* obj = objectPool[id].obj;
	if (obj->unit >= HARVESTER && obj->unit <= TANK)
		return TRUE;
	else
		return FALSE;
}

int is_building(int id)
{
	if (id < 0) return FALSE;
	OBJECT* obj = objectPool[id].obj;
	if (obj->unit >= PLATE && obj->unit <= FACTORY)
		return TRUE;
	else
		return FALSE;
}

void worm_action(OBJECT* worm, POSITION pos)
{

	// layer 0가 장판 (plate)이라면, worm은 유닛을 공격하지 않습니다.
	if (map[BASE_LAYER][pos.row][pos.column] == 'p' || map[BASE_LAYER][pos.row][pos.column] == 'P')
		return;
	else if (is_over_plate(pos)) // worm이 장판위로 올라가면 공격하지 않습니다.
		return;

	// worm은 머리만 object를 공격할 수 있습니다.
	int selected = get_in_object_id(pos);

	if (is_generalUnit(selected)) {
		add_system_fmessage("%s이(가) (%d, %d)에서 SandWorm에게 잡혔습니다.",
			get_object_name(objectPool[selected].obj->repr), pos.row, pos.column);
		add_system_fmessage("%s을(를) objectPool에서 삭제합니다.",
			get_object_name(objectPool[selected].obj->repr));
		remove_object(selected);

		worm_lengthen(worm);
	}
}

int check_unit(int r, int c)
{
	// 일반 유닛은 layer 2에 있음
	// Harvest, Soldier, freMen, Fighter, Tank를 만나면 TRUE 리턴
	if (map[UNIT_LAYER][r][c] == 'h' || map[UNIT_LAYER][r][c] == 'H' ||
		map[UNIT_LAYER][r][c] == 'S' || map[UNIT_LAYER][r][c] == 'M' ||
		map[UNIT_LAYER][r][c] == 'F' || map[UNIT_LAYER][r][c] == 'T')
		return 1;
	else if (map[BASE_LAYER][r][c] == 'R' || map[UNIT_LAYER][r][c] == 'w' || map[UNIT_LAYER][r][c] == 'W')
		return -1;
	else  // 그렇지 않으면 FALSE 리턴
		return 0;
}

void move_step(POSITION* pos, DIRECTION dir, int size)
{
	switch (dir) {
	case 1:	// up
		if (pos->row > 0)
			pos->row--;
		break;
	case 2: // right
		if (pos->column < MAP_WIDTH - size)
			pos->column++;
		break;
	case 3: // down
		if (pos->row < MAP_HEIGHT - size)
			pos->row++;
		break;
	case 4: // left
		if (pos->column > 0)
			pos->column--;
		break;
	}
}

// SandWorm이 이동 가능하면 TRUE, 이동 불가인 경우에는 FALSE를 반환
int is_moveable(POSITION pos)
{
	int r = pos.row, c = pos.column;
	if (map[BASE_LAYER][r][c] != 'R' && map[UNIT_LAYER][r][c] != 'w' && map[UNIT_LAYER][r][c] != 'W') // 이동 가능한 경우 웜이 먹을 수 있음
		return TRUE;
	else // Rock이거나 Worm인 경우에는 이동 불가.
		return FALSE;
}


int check_moveable_dir(int moveable[], POSITION pos)
{
	int d = 0;
	for (int i = 1; i <= 4; i++) {
		POSITION new_pos = pos;
		move_step(&new_pos, i, 1);
		if (is_moveable(new_pos))
			moveable[d++] = i;
	}
	return d;
}

POSITION decide_next_move(POSITION pos)
{
	DIRECTION dir = 0; // staty, up, right, down, left
	int dir_dist[5] = { -1,-1,-1,-1,-1 }, i;
	// 동서남북을 관측해서 object를 발견하면 가까운 object로 이동
	// object를 만나기 전에 Rock이 있으면 더이상 진행하지 않고 벽끝까지 간것처럼 처리.
	// up
	for (i = pos.row - 1; i >= 0; i--) {
		int check = check_unit(i, pos.column);
		if (check == -1) { i = -1; break; }
		else if (check == 1) break;
	}
	if (i >= 0)
		dir_dist[1] = pos.row - i;
	// right
	for (i = pos.column + 1; i < MAP_WIDTH; i++) {
		int check = check_unit(pos.row, i);
		if (check == -1) { i = MAP_WIDTH; break; }
		else if (check == 1) break;
	}
	if (i < MAP_WIDTH)
		dir_dist[2] = i - pos.column;
	// down
	for (i = pos.row + 1; i < MAP_HEIGHT; i++) {
		int check = check_unit(i, pos.column);
		if (check == -1) { i = MAP_HEIGHT; break; }
		else if (check == 1) break;
	}
	if (i < MAP_HEIGHT)
		dir_dist[3] = i - pos.row;
	// left
	for (i = pos.column - 1; i >= 0; i--) {
		int check = check_unit(pos.row, i);
		if (check == -1) { i = -1; break; }
		else if (check == 1) break;
	}
	if (i >= 0)
		dir_dist[4] = pos.column - i;

	int min_dist = 1000;
	for (i = 1; i < 5; i++) {
		if (min_dist > dir_dist[i] && dir_dist[i] != -1) {
			min_dist = dir_dist[i];
			dir = i;
		}
	}
	if (dir == 0) { // object가 발견되지 않으며, 움직일 수 있는 방향 중에서 하나를 선택
		int available[4];
		int navail = check_moveable_dir(available, pos);
		if (navail == 0) return pos; // worm이 구석에 갇힘.. ㅠㅠ
		dir = available[rand() % navail];
	}
	move_step(&pos, dir, 1);

	return pos;
}


// Bonus 2)
void eagle_move(OBJECT* obj)
{
	POSITION newp = obj->pos;
	DIRECTION dir = rand() % 4 + 1;

	move_step(&newp, dir, obj->size);
	obj->pos = newp;
}

void decrease_strength(int selected)
{
	OBJECT* obj = objectPool[selected].obj;
	obj->strength /= 2;
}
void storm_move(OBJECT* obj)
{
	POSITION newp = obj->pos;
	DIRECTION dir = rand() % 4 + 1;

	move_step(&newp, dir, obj->size);
	obj->pos = newp;

	int selected = get_overlapped_id((SMALL_RECT) {
		newp.column, newp.row,
			newp.column + obj->size, newp.row + obj->size
	});

	if (is_generalUnit(selected)) {
		add_system_fmessage("%s이(가) (%d, %d)에서 SandStorm에게 소멸되었습니다.",
			get_object_name(objectPool[selected].obj->repr), newp.row, newp.column);
		add_system_fmessage("%s을(를) objectPool에서 삭제합니다.",
			get_object_name(objectPool[selected].obj->repr));
		remove_object(selected);
	}
	else if (is_building(selected)) {
		decrease_strength(selected);
		add_system_fmessage("%s이(가) SandStorm과 겹쳐 내구성이 1/2로 축소되었습니다.",
			get_object_name(objectPool[selected].obj->repr));
		if (objectPool[selected].obj->strength <= 0) {
			add_system_fmessage("%s의 내구성이 0이어서 건물이 소멸되었습니다.",
				get_object_name(objectPool[selected].obj->repr));
			remove_object(selected);
		}
	}

}

void worm_move(OBJECT* obj)
{
	//가장 가까운 object의 좌표를 찾음
	POSITION newp = decide_next_move(obj->pos);

	// 이동이 불가능하면
	if (newp.row == obj->pos.row && newp.column == obj->pos.column) {
		add_system_fmessage("움직일 수 없어 현재 자리(%d,%d)에서 대기하겠습니다.", newp.row, newp.column);
		return;
	}

	// 새로운 좌표의 이동의 변경이 0 && 0이 아니고, 이동 가능하고, 맵 안에 있으면
	if (is_moveable(newp)) {
		// 이동하기전에 Spice를 생성
		if (rand() % 100 < 10) {	// 10%확률로 Spice를 생성
			if (add_spice(obj->pos, -1)) {
				worm_shorten(obj); // Bonus 1
			}
		}

		// worm이 객체와 만나는 경우, action을 취함
		worm_action(obj, newp);

		// worm의 길이가 1보다 크면, 한칸씩 앞으로 이동함.
		if (obj->nblock > 0) {
			for (int i = obj->nblock - 1; i > 0; i--) {
				obj->block[i] = obj->block[i - 1];
			}
			// 블록의 처음은, 바로 이동하기전의 좌표를 저장
			obj->block[0] = obj->pos;
		}
		// 이동하는 새로운 좌표로 변경
		obj->pos = newp;
	}
	// worm의 새로운 좌표가 map을 벗어난 경우 이동하지 않음

}

void object_move()
{
	static int sandstorm_period, sandstorm_last = STORM_LAST_TIME;
	extern CURSOR cursor;

	// Bonus 2)
	// 일정 시간이 지나면 storm이 소멸됨
	if (--sandstorm_last <= 0 && sandstorm_id != -1) {
		remove_object(sandstorm_id);
		sandstorm_id = -1;
		sandstorm_period = STORM_OCCUR_PERIOD;
		add_system_message("사막태풍이 소멸되었습니다.");
	}
	// 일정한 시간이 발생하면 storm이 발생됨
	else if (sandstorm_id == -1) {
		if (--sandstorm_period <= 0) {
			extern POSITION storm_occur_position;
			add_storm(storm_occur_position);
			sandstorm_last = STORM_LAST_TIME;
			add_system_message("사막태풍이 생성되었습니다.");
		}
		else if (sandstorm_period % 500 == 0) { // 5초마다 한번씩 태풍 발생 시간을 알려줌, 태풍은 항상 동일한 장소에서 발생함
			// 디버그를 위해 사막태풍 알림
			// add_system_fmessage("사막태풍이 %d초 후 발생할 예정입니다.", sandstorm_period/100);
		}
	}
	POSITION pos = cvt_map_position(cursor.pos);

	for (int n = 0; n < nobject; n++)
	{
		OBJECT* obj = objectPool[n].obj;
		POSITION before = obj->pos;
		obj->next_move_time--;
		obj->next_attack_time--;
		if (obj->next_move_time == 0) {
			switch (obj->unit) {
			case SANDSTORM:	// sand storm
				storm_move(obj);
				break;
			case SANDEAGLE:	// sand eagle
				eagle_move(obj);
				break;
			case SANDWORM:	// sand worm
				worm_move(obj);
				break;
			case HARVESTER:
			case FREMEN:
			case SOLDIER:
			case FIGHTER:
			case TANK:
				if (obj->cmd == c_harvest) harvest(obj);
				else if (obj->cmd == c_move) unit_move(obj);
				else if (obj->cmd == c_patrol) unit_patrol(obj);
				break;
			}
			// 이전 좌표나 변경 좌표가 커서와 겹치면 커서 정보를 갱신
			if (before.row == pos.row && before.column == pos.column ||
				obj->pos.row == pos.row && obj->pos.column == pos.column)
				update_cursor_info(&cursor);
			obj->next_move_time = obj->move_period;
		}
	}
}