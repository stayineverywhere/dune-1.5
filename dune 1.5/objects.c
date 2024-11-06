#include "dune.h"
#include <stdlib.h>
#include <malloc.h>
#include <math.h>

int nobject = 0;
OBJECT_POOL objectPool[MAX_POOL_SIZE];

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

void add_object(int layer, OBJECT* obj)
{
	// object��ü�� ����
	if (nobject >= MAX_POOL_SIZE) {
		add_system_message("Object pool is full.");
		add_system_message("MAX_POOL_SIZE�� �����ؾ� �մϴ�.");
		return;
	}
	objectPool[nobject].layer = layer;
	objectPool[nobject++].obj = obj;
}

// �迭���� object�� �����ϰ�, �ʿ��ϸ� OBJECT ����ü�� ��ȯ
OBJECT remove_object(int id)
{
	OBJECT obj = { 0 };
	if (id < 0) return obj;
	obj = *(objectPool[id].obj);
	// ���� object�� �迭 ������ �ƴϰ�, ���� �޸� �Ҵ��� ��쿡 ������ free ȣ��
	if (objectPool[id].obj != NULL)
		free(objectPool[id].obj);
	for (int i = id; i < nobject - 1; i++) {
		objectPool[i] = objectPool[i + 1];
	}
	nobject--;
	return obj;
}

// �ʹ� ���� spice�� �����Ǵ� ���� �����ϱ� ���Ͽ� ���� ��ġ���� [-dist, dist] �ȿ� spice�� ����� ������
int check_near_spice(POSITION pos, int dist)
{
	int row_start = max(0, pos.row - dist), row_end = min(MAP_HEIGHT - 1, pos.row + dist);
	int col_start = max(0, pos.column - dist), col_end = min(MAP_WIDTH - 1, pos.column + dist);

	for (int r = row_start; r <= row_end; r++) {
		for (int c = col_start; c <= col_end; c++) {
			// spice�� layer 0�� ��ġ�ϰ� ����. '1'~'9'�� ���� ������ ����
			if (map[0][r][c] >= '1' && map[0][r][c] <= '9')
				return TRUE;
		}
	}
	return FALSE;
}

void worm_longer(OBJECT* worm)
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

void worm_shorter(OBJECT* worm)
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

