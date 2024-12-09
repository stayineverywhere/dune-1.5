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

void harvest(OBJECT* obj)
{
	// dest�� spice ����� ��
	// spice>0�̸� base�� �̵�, spice=0�̸� dest�� �̵�
	POSITION prev = obj->pos;
	if (obj->spice > 0) {
		// spice�� 0���� ũ�� dest�� �̵�
		// rock�̳� worm�� �����ȿ��� ������ �����̳� ���������� �̵�, ���ʺ��� ���� �˻�
		move_to(obj, obj->orig);
		obj->dir = 0;
	}
	else if (obj->spice == 0) {
		move_to(obj, obj->dest);
	}

	// pos�� dest�� ��ġ��, harvest�Ͽ� spice �� ����
	if (obj->pos.row == obj->dest.row && obj->pos.column == obj->dest.column) {
		// spice�� ����
		for (int o = 0; o < nobject; o++) {
			OBJECT* sp = objectPool[o].obj;
			if (sp->unit == SPICE && sp->pos.row == obj->pos.row && sp->pos.column == obj->pos.column) {
				add_system_message("�Ϻ����Ͱ� ��Ȯ ���Դϴ�.");
				// ��Ȯ�ϴµ� 3~5�� ���� �ɸ�
				Sleep(rand() % 2000 + 3000);
				// �����̽��� �ѹ��� 2~4��ŭ�� ä��
				int spice = rand() % 3 + 2; // 2~4
				if (spice > sp->spice) spice = sp->spice;
				obj->spice += spice; // spice �� ����
				sp->spice -= spice; // spice�� ����
				add_system_fmessage("%d���� �����̽��� ��Ȯ�Ͽ����ϴ�.", spice);
				sp->repr = sp->spice + '0';
				if (sp->spice == 0)
					remove_object(o);
				break;
			}
		}
	}
	else if (obj->pos.row == obj->orig.row && obj->pos.column == obj->orig.column) {
		// base�� �����ؼ� spice �� ����
		extern RESOURCE resource;
		if (resource.spice + obj->spice <= resource.spice_max)
			resource.spice += obj->spice;
		else // �ִ� �����̽� ���� �ʰ��ϸ�, �ִ� �����̽���ŭ �����ϰ� �������� ����
			resource.spice = resource.spice_max;
		obj->spice = 0;
		obj->dir = 1;
	}
	// ������ �� ������ ���� ��ǥ�� ����
	if (map[BASE_LAYER][obj->pos.row][obj->pos.column] != ' ' && map[BASE_LAYER][obj->pos.row][obj->pos.column] != 0 &&
		map[UNIT_LAYER][obj->pos.row][obj->pos.column] != ' ' && map[UNIT_LAYER][obj->pos.row][obj->pos.column] != 0)
		obj->pos = prev;
}

void unit_move(OBJECT* obj)
{
	// post�� dest�� ��ġ�� �ƹ� �۾����� ����
	if (obj->pos.row != obj->dest.row || obj->pos.column != obj->dest.column) {
		move_to(obj, obj->dest);
	}
	else {
		// �ӹ� �ϼ�, ��� ��� ���� (�ʱ�ȭ)
		obj->cmd = c_none;
	}
	// �������� �����ϴ���� ���� �����ϵ��� ����
	check_enemy(obj);
}

void unit_patrol(OBJECT* obj)
{
	// pos�� dest�� ��ġ�� orig�� �̵�
	if (obj->pos.row == obj->dest.row && obj->pos.column == obj->dest.column) {
		obj->dir = 0; // �������� �̵�
	}
	else if (obj->pos.row == obj->orig.row && obj->pos.column == obj->orig.column) {
		obj->dir = 1; // �������� �̵�
	}
	// orig�� dest�� �պ���.
	if (obj->dir == 0) { // orig�� �̵�
		move_to(obj, obj->orig);
	}
	else { // dest�� �̵�
		move_to(obj, obj->dest);
	}
	check_enemy(obj);
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
