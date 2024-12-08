#include "dune.h"


COMMAND_TYPE fetch_unit_command(int selected, KEY key)
{
	UNIT_TYPE unit = objectPool[selected].obj->unit;
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

void harvest(COMMAND_TYPE cmd, int selected)
{

}

void unit_move(COMMAND_TYPE cmd, int selected)
{

}

void unit_patrol(COMMAND_TYPE cmd, int selected)
{

}

void invoke_unit_command(COMMAND_TYPE cmd, int selected)
{
	if (cmd == c_none) return;

	switch (cmd) {
	case c_prod_harvestor:
		alloc_command(objectPool[selected].obj, cmd, 5 * 10,
			"하베스터 생산을 시작했습니다.");
		break;
	case c_prod_soldier:
		alloc_command(objectPool[selected].obj, cmd, 1 * 10,
			"보병 생산을 시작했습니다.");
		break;
	case c_prod_fremen:
		alloc_command(objectPool[selected].obj, cmd, 5 * 10,
			"프레멘 생산을 시작했습니다.");
		break;
	case c_prod_fighter:
		alloc_command(objectPool[selected].obj, cmd, 1 * 10,
			"투사 생산을 시작했습니다.");
		break;
	case c_produce_tank:
		alloc_command(objectPool[selected].obj, cmd, 12 * 10,
			"중전차 생산을 시작했습니다.");
		break;
	case c_harvest: harvest(cmd, selected); break;
	case c_move: unit_move(cmd, selected); break;
	case c_patrol: unit_patrol(cmd, selected); break;
	}
}

void generate_harvester(OBJECT* obj)
{
	POSITION pos;
	// harvester를 생성한다. 사용자의 하베스터 생성 위치는 4군데..
	int row = obj->pos.row, column = obj->pos.column;
	//명령이 실행가능하지 않더라도, 명령을 종료함.
	obj->cmd = c_none;

	if (map[UNIT_LAYER][row - 1][column] == 0) pos = (POSITION){ row - 1, column };
	else if (map[UNIT_LAYER][row - 1][column + 1] == 0) pos = (POSITION){ row - 1, column + 1 };
	else if (map[UNIT_LAYER][row][column + 2] == 0) pos = (POSITION){ row, column + 2 };
	else if (map[UNIT_LAYER][row + 1][column + 2] == 0) pos = (POSITION){ row + 1, column + 2 };
	else {
		add_system_message("하베스터를 배치할 공간이 부족합니다.");
		return;
	}
	add_harvester(USER, pos);
	add_system_message("하베스터가 준비되었습니다.");
}

void proc_object_command(OBJECT* obj)
{
	switch (obj->unit) {
	case BASE:
		generate_harvester(obj);
	}
}

void execute_unit_command()
{
	for (int o = 0; o < nobject; o++) {
		OBJECT* obj = objectPool[o].obj;
		if (obj->cmd != c_none) {
			if (--obj->consumed_time == 0) {
				proc_object_command(obj);
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
	pos.row -= rectMap.Top;
	pos.column -= rectMap.Left;

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
