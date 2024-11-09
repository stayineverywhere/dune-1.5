#include "dune.h"
#include "object.h"

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
		//worm의 꼬리를 증가시킴
		else if (obj->repr == 'W' && obj->nblock > 0) {
			for (int i = 0; i < obj->nblock; i++) {
				int r = obj->block[i].row, c = obj->block[i].column;
				map[l][r][c] = 'w';
			}
		}
	}

}

// 전역변수로 설정된 초기 object 정보를 map에 추가합니다.
// 동적으로 추가되는 object는 objects에서 추가합니다.
void init_map(char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH])
{
	add_object(0, copy_object(& user_base));
	add_object(0, copy_object(&ai_base));
	for (int o = 0; o < sizeof(plates) / sizeof(plates[0]); o++)
		add_object(0, copy_object(&plates[o]));
	for (int o = 0; o < sizeof(spices) / sizeof(spices[0]); o++)
		add_object(0, copy_object(&spices[0]));
	for (int o = 0; o < sizeof(rocks) / sizeof(rocks[0]); o++)
		add_object(0, copy_object(&rocks[o]));

	add_object(1, copy_object(&user_harvester));
	add_object(1, copy_object(&ai_harvester));
	for (int o = 0; o < sizeof(sandWorms) / sizeof(sandWorms[0]); o++)
		add_object(1, copy_object(&units[o]));

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
	case 'w':	// SandWorm 꼬리
		color = FG_YELLOW | BG_BROWN;
		break;
	case 'W':   //SandWorm 머리
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