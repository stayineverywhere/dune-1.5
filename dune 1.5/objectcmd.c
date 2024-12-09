#include "dune.h"


COMMAND_TYPE fetch_unit_command(int selected, KEY key)
{
	OBJECT* obj = objectPool[selected].obj;
	UNIT_TYPE unit = obj->unit;
	if (obj->type == AI) {
		add_system_fmessage("%s는 상대편 (컴퓨터) 유닛입니다.", get_object_name(obj->repr));
		return c_none;
	}
	switch (unit) {
	case BASE:
		if (key == k_H) return c_prod_harvestor;
	case BARRACKS:
		if (key == k_S) return c_prod_soldier;
		break;
	case SHELTER:
		if (key == k_F) return c_prod_fremen;
		break;
	case ARENA:
		if (key == k_F) return c_prod_fighter;
		break;
	case FACTORY:
		if (key == k_T) return c_produce_tank;
		break;
	case HARVESTER:
		switch (key) {
		case k_H: return c_harvest;
		case k_M: return c_move;
		}
		break;
	case FREMEN: case SOLDIER: case FIGHTER: case TANK:
		switch (key) {
		case k_M: return c_move;
		case k_P: return c_patrol;
		}
		break;
	}
	return c_none;
}

void alloc_command(OBJECT* obj, COMMAND_TYPE cmd, int delay_time, char* msg)
{
	// 현재 시간이 경과하지 않거나, 사용자 건물/유닛이 아닌 경우 return
	if (obj->type != USER) {
		add_system_message("명령 권한이 없습니다.");
		return;
	}
	else if (obj->consumed_time > 0) {
		add_system_message("수행 중인 명령이 있습니다.");
		return;
	}
	else if (obj->consumed_time == 0) {
		// 최초 명령 실행
		obj->cmd = cmd;
		obj->consumed_time = delay_time;
		add_system_message(msg);
	}
}

int get_enemy_dir(OBJECT* obj, int* dir)
{
	POSITION pos = obj->pos;
	// 시야를 검사하기 위하여, -vision <= row, column <= vision 의 범위를 검사함.
	SMALL_RECT vis_rect = { pos.column - obj->vision, pos.row - obj->vision,
							pos.column + obj->vision + 1, pos.row + obj->vision + 1 };
	int dist = 10000, e = -1;
	OBJECT* other;
	for (int o = 0; o < nobject; o++) {
		other = objectPool[o].obj;
		// 자신과 타입과 다른 유닛을 공격, 중립 유닛은 공격하지 않음
		if (other->type != NEUTRAL && other->type != obj->type && isPointIncluded(vis_rect, other->pos)) {
			int d = abs(other->pos.row - pos.row) + abs(other->pos.column - pos.column);
			if (dist > d) {
				dist = d;
				e = o;
			}
		}
	}
	if (e != -1) { // 가까운 enemy 찾음
		other = objectPool[e].obj;
		if (abs(other->pos.row - pos.row) > abs(other->pos.column - pos.column)) { // 0, 2
			if (other->pos.row > pos.row) *dir = 2;
			else *dir = 0;
		}
		else { // 1, 3
			if (other->pos.column > pos.column) return *dir = 1;
			else *dir = 3;
		}
		return e;
	}
	return -1;
}

void move_to(OBJECT* obj, POSITION dest)
{
	// dest로 한칸씩 이동함
	POSITION pos = obj->pos;
	int row_diff = dest.row - pos.row, column_diff = dest.column - pos.column;
	if (abs(row_diff) >= abs(column_diff) && row_diff != 0)
		pos.row += row_diff / abs(row_diff);
	else if (abs(column_diff) >= abs(row_diff) && column_diff != 0)
		pos.column += column_diff / abs(column_diff);
	// 이동 후 좌표가 rock이면 우회함
	if (map[BASE_LAYER][pos.row][pos.column] == 'R') {
		if (pos.row != obj->pos.row) { // 세로 이동 (row)
			int column_left = pos.column - 1 >= 0 ? pos.column - 1 : 0;
			int column_right = pos.column + 1 < MAP_WIDTH ? pos.column + 1 : MAP_WIDTH - 1;
			if (map[BASE_LAYER][pos.row][column_left] == ' ' || map[BASE_LAYER][pos.row][column_left] == 0)
			{
				pos.column = column_left;
			}

			if (map[BASE_LAYER][pos.row][column_right] == ' ' || map[BASE_LAYER][pos.row][column_right] == 0)
			{
				pos.column = column_right;
			}
			pos.row = obj->pos.row; // 원상 복귀 (현재 위치에서 x좌표만 변경
		}
		else { // 가로 이동
			int row_up = pos.row - 1 >= 0 ? pos.row - 1 : 0;
			int row_down = pos.row + 1 < MAP_HEIGHT ? pos.row + 1 : MAP_HEIGHT - 1;
			if (map[BASE_LAYER][row_up][pos.column] == ' ' || map[BASE_LAYER][row_up][pos.column] == 0)
			{
				pos.row = row_up;
			}
			else if (map[BASE_LAYER][row_down][pos.column] == ' ' || map[BASE_LAYER][row_down][pos.column] == 0)
			{
				pos.row = row_down;
			}
			pos.column = obj->pos.column; // 원상 복귀
		}
	}


	obj->pos = pos; // 유닛 좌표 업데이트
}

