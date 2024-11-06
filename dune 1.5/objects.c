#include "dune.h"
#include <stdlib.h>
#include <malloc.h>
#include <math.h>

int nobject = 0;
OBJECT_POOL objectPool[MAX_POOL_SIZE];

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

void add_object(int layer, OBJECT* obj)
{
	// object전체를 저장
	if (nobject >= MAX_POOL_SIZE) {
		add_system_message("Object pool is full.");
		add_system_message("MAX_POOL_SIZE를 증가해야 합니다.");
		return;
	}
	objectPool[nobject].layer = layer;
	objectPool[nobject++].obj = obj;
}

// 배열에서 object를 삭제하고, 필요하면 OBJECT 구조체를 반환
OBJECT remove_object(int id)
{
	OBJECT obj = { 0 };
	if (id < 0) return obj;
	obj = *(objectPool[id].obj);
	// 만약 object가 배열 생성이 아니고, 동적 메모리 할당인 경우에 삭제시 free 호출
	if (objectPool[id].obj != NULL)
		free(objectPool[id].obj);
	for (int i = id; i < nobject - 1; i++) {
		objectPool[i] = objectPool[i + 1];
	}
	nobject--;
	return obj;
}

// 너무 많은 spice가 생성되는 것을 방지하기 위하여 현재 위치에서 [-dist, dist] 안에 spice가 없어야 생성함
int check_near_spice(POSITION pos, int dist)
{
	int row_start = max(0, pos.row - dist), row_end = min(MAP_HEIGHT - 1, pos.row + dist);
	int col_start = max(0, pos.column - dist), col_end = min(MAP_WIDTH - 1, pos.column + dist);

	for (int r = row_start; r <= row_end; r++) {
		for (int c = col_start; c <= col_end; c++) {
			// spice는 layer 0에 위치하고 있음. '1'~'9'의 값을 가지고 있음
			if (map[0][r][c] >= '1' && map[0][r][c] <= '9')
				return TRUE;
		}
	}
	return FALSE;
}

void worm_longer(OBJECT* worm)
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

void worm_shorter(OBJECT* worm)
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

int add_spice(POSITION pos)
{
	// layer 0에 다른 object가 있으면 아무런 spice 생성하지 않음
	if (map[0][pos.row][pos.column] != ' ' && map[0][pos.row][pos.column] != 0)
		return FALSE;
	// 반경 [-5, 5]의 사각형 안에 이미 spice가 있으면 생성하지 않음
	if (check_near_spice(pos, 5))
		return FALSE;

	OBJECT* obj = (OBJECT*)malloc(sizeof(OBJECT));
	if (obj != NULL) {
		obj->repr = rand() % 9 + '1'; // '1' ~ '9'
		obj->size = 1;
		obj->pos = pos;

		add_object(0, obj);
		add_system_fmessage("매장량 %c인 스파이스가 생성되었습니다.", obj->repr);
	}
	return TRUE;
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
int isIncluded(SMALL_RECT rect, POSITION pos)
{
	if (pos.row >= rect.Top && pos.row < rect.Bottom && pos.column >= rect.Left && pos.column < rect.Right)
		return TRUE;
	else return FALSE;
}

// 현재 위치와 겹치는 object를 반환
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

// 커서는 화면의 위치를 가지고 있고, object는 map안에서의 상대적인 위치를 가지고 있기 때문에
// 보정이 필요함
int check_object_select(POSITION pos)
{
	// 스크린 좌표와 map 좌표를 보정함
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
		add_system_fmessage("%s이(가) (%d, %d)에서 SandWorm에게 잡혔습니다.",
			get_object_name(objectPool[selected].obj->repr), pos.row, pos.column);
		add_system_fmessage("%s을(를) objectPool에서 삭제합니다.",
			get_object_name(objectPool[selected].obj->repr));
		remove_object(selected);

		worm_longer(worm);
	}
}

int check_unit(int r, int c)
{
	// 일반 유닛은 layer 1에 있음
	// Harvest, Soldier, freMen, Fighter, Tank를 만나면 TRUE 리턴
	if (map[1][r][c] == 'h' || map[1][r][c] == 'H' ||
		map[1][r][c] == 'S' || map[1][r][c] == 'M' ||
		map[1][r][c] == 'F' || map[1][r][c] == 'T')
		return 1;
	else if (map[0][r][c] == 'R' || map[1][r][c] == 'w' || map[1][r][c] == 'W')
		return -1;
	else  // 그렇지 않으면 FALSE 리턴
		return 0;
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
	if (dir == 0) { // object가 발견되지 않으며, 무작위로 4방향으로 이동
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


// SandWorm이 이동 가능하면 TRUE, 이동 불가인 경우에는 FALSE를 반환
int is_moveable(POSITION pos)
{
	int r = pos.row, c = pos.column;
	if (map[0][r][c] != 'R' && map[1][r][c] != 'w' && map[1][r][c] != 'W') // 이동 가능한 경우 웜이 먹을 수 있음
		return TRUE;
	else // Rock이거나 Worm인 경우에는 이동 불가.
		return FALSE;
}

void worm_move(OBJECT* obj)
{
	int nr = 0, nc = 0;

	//가장 가까운 object의 좌표를 찾음
	POSITION newp = decide_next_move(obj->pos);
	
	// 이동이 불가능하면
	if (newp.row == obj->pos.row && newp.column == obj->pos.column) {
		add_system_fmessage("움직일 수 없어 현재 자리에서 대기하겠습니다.");
		return;
	}

	// 새로운 좌표의 이동의 변경이 0 && 0이 아니고, 이동 가능하고, 맵 안에 있으면
	if (is_moveable(newp)) {
		// 이동하기전에 Spice를 생성
		if (rand() % 100 < 10) {	// 10%확률로 Spice를 생성
			if (add_spice(obj->pos))
				worm_shorter(obj);
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
