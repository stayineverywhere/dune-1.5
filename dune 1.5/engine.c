#include <stdio.h>
#include "dune.h"

char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH] = { 0 };
RESOURCE resource = { 0 };
CURSOR cursor = { 0 };

int main() {
    KEY oldKey = k_none;
    int quit = FALSE;
    int prev_selected; // 선택 객체가 변경되었음을 알려줌

    initBuffers();

    // initBuffers에서 스크린 버터를 가져오거 생성하기 때문에, cursorOff는 initBuffers 다음에 위치하여야 함
    cursorOff();

    // 시작 화면
    Intro();

    add_system_message("프로그램을 시작합니다.");
    add_system_message("Dune Game programmed by Jooyoung Yun, 2024/11/05");

    // 3초 후에 화면을 삭제하고 프로그램 종료
    Sleep(3000);
    clearConsole();

    // 종료화면
    Outtro();
    cursorOn();

    return 0;
}