void check_enemy(OBJECT* obj)
{
	if (obj->unit == HARVESTER) return;

	// 인접한 좌표 또는 vision의 범위안에 적군이 있는지 확인
	// 하베스터를 제외하고, 다른 유닛의 경우 vision의 범위안에 적군이 있으면 적군 unit으로 있으면 이동
	int dir, enemy;
	POSITION pos = obj->pos;
	if ((enemy = get_enemy_dir(obj, &dir)) < 0) return;
	if (obj->next_attack_time > 0) return;

	switch (dir) {
	case 0:
		pos.row -= 1;
		if (pos.row < 0) pos.row = 0;
		break;
	case 1:
		pos.column += 1;
		if (pos.column >= MAP_WIDTH) pos.column = MAP_WIDTH - 1;
		break;
	case 2:
		pos.row += 1;
		if (pos.row >= MAP_HEIGHT) pos.row = MAP_HEIGHT - 1;
		break;
	case 3:
		pos.column -= 1;
		if (pos.column < 0) pos.column = 0;
		break;
	}

	// Bonus )
	// 적군의 체력을 거리에 따라 비율로 감소 (원거리 공격)
	// 인접한 경우에는 100%, 나머지는 거리/vision 만큼 반영
	// 적군의 체력이 0이 되면 적군 제거
	// (현재 unit의 체력이 0이 되면, 현재 unit 제거: 보류)
	OBJECT* other = objectPool[enemy].obj;
	int dist_row = abs(other->pos.row - pos.row);
	int dist_col = abs(other->pos.column - pos.column);
	int dist = dist_row + dist_col;
	if (dist <= 1) { // 인접한 경우
		other->strength -= obj->attack_power;
		// obj->strength -= other->attack_power;
	}
	else {
		int range = obj->vision - max(dist_row, dist_col);
		other->strength -= (obj->attack_power * range) / obj->vision;
	}
	if (other->strength <= 0) {
		add_system_fmessage("Unit %s의 체력이 고갈되어 파괴되었습니다.", get_object_name(other->repr));
		remove_object(enemy);
	}

	// 다음 공격 시간 설정
	obj->next_attack_time = obj->attack_period;
}


void harvest(OBJECT* obj)
{
	// dest이 spice 매장된 곳
	// spice>0이면 base로 이동, spice=0이면 dest로 이동
	POSITION prev = obj->pos;
	if (obj->spice > 0) {
		// spice가 0보다 크면 dest로 이동
		// rock이나 worm을 비전안에서 만나면 왼쪽이나 오른쪽으로 이동, 왼쪽부터 먼저 검사
		move_to(obj, obj->orig);
		obj->dir = 0;
	}
	else if (obj->spice == 0) {
		move_to(obj, obj->dest);
	}

	// pos이 dest과 겹치면, harvest하여 spice 양 증가
	if (obj->pos.row == obj->dest.row && obj->pos.column == obj->dest.column) {
		// spice에 도착
		for (int o = 0; o < nobject; o++) {
			OBJECT* sp = objectPool[o].obj;
			if (sp->unit == SPICE && sp->pos.row == obj->pos.row && sp->pos.column == obj->pos.column) {
				add_system_message("하베스터가 수확 중입니다.");
				// 수확하는데 3~5초 정도 걸림
				Sleep(rand() % 2000 + 3000);
				// 스파이스는 한번에 2~4만큼을 채취
				int spice = rand() % 3 + 2; // 2~4
				if (spice > sp->spice) spice = sp->spice;
				obj->spice += spice; // spice 양 증가
				sp->spice -= spice; // spice양 감소
				add_system_fmessage("%d개의 스파이스를 수확하였습니다.", spice);
				sp->repr = sp->spice + '0';
				if (sp->spice == 0)
					remove_object(o);
				break;
			}
		}
	}
	else if (obj->pos.row == obj->orig.row && obj->pos.column == obj->orig.column) {
		// base에 도착해서 spice 양 증가
		extern RESOURCE resource;
		if (resource.spice + obj->spice <= resource.spice_max)
			resource.spice += obj->spice;
		else // 최대 스파이스 양을 초과하면, 최대 스파이스만큼 저장하고 나머지는 삭제
			resource.spice = resource.spice_max;
		obj->spice = 0;
		obj->dir = 1;
	}
	// 움직일 수 없으면 이전 좌표로 복원
	if (map[BASE_LAYER][obj->pos.row][obj->pos.column] != ' ' && map[BASE_LAYER][obj->pos.row][obj->pos.column] != 0 &&
		map[UNIT_LAYER][obj->pos.row][obj->pos.column] != ' ' && map[UNIT_LAYER][obj->pos.row][obj->pos.column] != 0)
		obj->pos = prev;
}

