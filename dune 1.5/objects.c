#include "dune.h"
#include <stdlib.h>
#include <malloc.h>
#include <math.h>

// �߰� ���⶧ 1/10�� worm, eagle, storm�� �̵��� ǥ���Ͽ��� ������ ���������� �ð��� ����
#define WORM_MOVE_PERIOD 2500
#define EAGLE_MOVE_PERIOD 2000
#define STORM_MOVE_PERIOD 1500

#define STORM_OCCUR_PERIOD (50000/TICK)	// Sandstorm�� �߻��ϴ� �ֱ�, 50��
#define STORM_LAST_TIME (10000/TICK)	// Sandstorm�� �����ϴ� �ð�, 10��
int sandstorm_id = -1;

int nobject = 0;
OBJECT_POOL objectPool[MAX_POOL_SIZE];
extern RESOURCE resource;


// ������ object�� �´� ���� ����
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
	case 'W':	// SandWorm �Ӹ�
		color = FG_RED | BG_BROWN;
		break;
	case 'w':	// SandWorm ����
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

// object pool�� object ���� �߰���
// layer���� ����� object�� �߰���

OBJECT* copy_object(OBJECT* src)
{
	OBJECT* obj = (OBJECT*)malloc(sizeof(OBJECT));
	if (obj != NULL) {
		*obj = *src;
	}
	return obj;
}

