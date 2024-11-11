#include "dune.h"


COMMAND_TYPE fetch_command(int selected, KEY key)
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

void produce_harvestor(COMMAND_TYPE cmd, int selected)
{
	OBJECT* obj = objectPool[selected].obj;
	// 현재 시간이 경과하지 않거나, 사용자의 base가 아니면 return
	if (obj->consumed_time > 0 || obj->type != USER) {
		add_system_message("수행 중인 명령이 있거나 권한이 없습니다.");
		return;
	}
	else if (obj->consumed_time == 0) {
		// 최초 명령 실행
		obj->cmd = cmd;
		obj->consumed_time = 5 * 10; // 하베스터 생산비용 * 10 * 10 ms걸림
		add_system_message("하베스터 생산을 시작했습니다.");
	}
}

void produce_soldier(COMMAND_TYPE cmd, int selected)
{
}

void produce_fremen(COMMAND_TYPE cmd, int selected)
{

}

void produce_fighter(COMMAND_TYPE cmd, int selected)
{

}

void produce_tank(COMMAND_TYPE cmd, int selected)
{

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

void invoke_command(COMMAND_TYPE cmd, int selected)
{
	if (cmd == c_none) return;

	switch (cmd) {
	case c_prod_harvestor: produce_harvestor(cmd, selected); break;
	case c_prod_soldier: produce_soldier(cmd, selected); break;
	case c_prod_fremen: produce_fremen(cmd, selected); break;
	case c_prod_fighter: produce_fighter(cmd, selected); break;
	case c_produce_tank: produce_tank(cmd, selected); break;
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

	if (map[1][row - 1][column] == 0) pos = (POSITION){ row - 1, column };
	else if (map[1][row - 1][column + 1] == 0) pos = (POSITION){ row - 1, column + 1 };
	else if (map[1][row][column + 2] == 0) pos = (POSITION){ row, column + 2 };
	else if (map[1][row + 1][column + 2] == 0) pos = (POSITION){ row + 1, column + 2 };
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

void execute_command()
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