void unit_move(OBJECT* obj)
{
	// post이 dest과 겹치면 아무 작업하지 않음
	if (obj->pos.row != obj->dest.row || obj->pos.column != obj->dest.column) {
		move_to(obj, obj->dest);
	}
	else {
		// 임무 완수, 명령 대기 상태 (초기화)
		obj->cmd = c_none;
	}
	// 목적지에 도착하더라고 적을 감시하도록 수정
	check_enemy(obj);
}

void unit_patrol(OBJECT* obj)
{
	// pos이 dest과 겹치면 orig로 이동
	if (obj->pos.row == obj->dest.row && obj->pos.column == obj->dest.column) {
		obj->dir = 0; // 원점으로 이동
	}
	else if (obj->pos.row == obj->orig.row && obj->pos.column == obj->orig.column) {
		obj->dir = 1; // 목적지로 이동
	}
	// orig와 dest를 왕복함.
	if (obj->dir == 0) { // orig로 이동
		move_to(obj, obj->orig);
	}
	else { // dest로 이동
		move_to(obj, obj->dest);
	}
	check_enemy(obj);
}

void invoke_unit_command(COMMAND_TYPE cmd, int selected, POSITION pos)
{
	OBJECT* obj = objectPool[selected].obj;

	pos = cvt_map_position(pos); // cursor position을 map 좌표로 변환
	if (cmd == c_none) return;
	switch (cmd) {
	case c_prod_harvestor:
		alloc_command(obj, cmd, 5 * 10,
			"하베스터 생산을 시작했습니다.");
		break;
	case c_prod_soldier:
		alloc_command(obj, cmd, 1 * 10,
			"보병 생산을 시작했습니다.");
		break;
	case c_prod_fremen:
		alloc_command(obj, cmd, 5 * 10,
			"프레멘 생산을 시작했습니다.");
		break;
	case c_prod_fighter:
		alloc_command(obj, cmd, 1 * 10,
			"투사 생산을 시작했습니다.");
		break;
	case c_produce_tank:
		alloc_command(obj, cmd, 12 * 10,
			"중전차 생산을 시작했습니다.");
		break;
	case c_harvest: //harvest(obj); // object_move()에서 처리
		break;
	case c_move: //unit_move(obj); // object_move()에서 처리
		break;
	case c_patrol:
		//unit_patrol(obj);	// object_move()에서 처리
		obj->orig = pos;
		break;
	}
}

POSITION find_unit_space(POSITION pos)
{
	int range = 5;
	int r, c, fr, fc;

	// 현재 위치에서부터 원형으로 찾아감
	for (int rng = 1; rng <= range; rng++) {
		// 범위 별로 8개의 직사각형을 찾음
		for (r = -1; r <= 1; r++) {
			for (c = -1; c <= 1; c++) {
				fr = pos.row + r * rng;
				fr = max(fr, 0); fr = min(fr, MAP_HEIGHT - 1);
				fc = pos.column + c * rng;
				fc = max(fc, 0); fc = min(fc, MAP_WIDTH - 1);
				// 결정된 좌표가 현재 주어진 좌표와 동일하면 재 검색
				if (fr == pos.row && fc == pos.column) continue;
				// 배치 가능한 공간을 찾음
				if ((map[UNIT_LAYER][fr][fc] == ' ' || map[UNIT_LAYER][fr][fc] == 0) &&
					(map[GROUND_LAYER][fr][fc] == ' ' || map[GROUND_LAYER][fr][fc] == 0))
					return (POSITION) { fr, fc };
			}
		}
	}
	return (POSITION) { -1, -1 };
}

