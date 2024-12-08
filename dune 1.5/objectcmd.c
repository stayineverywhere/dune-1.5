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
	// ���� �ð��� ������� �ʰų�, ����� �ǹ�/������ �ƴ� ��� return
	if (obj->type != USER) {
		add_system_message("��� ������ �����ϴ�.");
		return;
	}
	else if (obj->consumed_time > 0) {
		add_system_message("���� ���� ����� �ֽ��ϴ�.");
		return;
	}
	else if (obj->consumed_time == 0) {
		// ���� ��� ����
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
			"�Ϻ����� ������ �����߽��ϴ�.");
		break;
	case c_prod_soldier:
		alloc_command(objectPool[selected].obj, cmd, 1 * 10,
			"���� ������ �����߽��ϴ�.");
		break;
	case c_prod_fremen:
		alloc_command(objectPool[selected].obj, cmd, 5 * 10,
			"������ ������ �����߽��ϴ�.");
		break;
	case c_prod_fighter:
		alloc_command(objectPool[selected].obj, cmd, 1 * 10,
			"���� ������ �����߽��ϴ�.");
		break;
	case c_produce_tank:
		alloc_command(objectPool[selected].obj, cmd, 12 * 10,
			"������ ������ �����߽��ϴ�.");
		break;
	case c_harvest: harvest(cmd, selected); break;
	case c_move: unit_move(cmd, selected); break;
	case c_patrol: unit_patrol(cmd, selected); break;
	}
}

void generate_harvester(OBJECT* obj)
{
	POSITION pos;
	// harvester�� �����Ѵ�. ������� �Ϻ����� ���� ��ġ�� 4����..
	int row = obj->pos.row, column = obj->pos.column;
	//����� ���డ������ �ʴ���, ����� ������.
	obj->cmd = c_none;

	if (map[UNIT_LAYER][row - 1][column] == 0) pos = (POSITION){ row - 1, column };
	else if (map[UNIT_LAYER][row - 1][column + 1] == 0) pos = (POSITION){ row - 1, column + 1 };
	else if (map[UNIT_LAYER][row][column + 2] == 0) pos = (POSITION){ row, column + 2 };
	else if (map[UNIT_LAYER][row + 1][column + 2] == 0) pos = (POSITION){ row + 1, column + 2 };
	else {
		add_system_message("�Ϻ����͸� ��ġ�� ������ �����մϴ�.");
		return;
	}
	add_harvester(USER, pos);
	add_system_message("�Ϻ����Ͱ� �غ�Ǿ����ϴ�.");
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
	char* msg = "���� ������ �ǹ� ����Ʈ\n\
- P  : ����, D  : ����,\n\
- G  : â��, B,K: ����,\n\
- S,L: ����ó\n\
- A  : ������\n\
- F,C: ����\n\n\
[ESC]: ��� ���\n[SPACE]: �ǹ� �Ǽ�\n";
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
			add_system_message("������ ��ġ�� ������ �����ϴ�.");
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
			add_system_message("�ǹ��� ��ġ�ϱ� ���ؼ��� ������ ���� ��ƾ� �մϴ�.");

	}
}