// object pool�� object �߰��ϰ�, �߰��� object �����͸� ��ȯ
OBJECT* add_object(int layer, OBJECT* obj)
{
	// object��ü�� ����
	if (nobject >= MAX_POOL_SIZE) {
		add_system_message("Object pool is full.");
		add_system_message("MAX_POOL_SIZE�� �����ؾ� �մϴ�.");
		return NULL;
	}
	// ������� ��쿡��, reource�� �˻�
	else if (obj->type == USER && resource.population + obj->population > resource.population_max) {
		add_system_message("�ִ� �α����� �ʰ��Ͽ����ϴ�.\n���Ҹ� �� �����ϱ� �ٶ��ϴ�.");
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
	obj.size = 2;	//ũ��
	obj.strength = 50; //������
	obj.consumed_time = 0;
	add_object(BASE_LAYER, copy_object(&obj));
}

void build_plate(POSITION pos)
{
	OBJECT obj = { 0 };
	obj.unit = PLATE;
	obj.repr = 'P';
	obj.pos = pos;
	obj.size = 2;	//ũ��
	obj.cost = 1; // �Ǽ����
	obj.strength = 0; //������
	obj.consumed_time = 0;
	add_object(BASE_LAYER, copy_object(&obj));
}

void build_dormitory(POSITION pos)
{
	OBJECT obj = { 0 };
	obj.unit = DORMITORY;
	obj.repr = 'D';
	obj.pos = pos;
	obj.size = 1;	//ũ��
	obj.cost = 2; // �Ǽ����
	obj.strength = 10; //������
	obj.consumed_time = obj.cost * 1000 / TICK;
	resource.population_max += 10; // �α� �ִ�ġ ������Ŵ (+10)
	add_object(GROUND_LAYER, copy_object(&obj));
}

void build_garage(POSITION pos)
{
	OBJECT obj = { 0 };
	obj.unit = GARAGE;
	obj.repr = 'G';
	obj.pos = pos;
	obj.size = 1;	//ũ��
	obj.cost = 4; // �Ǽ����
	obj.strength = 10; //������
	obj.consumed_time = obj.cost * 1000 / TICK;
	resource.spice_max += 10; // �����̽� �ִ� ���差 ���� (+10)
	add_object(GROUND_LAYER, copy_object(&obj));
}

void build_barracks(POSITION pos)
{
	OBJECT obj = { 0 };
	obj.type = USER;
	obj.unit = BARRACKS;
	obj.repr = 'K';
	obj.pos = pos;
	obj.size = 1;	//ũ��
	obj.cost = 4; // �Ǽ����
	obj.strength = 20; //������
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
	obj.size = 1;	//ũ��
	obj.cost = 5; // �Ǽ����
	obj.strength = 30; //������
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
	obj.size = 1;	//ũ��
	obj.cost = 3; // �Ǽ����
	obj.strength = 15; //������
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
	obj.size = 1;	//ũ��
	obj.cost = 5; // �Ǽ����
	obj.strength = 30; //������
	obj.consumed_time = obj.cost * 1000 / TICK;
	add_object(GROUND_LAYER, copy_object(&obj));
}


// �ʹ� ���� spice�� �����Ǵ� ���� �����ϱ� ���Ͽ� ���� ��ġ���� [-dist, dist] �ȿ� spice�� ����� ������
int check_near_spice(POSITION pos, int dist)
{
	int row_start = max(0, pos.row - dist), row_end = min(MAP_HEIGHT - 1, pos.row + dist);
	int col_start = max(0, pos.column - dist), col_end = min(MAP_WIDTH - 1, pos.column + dist);

	for (int r = row_start; r <= row_end; r++) {
		for (int c = col_start; c <= col_end; c++) {
			// spice�� layer 0�� ��ġ�ϰ� ����. '1'~'9'�� ���� ������ ����
			if (map[BASE_LAYER][r][c] >= '1' && map[BASE_LAYER][r][c] <= '9')
				return TRUE;
		}
	}
	return FALSE;
}

int add_spice(POSITION pos, int reserves)
{
	// layer 0, 1�� �ٸ� object�� ������ �ƹ��� spice �������� ����
	for (int l = 0; l < 2; l++) {
		if (map[l][pos.row][pos.column] != ' ' && map[l][pos.row][pos.column] != 0)
			return FALSE;
	}
	// �ݰ� [-5, 5]�� �簢�� �ȿ� �̹� spice�� ������ �������� ����
	if (check_near_spice(pos, 5))
		return FALSE;

	OBJECT obj = { 0 };
	obj.unit = SPICE;
	if (reserves < 0) {
		reserves = rand() % 9 + 1;
		add_system_fmessage("���差 %c�� �����̽��� �����Ǿ����ϴ�.", reserves + '0');
	}
	obj.spice = reserves;
	obj.repr = reserves + '0';
	obj.pos = pos;
	obj.size = 1;	//ũ��
	add_object(BASE_LAYER, copy_object(&obj));
	return TRUE;
}

void add_rock(POSITION pos, int size)
{
	OBJECT obj = { 0 };
	obj.unit = ROCK;
	obj.repr = 'R';
	obj.pos = pos;
	obj.size = size;	//ũ��
	add_object(BASE_LAYER, copy_object(&obj));
}

void add_harvester(USER_TYPE type, POSITION pos)
{
	OBJECT obj = { 0 };
	obj.unit = HARVESTER;
	if (type == USER) {
		obj.repr = 'h';
		// ������� base ��ġ
		obj.orig = (POSITION){ MAP_HEIGHT - 2, 0 };
	}
	else {
		obj.repr = 'H';
		// AI�� base ��ġ
		obj.orig = (POSITION){ 0, MAP_WIDTH - 2 };
	}
	obj.type = type;
	obj.pos = pos;
	obj.size = 1;	//ũ��
	obj.cost = 5;	// ������
	obj.population = 5; // �α���
	obj.strength = 70; //ü��
	obj.move_period = 2000 / TICK; // �̵��ֱ�
	obj.next_move_time = obj.move_period;
	add_object(UNIT_LAYER, copy_object(&obj)); // layer 2�� ����
}

void add_worm(POSITION pos)
{
	OBJECT obj = { 0 };
	obj.unit = SANDWORM;
	obj.repr = 'W';
	obj.pos = pos;
	obj.size = 1;	//ũ��
	obj.strength = INT_MAX; // ü��, ���Ѵ�
	obj.move_period = WORM_MOVE_PERIOD / TICK; // �̵��ֱ�
	obj.next_move_time = obj.move_period;
	obj.attack_period = 10000 / TICK; //�����ֱ�
	obj.attack_power = INT_MAX; // ���ݷ�, ���Ѵ�
	obj.vision = INT_MAX; // �þ�, ���Ѵ�
	add_object(UNIT_LAYER, copy_object(&obj)); // layer 2�� ����
}

void add_eagle(POSITION pos)
{
	OBJECT obj = { 0 };
	obj.unit = SANDEAGLE;
	obj.repr = 'E';
	obj.pos = pos;
	obj.size = 1;	//ũ��
	obj.strength = INT_MAX; // ü��, ���Ѵ�
	obj.move_period = EAGLE_MOVE_PERIOD / TICK; // �̵��ֱ�
	obj.next_move_time = obj.move_period;
	obj.vision = INT_MAX; // �þ�, ���Ѵ�
	add_object(SKY_LAYER, copy_object(&obj)); // layer 3�� ����
}

void add_storm(POSITION pos)
{
	OBJECT obj = { 0 };
	obj.unit = SANDSTORM;
	obj.repr = 'O';
	obj.pos = pos;
	obj.size = 2;	//ũ��
	obj.strength = INT_MAX; // ü��, ���Ѵ�
	obj.move_period = STORM_MOVE_PERIOD / TICK; // �̵��ֱ�
	obj.next_move_time = obj.move_period;
	obj.attack_period = 10000 / TICK; //�����ֱ�
	obj.attack_power = INT_MAX; // ���ݷ�, ���Ѵ�
	obj.vision = INT_MAX; // �þ�, ���Ѵ�
	add_object(SKY_LAYER, copy_object(&obj)); // layer 3�� ����
	sandstorm_id = nobject - 1;
}

void add_soldier(POSITION pos)
{
	OBJECT obj = { 0 };
	obj.type = USER;
	obj.unit = SOLDIER;
	obj.repr = 'S';
	obj.pos = pos;
	obj.size = 1;	//ũ��
	obj.cost = 1;
	obj.population = 1;
	obj.move_period = 1000 / TICK; // �̵��ֱ�
	obj.next_move_time = obj.move_period;
	obj.attack_power = 5; // ���ݷ�, ���Ѵ�
	obj.attack_period = 800 / TICK; //�����ֱ�
	obj.next_attack_time = obj.attack_period;
	obj.strength = 15; // ü��, ���Ѵ�
	obj.vision = 1; // �þ�, ���Ѵ�
	add_object(UNIT_LAYER, copy_object(&obj)); // layer 2�� ����
}

void add_fremen(POSITION pos)
{
	OBJECT obj = { 0 };
	obj.type = USER;
	obj.unit = FREMEN;
	obj.repr = 'M';
	obj.pos = pos;
	obj.size = 1;	//ũ��
	obj.cost = 5;
	obj.population = 2;
	obj.move_period = 400 / TICK; // �̵��ֱ�
	obj.next_move_time = obj.move_period;
	obj.attack_power = 15; // ���ݷ�, ���Ѵ�
	obj.attack_period = 200 / TICK; //�����ֱ�
	obj.next_attack_time = obj.attack_period;
	obj.strength = 25; // ü��, ���Ѵ�
	obj.vision = 8; // �þ�, ���Ѵ�
	add_object(UNIT_LAYER, copy_object(&obj)); // layer 2�� ����
}

OBJECT* add_fighter(POSITION pos)
{
	OBJECT obj = { 0 };
	obj.type = AI;
	obj.unit = FIGHTER;
	obj.repr = 'F';
	obj.pos = pos;
	obj.size = 1;	//ũ��
	obj.cost = 1;
	obj.population = 1;
	obj.move_period = 1200 / TICK; // �̵��ֱ�
	obj.next_move_time = obj.move_period;
	obj.attack_power = 6; // ���ݷ�, ���Ѵ�
	obj.attack_period = 600 / TICK; //�����ֱ�
	obj.next_attack_time = obj.attack_period;
	obj.strength = 10; // ü��, ���Ѵ�
	obj.vision = 1; // �þ�, ���Ѵ�
	return add_object(UNIT_LAYER, copy_object(&obj)); // layer 2�� ����
}
OBJECT* add_tank(POSITION pos)
{
	OBJECT obj = { 0 };
	obj.type = AI;
	obj.unit = TANK;
	obj.repr = 'T';
	obj.pos = pos;
	obj.size = 1;	//ũ��
	obj.cost = 12;
	obj.population = 5;
	obj.move_period = 3000 / TICK; // �̵��ֱ�
	obj.next_move_time = obj.move_period;
	obj.attack_power = 45; // ���ݷ�, ���Ѵ�
	obj.attack_period = 4000 / TICK; //�����ֱ�
	obj.next_attack_time = obj.attack_period;
	obj.strength = 60; // ü��, ���Ѵ�
	obj.vision = 4; // �þ�, ���Ѵ�
	return add_object(UNIT_LAYER, copy_object(&obj)); // layer 2�� ����
}

// �迭���� object�� �����ϰ�, �ʿ��ϸ� OBJECT ����ü�� ��ȯ
OBJECT remove_object(int id)
{
	OBJECT obj = { 0 };
	if (id < 0) return obj;
	obj = *(objectPool[id].obj);
	// object�� ���� �޸� �Ҵ�Ǿ��� ������, free ȣ��
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
	// SandWorm�� ������ ������.
	worm->nblock++;
	POSITION* block;
	if (worm->block == NULL)
		block = (POSITION*)malloc(worm->nblock * sizeof(POSITION));
	else
		block = (POSITION*)realloc(worm->block, worm->nblock * sizeof(POSITION));
	if (block != NULL) {
		worm->block = block;
	}
	// ������ ������Ű�µ� �����ϸ�, ũ�⸦ �״�� ����
	else worm->nblock--;
}

void worm_shorten(OBJECT* worm)
{
	// SandWorm�� ������ ������.
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


// objectPool�� ����� object���� �������� ������ �޸𸮸� ��ȯ
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

// �־��� �簢���� ��ǥ�� ���Ͽ� ��ġ�� TRUE, �׷��� ������ FALSE�� ��ȯ
int isPointIncluded(SMALL_RECT rect, POSITION pos)
{
	if (pos.row >= rect.Top && pos.row < rect.Bottom && pos.column >= rect.Left && pos.column < rect.Right)
		return TRUE;
	else return FALSE;
}

int isRectOverlapped(SMALL_RECT r1, SMALL_RECT r2)
{
	// Right�� Bottom�� ��ǥ�� ���Ե��� �ʱ� ������, ���Կ��θ� Ȯ���ϱ� ���ؼ��� =�� ���ԵǾ�� ��.
	if (r1.Left >= r2.Right || r1.Right <= r2.Left || r1.Top >= r2.Bottom || r1.Bottom <= r2.Top)
		return FALSE;
	else
		return TRUE;
}

// ���� ��ġ�� ��ġ�� object�� ��ȯ, ������ ��ġ�� ��쿡�� ���� layer�� object�� ��ȯ
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

// storm�� size=2�� ��쿡�� �簢���� �簢���� ��ġ�� ���� Ȯ����
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

// Ŀ���� ȭ���� ��ġ�� ������ �ְ�, object�� map�ȿ����� ������� ��ġ�� ������ �ֱ� ������
// ������ �ʿ���
int check_object_select(POSITION pos)
{
	// ��ũ�� ��ǥ�� map ��ǥ�� ������
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

	// layer 0�� ���� (plate)�̶��, worm�� ������ �������� �ʽ��ϴ�.
	if (map[BASE_LAYER][pos.row][pos.column] == 'p' || map[BASE_LAYER][pos.row][pos.column] == 'P')
		return;
	else if (is_over_plate(pos)) // worm�� �������� �ö󰡸� �������� �ʽ��ϴ�.
		return;

	// worm�� �Ӹ��� object�� ������ �� �ֽ��ϴ�.
	int selected = get_in_object_id(pos);

	if (is_generalUnit(selected)) {
		add_system_fmessage("%s��(��) (%d, %d)���� SandWorm���� �������ϴ�.",
			get_object_name(objectPool[selected].obj->repr), pos.row, pos.column);
		add_system_fmessage("%s��(��) objectPool���� �����մϴ�.",
			get_object_name(objectPool[selected].obj->repr));
		remove_object(selected);

		worm_lengthen(worm);
	}
}

int check_unit(int r, int c)
{
	// �Ϲ� ������ layer 2�� ����
	// Harvest, Soldier, freMen, Fighter, Tank�� ������ TRUE ����
	if (map[UNIT_LAYER][r][c] == 'h' || map[UNIT_LAYER][r][c] == 'H' ||
		map[UNIT_LAYER][r][c] == 'S' || map[UNIT_LAYER][r][c] == 'M' ||
		map[UNIT_LAYER][r][c] == 'F' || map[UNIT_LAYER][r][c] == 'T')
		return 1;
	else if (map[BASE_LAYER][r][c] == 'R' || map[UNIT_LAYER][r][c] == 'w' || map[UNIT_LAYER][r][c] == 'W')
		return -1;
	else  // �׷��� ������ FALSE ����
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

// SandWorm�� �̵� �����ϸ� TRUE, �̵� �Ұ��� ��쿡�� FALSE�� ��ȯ
int is_moveable(POSITION pos)
{
	int r = pos.row, c = pos.column;
	if (map[BASE_LAYER][r][c] != 'R' && map[UNIT_LAYER][r][c] != 'w' && map[UNIT_LAYER][r][c] != 'W') // �̵� ������ ��� ���� ���� �� ����
		return TRUE;
	else // Rock�̰ų� Worm�� ��쿡�� �̵� �Ұ�.
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
	// ���������� �����ؼ� object�� �߰��ϸ� ����� object�� �̵�
	// object�� ������ ���� Rock�� ������ ���̻� �������� �ʰ� �������� ����ó�� ó��.
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
	if (dir == 0) { // object�� �߰ߵ��� ������, ������ �� �ִ� ���� �߿��� �ϳ��� ����
		int available[4];
		int navail = check_moveable_dir(available, pos);
		if (navail == 0) return pos; // worm�� ������ ����.. �Ф�
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
		add_system_fmessage("%s��(��) (%d, %d)���� SandStorm���� �Ҹ�Ǿ����ϴ�.",
			get_object_name(objectPool[selected].obj->repr), newp.row, newp.column);
		add_system_fmessage("%s��(��) objectPool���� �����մϴ�.",
			get_object_name(objectPool[selected].obj->repr));
		remove_object(selected);
	}
	else if (is_building(selected)) {
		decrease_strength(selected);
		add_system_fmessage("%s��(��) SandStorm�� ���� �������� 1/2�� ��ҵǾ����ϴ�.",
			get_object_name(objectPool[selected].obj->repr));
		if (objectPool[selected].obj->strength <= 0) {
			add_system_fmessage("%s�� �������� 0�̾ �ǹ��� �Ҹ�Ǿ����ϴ�.",
				get_object_name(objectPool[selected].obj->repr));
			remove_object(selected);
		}
	}

}

void worm_move(OBJECT* obj)
{
	//���� ����� object�� ��ǥ�� ã��
	POSITION newp = decide_next_move(obj->pos);

	// �̵��� �Ұ����ϸ�
	if (newp.row == obj->pos.row && newp.column == obj->pos.column) {
		add_system_fmessage("������ �� ���� ���� �ڸ�(%d,%d)���� ����ϰڽ��ϴ�.", newp.row, newp.column);
		return;
	}

	// ���ο� ��ǥ�� �̵��� ������ 0 && 0�� �ƴϰ�, �̵� �����ϰ�, �� �ȿ� ������
	if (is_moveable(newp)) {
		// �̵��ϱ����� Spice�� ����
		if (rand() % 100 < 10) {	// 10%Ȯ���� Spice�� ����
			if (add_spice(obj->pos, -1)) {
				worm_shorten(obj); // Bonus 1
			}
		}

		// worm�� ��ü�� ������ ���, action�� ����
		worm_action(obj, newp);

		// worm�� ���̰� 1���� ũ��, ��ĭ�� ������ �̵���.
		if (obj->nblock > 0) {
			for (int i = obj->nblock - 1; i > 0; i--) {
				obj->block[i] = obj->block[i - 1];
			}
			// ����� ó����, �ٷ� �̵��ϱ����� ��ǥ�� ����
			obj->block[0] = obj->pos;
		}
		// �̵��ϴ� ���ο� ��ǥ�� ����
		obj->pos = newp;
	}
	// worm�� ���ο� ��ǥ�� map�� ��� ��� �̵����� ����

}

void object_move()
{
	static int sandstorm_period, sandstorm_last = STORM_LAST_TIME;
	extern CURSOR cursor;

	// Bonus 2)
	// ���� �ð��� ������ storm�� �Ҹ��
	if (--sandstorm_last <= 0 && sandstorm_id != -1) {
		remove_object(sandstorm_id);
		sandstorm_id = -1;
		sandstorm_period = STORM_OCCUR_PERIOD;
		add_system_message("�縷��ǳ�� �Ҹ�Ǿ����ϴ�.");
	}
	// ������ �ð��� �߻��ϸ� storm�� �߻���
	else if (sandstorm_id == -1) {
		if (--sandstorm_period <= 0) {
			extern POSITION storm_occur_position;
			add_storm(storm_occur_position);
			sandstorm_last = STORM_LAST_TIME;
			add_system_message("�縷��ǳ�� �����Ǿ����ϴ�.");
		}
		else if (sandstorm_period % 500 == 0) { // 5�ʸ��� �ѹ��� ��ǳ �߻� �ð��� �˷���, ��ǳ�� �׻� ������ ��ҿ��� �߻���
			// ����׸� ���� �縷��ǳ �˸�
			// add_system_fmessage("�縷��ǳ�� %d�� �� �߻��� �����Դϴ�.", sandstorm_period/100);
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
			// ���� ��ǥ�� ���� ��ǥ�� Ŀ���� ��ġ�� Ŀ�� ������ ����
			if (before.row == pos.row && before.column == pos.column ||
				obj->pos.row == pos.row && obj->pos.column == pos.column)
				update_cursor_info(&cursor);
			obj->next_move_time = obj->move_period;
		}
	}
}