void produce_unit(OBJECT* obj, UNIT_TYPE unit, char* name)
{
	POSITION pos;
	// unit을 생성한다. 사용자의 unit 생성 위치는 현재 생성가능한 부근의 공간에서 찾는다.
	//명령이 실행가능하지 않더라도, 명령을 종료함.
	obj->cmd = c_none;

	pos = find_unit_space(obj->pos);
	if (pos.row == -1 || pos.column == -1) {
		add_system_fmessage("%s를 배치할 공간이 부족합니다.", name);
		return;
	}
	switch (unit) {
	case BASE:
		add_harvester(USER, pos);
		break;
	case BARRACKS:
		add_soldier(pos);
		break;
	case SHELTER:
		add_fremen(pos);
		break;
	case ARENA:
		add_fighter(pos);
		break;
	case FACTORY:
		add_tank(pos);
		break;

	}
	add_system_fmessage("%s가 준비되었습니다. (%d, %d)", name, pos.row, pos.column);

}

void proc_object_command(OBJECT* obj)
{
	char* name[] = { "보병", "프레멘", "투사", "탱크" };
	if (obj->unit == BASE)
		produce_unit(obj, obj->unit, "하베스터");
	else if (obj->unit >= BARRACKS && obj->unit <= FACTORY)
		produce_unit(obj, obj->unit, name[obj->unit - BARRACKS]);
}

void execute_unit_command()
{
	for (int o = 0; o < nobject; o++) {
		OBJECT* obj = objectPool[o].obj;
		if (obj->cmd != c_none) {
			if (--obj->consumed_time == 0) {
				proc_object_command(obj);
				obj->cmd = c_none;
			}
		}
	}
}

void execute_build_command()
{
	for (int o = 0; o < nobject; o++) {
		OBJECT* obj = objectPool[o].obj;
		// DORMITORY, GARAGE, BARRACKS, SHELTER, ARENA, FACTORY
		if (obj->unit >= DORMITORY && obj->unit <= FACTORY && obj->consumed_time > 0) {
			if (--obj->consumed_time == 0) {
				// proc_build_command(obj);
				// nothing
				;
			}
		}
	}
}

void show_building_command()
{
	char* msg = "건축 가능한 건물 리스트\n\
- P  : 장판, D  : 숙소,\n\
- G  : 창고, B,K: 병영,\n\
- S,L: 은신처\n\
- A  : 투기장\n\
- F,C: 공장\n\n\
[ESC]: 명령 취소\n[SPACE]: 건물 건설\n";
	add_overlap_messages(rectCommand, msg);
}

COMMAND_TYPE fetch_build_command(KEY key)
{
	extern CURSOR cursor;
	switch (key) {
	case k_P:
		increase_cursor_size(&cursor);
		return c_build_plate;
	case k_D:
		return c_build_dorm;
	case k_G:
		return c_build_garage;
	case k_B: case k_K:
		return c_build_barracks;
	case k_S: case k_L:
		return c_build_shelter;
	case k_A:
		return c_build_arena;
	case k_F: case k_C:
		return c_build_factory;
	}
	return c_none;
}

void invoke_build_command(COMMAND_TYPE cmd, POSITION pos)
{
	// update position
	pos = cvt_map_position(pos);

	if (cmd == c_build_plate) {
		if (check_empty(pos, 2))
			build_plate(pos);
		else
			add_system_message("장판을 배치할 공간이 없습니다.");
	}
	else {
		if (check_plate(pos))
			switch (cmd) {
			case c_build_dorm:
				build_dormitory(pos);
				break;
			case c_build_garage:
				build_garage(pos);
				return;
			case c_build_barracks:
				build_barracks(pos);
				break;
			case c_build_shelter:
				build_shelter(pos);
				break;
			case c_build_arena:
				build_arena(pos);
				break;
			case c_build_factory:
				build_factory(pos);
				break;
			}
		else
			add_system_message("건물을 배치하기 위해서는 장판을 먼저 깔아야 합니다.");

	}
}


int is_pos_command(COMMAND_TYPE command)
{
	switch (command) {
	case c_harvest: case c_move: case c_patrol:
		return TRUE;
	}
	return FALSE;
}

void cancel_build_command(int selected)
{
	if (selected < 0) return;
	OBJECT* obj = objectPool[selected].obj;
	if (obj->unit >= DORMITORY && obj->unit <= FACTORY && obj->consumed_time > 0) {
		remove_object(selected);
	}
}