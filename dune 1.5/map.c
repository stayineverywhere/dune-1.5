#include "dune.h"
#include "object.h"

int nobject = 0;
OBJECT_POOL objectPool[MAX_OBJECT];

// object pool에 object 들을 추가함
// layer별로 선언된 object를 추가함
void add_object(int layer, OBJECT* obj)
{
	// object전체를 저장
	if (nobject >= MAX_OBJECT) {
		add_system_message("Object pool is full.");
		add_system_message("MAX_OBJECT를 증가해야 합니다.");
		return;
	}
	objectPool[nobject].layer = layer;
	objectPool[nobject++].obj = obj;

}

// 추가된 object 들을 map 위에 놓음.
void put_object(char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH])
{
	memset(map, 0, sizeof(char) * N_LAYER * MAP_HEIGHT * MAP_WIDTH);

	for (int n = 0; n < nobject; n++) {
		OBJECT* obj = objectPool[n].obj;
		int l = objectPool[n].layer;
		map[l][obj->pos.row][obj->pos.column] = obj->repr;
		if (obj->size == 2) {
			int r = obj->pos.row, c = obj->pos.column;
			map[l][r + 1][c] = obj->repr;
			map[l][r][c + 1] = obj->repr;
			map[l][r + 1][c + 1] = obj->repr;
		}
	}

}

// 전역변수로 설정된 초기 object 정보를 map에 추가합니다.
// 동적으로 추가되는 object는 objects에서 추가합니다.
void init_map(char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH])
{
	add_object(0, &user_base);
	add_object(0, &ai_base);
	for (int o = 0; o < sizeof(plates) / sizeof(plates[0]); o++)
		add_object(0, &plates[o]);
	for (int o = 0; o < sizeof(spices) / sizeof(spices[0]); o++)
		add_object(0, &spices[0]);
	for (int o = 0; o < sizeof(rocks) / sizeof(rocks[0]); o++)
		add_object(0, &rocks[o]);

	add_object(1, &user_harvester);
	add_object(1, &ai_harvester);
	for (int o = 0; o < sizeof(sandWorms) / sizeof(sandWorms[0]); o++)
		add_object(1, &sandWorms[o]);

	put_object(map);
}

// 각각의 object에 맞는 색을 지정
WORD setObjectColor(char repr)
{
	WORD color;
	switch (repr) {
	case '0': case'1': case'2': case'3': case'4': case'5': case'6': case'7': case'8': case'9':
		color = FG_WHITE | BG_MAGENTA;
		break;
	case 'b': case 'h':
		color = FG_WHITE | BG_BLUE;
		break;
	case 'B': case 'H':
		color = FG_WHITE | BG_RED;
		break;
	case 'R':
		color = FG_BLACK | BG_DARKGRAY;
		break;
	case 'W':
		color = FG_WHITE | BG_BROWN;
		break;
	case 'P':
		color = FG_WHITE | BG_BLACK;
		break;
	default:
		return 0;
	}
	return setTextAttribute(color);
}
void display_map(char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH])
{
	WORD wAttr;
	// 맵을 그리기 전에, 맵의 바탕을 흰색으로 그리기: 나중에 필요없으면 삭제
	for (int y = rectMap.Top; y <= rectMap.Bottom; ++y) {
		fillLine(rectMap.Left, y, MAP_WIDTH, ' ', BG_WHITE);
	}

	// 낮은 층 (layer)에서부터 object를 그림.
	for (int l = 0; l < N_LAYER; ++l) {
		for (int y = 0; y < MAP_HEIGHT; ++y) {
			for (int x = 0; x < MAP_WIDTH; ++x) {
				if (map[l][y][x] != 0) {
					wAttr = setObjectColor(map[l][y][x]);
					putCharXY(rectMap.Left + x, rectMap.Top + y, map[l][y][x]);
					if (wAttr)
						setTextAttribute(wAttr);
				}
			}
		}
	}
}