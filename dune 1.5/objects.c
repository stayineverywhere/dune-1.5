#include "dune.h"
#include <stdlib.h>
#include <math.h>

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



int worm_action(int r, int c)
{
	POSITION pos = { r, c };
	int selected = get_object_id(pos);
	if (selected != -1) {
		// ��ü�� ���õǾ�����..
		int primary = selected / 100;
		int secondary = selected % 100;
	}
	return TRUE;
}
POSITION find_nearest(POSITION pos)
{
	POSITION newp = { 0,0 };
	double min_dist = 10000.0, dist;

	for (int i = 0; i < nobject; i++) {
		OBJECT* obj = objectPool[i].obj;
		if (obj->repr == 'W') continue;	// sandworm�� ����� ��ü���� ����
		dist = sqrt((obj->pos.row - pos.row) * (obj->pos.row - pos.row) +
			(obj->pos.column - pos.column) * (obj->pos.column - pos.column));
		if (dist < min_dist) {
			min_dist = dist;
			newp = obj->pos;
		}
		//add_system_fmessage("%c : �Ÿ� %f", obj->repr, dist);
	}
	return newp;
}
void worm_move(OBJECT* obj)
{
	int nr = 0, nc = 0;

	//���� ����� object�� ��ǥ�� ã��
	POSITION newp = find_nearest(obj->pos);
	//add_system_fmessage("���� ����� ��ü ��ǥ: %d, %d", newp.row, newp.column);

	// ��ĭ�� ������ ������
	if (newp.row != obj->pos.row)
		nr = (newp.row - obj->pos.row) > 0 ? 1 : -1;
	if (newp.column != obj->pos.column)
		nc = (newp.column - obj->pos.column) > 0 ? 1 : -1;

	if ((nr || nc) &&	// ���ο� ��ǥ�� �̵��� (0,0)�� �ƴ� ���
		obj->pos.row + nr > 0 && obj->pos.row + nr < MAP_HEIGHT &&
		obj->pos.column + nc > 0 && obj->pos.column + nc < MAP_WIDTH) {
		nr += obj->pos.row;
		nc += obj->pos.column;

		// worm�� ��ü�� ������ ���, ���� action�� ����
		// �������� ���ϰų�, ���� worm�� ������ ��� ��... 
		if (worm_action(nr, nc) == FALSE)
			return;

		// worm�� ���̰� 1���� ũ��, ��ĭ�� ������ �̵���.
		if (obj->nblock > 0) {
			for (int i = obj->nblock - 1; i > 0; i--) {
				obj->block[i] = obj->block[i - 1];
			}
			// ����� ó����, �ٷ� �̵��ϱ����� ��ǥ�� ����
			obj->block[0].row = obj->pos.row;
			obj->block[0].column = obj->pos.column;
		}
		// �̵��ϴ� ���ο� ��ǥ�� ����
		obj->pos.row = nr;
		obj->pos.column = nc;
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
