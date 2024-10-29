#include <stdio.h>
#include "dune.h"

HANDLE hStdOut, hHiddenBuffer;
COORD screenSize;
// 화면 비교를 위한 문자 정보 배열, 화면의 크기를 알수 없기 때문에 동적 메모리 할당 이용
CHAR_INFO* frameData, * frameBack;
SMALL_RECT rectResourceMesg, rectMap, rectSysMesg, rectStatusMsg, rectCommand;

void display(RESOURCE resource, char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH], CURSOR cursor, int selectedObj)
{
    // 각 윈도우의 경계를 출력
    for (int x = rectMap.Left - 1; x <= rectCommand.Right + 1; x++)
        putCharXY(x, rectMap.Top - 1, '*');
    for (int x = rectMap.Left - 1; x <= rectCommand.Right + 1; x++)
        putCharXY(x, rectSysMesg.Top - 1, '*');
    for (int x = rectMap.Left - 1; x <= rectCommand.Right + 1; x++)
        putCharXY(x, rectCommand.Bottom + 1, '*');

    for (int y = rectMap.Top - 1; y <= rectCommand.Bottom; y++)
        putCharXY(rectMap.Left - 1, y, '*');
    for (int y = rectMap.Top - 1; y <= rectCommand.Bottom; y++)
        putCharXY(rectSysMesg.Right + 1, y, '*');
    for (int y = rectMap.Top - 1; y <= rectCommand.Bottom; y++)
        putCharXY(rectCommand.Right + 1, y, '*');

    display_resource(resource);
    display_map(map);
    display_system_message();
    // if selected:
    if (selectedObj != -1) {
        display_status(selectedObj);
        display_command(selectedObj);
    }
    display_cursor(cursor);
}