int add_spice(POSITION pos)
{
	// layer 0�� �ٸ� object�� ������ �ƹ��� spice �������� ����
	if (map[0][pos.row][pos.column] != ' ' && map[0][pos.row][pos.column] != 0)
		return FALSE;
	// �ݰ� [-5, 5]�� �簢�� �ȿ� �̹� spice�� ������ �������� ����
	if (check_near_spice(pos, 5))
		return FALSE;

	OBJECT* obj = (OBJECT*)malloc(sizeof(OBJECT));
	if (obj != NULL) {
		obj->repr = rand() % 9 + '1'; // '1' ~ '9'
		obj->size = 1;
		obj->pos = pos;

		add_object(0, obj);
		add_system_fmessage("���差 %c�� �����̽��� �����Ǿ����ϴ�.", obj->repr);
	}
	return TRUE;
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
int isIncluded(SMALL_RECT rect, POSITION pos)
{
	if (pos.row >= rect.Top && pos.row < rect.Bottom && pos.column >= rect.Left && pos.column < rect.Right)
		return TRUE;
	else return FALSE;
}

// ���� ��ġ�� ��ġ�� object�� ��ȯ
int get_object_id(POSITION pos)
{
	for (int i = 0; i < nobject; i++) {
		OBJECT* obj = objectPool[i].obj;
		SMALL_RECT rect = { obj->pos.column, obj->pos.row,obj->pos.column + obj->size,
							obj->pos.row + obj->size };
		//add_system_fmessage("%c : %d, %d, %d, %d", obj->repr, obj->pos.row, obj->pos.column, pos->row, pos->column);
		if (isIncluded(rect, pos)) {
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
	POSITION mod_pos = { pos.row - rectMap.Top, pos.column - rectMap.Left };
	return get_object_id(mod_pos);
}

int is_generalUnit(int id)
{
	if (id < 0) return FALSE;
	OBJECT* obj = objectPool[id].obj;
	if (obj->repr == 'h' || obj->repr == 'H' ||
		obj->repr == 'S' || obj->repr == 'M' ||
		obj->repr == 'F' || obj->repr == 'T')
		return TRUE;
	else
		return FALSE;
}

void worm_action(OBJECT* worm, POSITION pos)
{
	int selected = get_object_id(pos);
	if (is_generalUnit(selected)) {
		add_system_fmessage("%s��(��) (%d, %d)���� SandWorm���� �������ϴ�.",
			get_object_name(objectPool[selected].obj->repr), pos.row, pos.column);
		add_system_fmessage("%s��(��) objectPool���� �����մϴ�.",
			get_object_name(objectPool[selected].obj->repr));
		remove_object(selected);

		worm_longer(worm);
	}
}

int check_unit(int r, int c)
{
	// �Ϲ� ������ layer 1�� ����
	// Harvest, Soldier, freMen, Fighter, Tank�� ������ TRUE ����
	if (map[1][r][c] == 'h' || map[1][r][c] == 'H' ||
		map[1][r][c] == 'S' || map[1][r][c] == 'M' ||
		map[1][r][c] == 'F' || map[1][r][c] == 'T')
		return 1;
	else if (map[0][r][c] == 'R' || map[1][r][c] == 'w' || map[1][r][c] == 'W')
		return -1;
	else  // �׷��� ������ FALSE ����
		return 0;
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
	if (dir == 0) { // object�� �߰ߵ��� ������, �������� 4�������� �̵�
		dir = rand() % 4 + 1;
	}
	switch (dir) {
	case 1:	// up
		if (pos.row > 0)
			pos.row--;
		break;
	case 2: // right
		if (pos.column < MAP_WIDTH - 1)
			pos.column++;
		break;
	case 3: // down
		if (pos.row < MAP_HEIGHT - 1)
			pos.row++;
		break;
	case 4: // left
		if (pos.column > 0)
			pos.column--;
		break;
	}
	return pos;
}


// SandWorm�� �̵� �����ϸ� TRUE, �̵� �Ұ��� ��쿡�� FALSE�� ��ȯ
int is_moveable(POSITION pos)
{
	int r = pos.row, c = pos.column;
	if (map[0][r][c] != 'R' && map[1][r][c] != 'w' && map[1][r][c] != 'W') // �̵� ������ ��� ���� ���� �� ����
		return TRUE;
	else // Rock�̰ų� Worm�� ��쿡�� �̵� �Ұ�.
		return FALSE;
}

void worm_move(OBJECT* obj)
{
	int nr = 0, nc = 0;

	//���� ����� object�� ��ǥ�� ã��
	POSITION newp = decide_next_move(obj->pos);
	
	// �̵��� �Ұ����ϸ�
	if (newp.row == obj->pos.row && newp.column == obj->pos.column) {
		add_system_fmessage("������ �� ���� ���� �ڸ����� ����ϰڽ��ϴ�.");
		return;
	}

	// ���ο� ��ǥ�� �̵��� ������ 0 && 0�� �ƴϰ�, �̵� �����ϰ�, �� �ȿ� ������
	if (is_moveable(newp)) {
		// �̵��ϱ����� Spice�� ����
		if (rand() % 100 < 10) {	// 10%Ȯ���� Spice�� ����
			if (add_spice(obj->pos))
				worm_shorter(obj);
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
	for (int n = 0; n < nobject; n++)
	{
		OBJECT* obj = objectPool[n].obj;
		obj->next_move_time--;
		switch (obj->repr) {
		case 'W':
			if (obj->next_move_time == 0) {
				worm_move(obj);
				obj->next_move_time = obj->move_period;
			}
			break;
		}
	}
}
