#pragma once

//// base와 harvester는 대소문자로 사용자와 ai (상대편)을 구분함
//// 문자로 구분하지 않을 경우에는 별도의 flag를 사용 가능
//OBJECT user_base = { {MAP_HEIGHT - 2, 0}, {0,0}, 2, 'b', -1, -1 };
//OBJECT ai_base = { {0, MAP_WIDTH - 2}, {0,0}, 2, 'B', -1, -1 };
//OBJECT user_harvester = { {MAP_HEIGHT - 3, 0}, {0, 0}, 1, 'h', 200, 200 };
//OBJECT ai_harvester = { {2, MAP_WIDTH - 1}, {0, 0}, 1, 'H', 200, 200 };
//
//OBJECT plates[] = {
//	{{MAP_HEIGHT - 2, 2}, {0, 0}, 2, 'P', -1, -1},
//	{{0, MAP_WIDTH - 4}, {0, 0}, 2, 'P', -1, -1}
//};
//OBJECT spices[] = {
//	{{ MAP_HEIGHT - 6, 0}, {0, 0}, 1, '5', -1, -1},
//	{{4, MAP_WIDTH - 1}, {0, 0}, 1, '5', -1, -1}
//};
//#define WORM_PERIOD 25
//OBJECT sandWorms[] = {
//	{{1, 3}, {0, 0}, 1, 'W', WORM_PERIOD, WORM_PERIOD},
//	{{MAP_HEIGHT - 4, MAP_WIDTH - 6}, {0, 0}, 1, 'W', WORM_PERIOD, WORM_PERIOD}
//};
//
//OBJECT sandEagles = { {9, 20}, {0, 0}, 1, 'E', 10, 10 };
//OBJECT sandStorm = { {9, 40}, {0, 0}, 2, 'A', 10, 10 };
//
//OBJECT rocks[] = {
//	{{3, 6}, {0,0}, 2, 'R', -1, -1},
//	{{MAP_HEIGHT - 4, 7}, {0, 0}, 2, 'R', -1, -1},
//	{{MAP_HEIGHT - 5, 4}, {0, 0}, 1, 'R', -1, -1},
//	{{4, MAP_WIDTH - 5}, {0, 0}, 1, 'R', -1, -1},
//	{{MAP_HEIGHT - 2, MAP_WIDTH - 4}, {0, 0}, 1, 'R', -1, -1},
//};
//
//// Worm의 꼬리 증가 테스트를 위하여 SandWorm이 잡아 먹는 유닛 배치
//OBJECT units[] = {
//	{{8, 6}, {0,0}, 1, 'S', -1, -1},
//	{{8, 20}, {0,0}, 1, 'S', -1, -1},
//	{{10, 8}, {0,0}, 1, 'M', -1, -1},
//	{{12, 15}, {0,0}, 1, 'M', -1, -1},
//	{{4, 50}, {0,0}, 1, 'F', -1, -1},
//	{{4, 50}, {0,0}, 1, 'F', -1, -1},
//	{{7, 55}, {0,0}, 1, 'T', -1, -1},
//	{{9, 45}, {0,0}, 1, 'T', -1, -1}
//};

