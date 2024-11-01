#include "dune.h"
#include <stdlib.h>
#include <math.h>

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



int worm_action(int r, int c)
{
	POSITION pos = { r, c };
	int selected = get_object_id(pos);
	if (selected != -1) {
		// 객체가 선택되었으면..
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
		if (obj->repr == 'W') continue;	// sandworm은 가까운 객체에서 제외
		dist = sqrt((obj->pos.row - pos.row) * (obj->pos.row - pos.row) +
			(obj->pos.column - pos.column) * (obj->pos.column - pos.column));
		if (dist < min_dist) {
			min_dist = dist;
			newp = obj->pos;
		}
		//add_system_fmessage("%c : 거리 %f", obj->repr, dist);
	}
	return newp;
}
void worm_move(OBJECT* obj)
{
	int nr = 0, nc = 0;

	//가장 가까운 object의 좌표를 찾음
	POSITION newp = find_nearest(obj->pos);
	//add_system_fmessage("가장 가까운 물체 좌표: %d, %d", newp.row, newp.column);

	// 한칸씩 가까운데로 움직임
	if (newp.row != obj->pos.row)
		nr = (newp.row - obj->pos.row) > 0 ? 1 : -1;
	if (newp.column != obj->pos.column)
		nc = (newp.column - obj->pos.column) > 0 ? 1 : -1;

	if ((nr || nc) &&	// 새로운 좌표의 이동이 (0,0)이 아닌 경우
		obj->pos.row + nr > 0 && obj->pos.row + nr < MAP_HEIGHT &&
		obj->pos.column + nc > 0 && obj->pos.column + nc < MAP_WIDTH) {
		nr += obj->pos.row;
		nc += obj->pos.column;

		// worm이 객체와 만나는 경우, 다음 action을 취함
		// 움직이지 못하거나, 같은 worm을 만나는 경우 등... 
		if (worm_action(nr, nc) == FALSE)
			return;

		// worm의 길이가 1보다 크면, 한칸씩 앞으로 이동함.
		if (obj->nblock > 0) {
			for (int i = obj->nblock - 1; i > 0; i--) {
				obj->block[i] = obj->block[i - 1];
			}
			// 블록의 처음은, 바로 이동하기전의 좌표를 저장
			obj->block[0].row = obj->pos.row;
			obj->block[0].column = obj->pos.column;
		}
		// 이동하는 새로운 좌표로 변경
		obj->pos.row = nr;
		obj->pos.column = nc;
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
