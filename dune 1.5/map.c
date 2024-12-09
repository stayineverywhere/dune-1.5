#include "dune.h"
#include "object.h"

// �߰��� object ���� map ���� ����.
POSITION storm_occur_position = { 9, 40 };
void put_object(char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH])
{
	memset(map, 0, sizeof(char) * N_LAYER * MAP_HEIGHT * MAP_WIDTH);

	for (int n = 0; n < nobject; n++) {
		OBJECT* obj = objectPool[n].obj;
		int l = objectPool[n].layer;
		char repr = obj->repr;
		if (obj->unit >= DORMITORY && obj->unit <= FACTORY && obj->consumed_time > 0)
			repr *= -1;
		map[l][obj->pos.row][obj->pos.column] = repr;
		if (obj->size == 2) {
			int r = obj->pos.row, c = obj->pos.column;
			map[l][r + 1][c] = repr;
			map[l][r][c + 1] = repr;
			map[l][r + 1][c + 1] = repr;
		}
		// worm�� ������ ������Ŵ
		else if (obj->unit == SANDWORM && obj->nblock > 0) {
			for (int i = 0; i < obj->nblock; i++) {
				int r = obj->block[i].row, c = obj->block[i].column;
				map[l][r][c] = obj->repr - 'A' + 'a'; // �빮�ڸ� �ҹ��ڷ� ����
			}
		}
	}

}

// ���������� ������ �ʱ� object ������ map�� �߰��մϴ�.
// copy_object�� ���Ͽ� �������� object�� �Ҵ�ް� ���� ������ ������ �� objectPool�� �߰�
// ��Ʈ���̴��� ������ ���ְ� �߸� ������ ������
void init_map(char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH])
{
	build_base(USER, (POSITION) { MAP_HEIGHT - 2, 0 });

	build_plate((POSITION) { MAP_HEIGHT - 2, 2 });

	add_spice((POSITION) { MAP_HEIGHT - 6, 0 }, 5);
	add_spice((POSITION) { 4, MAP_WIDTH - 1 }, 5);

	add_rock((POSITION) { 3, 6 }, 2);
	add_rock((POSITION) { MAP_HEIGHT - 4, 7 }, 2);
	add_rock((POSITION) { MAP_HEIGHT - 5, 4 }, 1);
	add_rock((POSITION) { 4, MAP_WIDTH - 5 }, 1);
	add_rock((POSITION) { MAP_HEIGHT - 2, MAP_WIDTH - 4 }, 1);

	add_harvester(USER, (POSITION) { MAP_HEIGHT - 3, 0 });

	add_worm((POSITION) { 1, 3 });
	add_worm((POSITION) { MAP_HEIGHT - 4, MAP_WIDTH - 6 });

	add_eagle((POSITION) { 9, 20 });
	add_storm(storm_occur_position);

	add_soldier((POSITION) { 8, 6 });
	add_soldier((POSITION) { 8, 20 });

	add_fremen((POSITION) { 10, 8 });
	add_fremen((POSITION) { 12, 15 });
}

// ���ڳ� ������ unit ����
void hakonen_map(char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH])
{
	OBJECT* obj;
	build_base(AI, (POSITION) { 0, MAP_WIDTH - 2 });
	build_plate((POSITION) { 0, MAP_WIDTH - 4 });
	add_harvester(AI, (POSITION) { 2, MAP_WIDTH - 1 });

	// ���ڳ� ������ ��ɾ� ��� �߰�
	// ������� ��ũ�� ��Ʈ�� ���� ��ȯ
	obj = add_fighter((POSITION) { 4, 50 });
	obj->cmd = c_patrol;
	obj->orig = obj->pos;
	obj->dest = (POSITION){ 12, 60 };

	obj = add_fighter((POSITION) { 5, 60 });
	obj->cmd = c_patrol;
	obj->orig = obj->pos;
	obj->dest = (POSITION){ 13, 60 };

	obj = add_tank((POSITION) { 3, 55 });
	obj->cmd = c_patrol;
	obj->orig = obj->pos;
	obj->dest = (POSITION){ 5, 60 };

	obj = add_tank((POSITION) { 5, 45 });
	obj->cmd = c_patrol;
	obj->orig = obj->pos;
	obj->dest = (POSITION){ 10, 60 };
}

char UnitSymbol[] = { ' ', 'B', 'P', '\0', 'R', 'D', 'G', 'K', 'L', 'A', 'C',
	'H', 'M', 'S', 'F', 'T', 'W', 'E', 'O' };

POSITION cvt_map_position(POSITION pos)
{
	POSITION ret;
	ret.row = pos.row - rectMap.Top;
	ret.column = pos.column - rectMap.Left;
	return ret;
}
void display_map(char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH])
{
	static int toggle = 0;
	WORD wAttr;
	// ���� �׸��� ����, ���� ������ ������� �׸���: ���߿� �ʿ������ ����
	for (int y = rectMap.Top; y <= rectMap.Bottom; ++y) {
		fillLine(rectMap.Left, y, MAP_WIDTH, ' ', BG_WHITE);
	}

	// ���� �� (layer)�������� object�� �׸�.
	for (int l = 0; l < N_LAYER; ++l) {
		for (int y = 0; y < MAP_HEIGHT; ++y) {
			for (int x = 0; x < MAP_WIDTH; ++x) {
				if (map[l][y][x] != 0) {
					//UNIT_TYPE unit = (UNIT_TYPE)map[l][y][x];
					char sym = map[l][y][x];
					if (sym < 0) {
						sym = -sym;
						wAttr = setObjectColor(sym);
						toggle++;
						if (toggle % 4 == 0)
							wAttr = reverseAttribute(wAttr);
					}
					else
						wAttr = setObjectColor(sym);

					putCharXY(rectMap.Left + x, rectMap.Top + y, sym);
					if (wAttr)
						setTextAttribute(wAttr);
				}
			}
		}
	}
}

// plate(����)�� �����ϱ� ���Ͽ� layer0�� ������� �˻�
int check_empty(POSITION pos, int size)
{
	for (int r = 0; r < size; r++) {
		for (int c = 0; c < size; c++) {
			if (map[BASE_LAYER][r + pos.row][c + pos.column] != ' ' &&
				map[BASE_LAYER][r + pos.row][c + pos.column] != 0)
				return FALSE;
		}
	}
	return TRUE;
}

int check_plate(POSITION pos)
{
	// layer0�� ������ ��� �ִ��� �˻�
	if (map[BASE_LAYER][pos.row][pos.column] == 'P' || map[BASE_LAYER][pos.row][pos.column] == 'p')
		return TRUE;
	else
		return FALSE;
}