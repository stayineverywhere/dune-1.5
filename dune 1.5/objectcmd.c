#include "dune.h"


COMMAND_TYPE fetch_unit_command(int selected, KEY key)
{
	OBJECT* obj = objectPool[selected].obj;
	UNIT_TYPE unit = obj->unit;
	if (obj->type == AI) {
		add_system_fmessage("%s�� ����� (��ǻ��) �����Դϴ�.", get_object_name(obj->repr));
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

int get_enemy_dir(OBJECT* obj, int* dir)
{
	POSITION pos = obj->pos;
	// �þ߸� �˻��ϱ� ���Ͽ�, -vision <= row, column <= vision �� ������ �˻���.
	SMALL_RECT vis_rect = { pos.column - obj->vision, pos.row - obj->vision,
							pos.column + obj->vision + 1, pos.row + obj->vision + 1 };
	int dist = 10000, e = -1;
	OBJECT* other;
	for (int o = 0; o < nobject; o++) {
		other = objectPool[o].obj;
		// �ڽŰ� Ÿ�԰� �ٸ� ������ ����, �߸� ������ �������� ����
		if (other->type != NEUTRAL && other->type != obj->type && isPointIncluded(vis_rect, other->pos)) {
			int d = abs(other->pos.row - pos.row) + abs(other->pos.column - pos.column);
			if (dist > d) {
				dist = d;
				e = o;
			}
		}
	}
	if (e != -1) { // ����� enemy ã��
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
	// dest�� ��ĭ�� �̵���
	POSITION pos = obj->pos;
	int row_diff = dest.row - pos.row, column_diff = dest.column - pos.column;
	if (abs(row_diff) >= abs(column_diff) && row_diff != 0)
		pos.row += row_diff / abs(row_diff);
	else if (abs(column_diff) >= abs(row_diff) && column_diff != 0)
		pos.column += column_diff / abs(column_diff);
	// �̵� �� ��ǥ�� rock�̸� ��ȸ��
	if (map[BASE_LAYER][pos.row][pos.column] == 'R') {
		if (pos.row != obj->pos.row) { // ���� �̵� (row)
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
			pos.row = obj->pos.row; // ���� ���� (���� ��ġ���� x��ǥ�� ����
		}
		else { // ���� �̵�
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
			pos.column = obj->pos.column; // ���� ����
		}
	}


	obj->pos = pos; // ���� ��ǥ ������Ʈ
}

void check_enemy(OBJECT* obj)
{
	if (obj->unit == HARVESTER) return;

	// ������ ��ǥ �Ǵ� vision�� �����ȿ� ������ �ִ��� Ȯ��
	// �Ϻ����͸� �����ϰ�, �ٸ� ������ ��� vision�� �����ȿ� ������ ������ ���� unit���� ������ �̵�
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
	// ������ ü���� �Ÿ��� ���� ������ ���� (���Ÿ� ����)
	// ������ ��쿡�� 100%, �������� �Ÿ�/vision ��ŭ �ݿ�
	// ������ ü���� 0�� �Ǹ� ���� ����
	// (���� unit�� ü���� 0�� �Ǹ�, ���� unit ����: ����)
	OBJECT* other = objectPool[enemy].obj;
	int dist_row = abs(other->pos.row - pos.row);
	int dist_col = abs(other->pos.column - pos.column);
	int dist = dist_row + dist_col;
	if (dist <= 1) { // ������ ���
		other->strength -= obj->attack_power;
		// obj->strength -= other->attack_power;
	}
	else {
		int range = obj->vision - max(dist_row, dist_col);
		other->strength -= (obj->attack_power * range) / obj->vision;
	}
	if (other->strength <= 0) {
		add_system_fmessage("Unit %s�� ü���� ���Ǿ� �ı��Ǿ����ϴ�.", get_object_name(other->repr));
		remove_object(enemy);
	}

	// ���� ���� �ð� ����
	obj->next_attack_time = obj->attack_period;
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

void invoke_unit_command(COMMAND_TYPE cmd, int selected, POSITION pos)
{
	OBJECT* obj = objectPool[selected].obj;

	pos = cvt_map_position(pos); // cursor position�� map ��ǥ�� ��ȯ
	if (cmd == c_none) return;
	switch (cmd) {
	case c_prod_harvestor:
		alloc_command(obj, cmd, 5 * 10,
			"�Ϻ����� ������ �����߽��ϴ�.");
		break;
	case c_prod_soldier:
		alloc_command(obj, cmd, 1 * 10,
			"���� ������ �����߽��ϴ�.");
		break;
	case c_prod_fremen:
		alloc_command(obj, cmd, 5 * 10,
			"������ ������ �����߽��ϴ�.");
		break;
	case c_prod_fighter:
		alloc_command(obj, cmd, 1 * 10,
			"���� ������ �����߽��ϴ�.");
		break;
	case c_produce_tank:
		alloc_command(obj, cmd, 12 * 10,
			"������ ������ �����߽��ϴ�.");
		break;
	case c_harvest: //harvest(obj); // object_move()���� ó��
		break;
	case c_move: //unit_move(obj); // object_move()���� ó��
		break;
	case c_patrol:
		//unit_patrol(obj);	// object_move()���� ó��
		obj->orig = pos;
		break;
	}
}

POSITION find_unit_space(POSITION pos)
{
	int range = 5;
	int r, c, fr, fc;

	// ���� ��ġ�������� �������� ã�ư�
	for (int rng = 1; rng <= range; rng++) {
		// ���� ���� 8���� ���簢���� ã��
		for (r = -1; r <= 1; r++) {
			for (c = -1; c <= 1; c++) {
				fr = pos.row + r * rng;
				fr = max(fr, 0); fr = min(fr, MAP_HEIGHT - 1);
				fc = pos.column + c * rng;
				fc = max(fc, 0); fc = min(fc, MAP_WIDTH - 1);
				// ������ ��ǥ�� ���� �־��� ��ǥ�� �����ϸ� �� �˻�
				if (fr == pos.row && fc == pos.column) continue;
				// ��ġ ������ ������ ã��
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
	// unit�� �����Ѵ�. ������� unit ���� ��ġ�� ���� ���������� �α��� �������� ã�´�.
	//����� ���డ������ �ʴ���, ����� ������.
	obj->cmd = c_none;

	pos = find_unit_space(obj->pos);
	if (pos.row == -1 || pos.column == -1) {
		add_system_fmessage("%s�� ��ġ�� ������ �����մϴ�.", name);
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
	add_system_fmessage("%s�� �غ�Ǿ����ϴ�. (%d, %d)", name, pos.row, pos.column);

}

void proc_object_command(OBJECT* obj)
{
	char* name[] = { "����", "������", "����", "��ũ" };
	if (obj->unit == BASE)
		produce_unit(obj, obj->unit, "�Ϻ�����");
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
	pos = cvt_map_position(pos);

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