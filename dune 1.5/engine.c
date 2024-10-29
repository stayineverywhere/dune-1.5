#include <stdio.h>
#include "dune.h"

char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH] = { 0 };
RESOURCE resource = { 0 };
CURSOR cursor = { 0 };

int main() {
    KEY oldKey = k_none;
    int quit = FALSE;
    int prev_selected; // ���� ��ü�� ����Ǿ����� �˷���

    initBuffers();

    // initBuffers���� ��ũ�� ���͸� �������� �����ϱ� ������, cursorOff�� initBuffers ������ ��ġ�Ͽ��� ��
    cursorOff();

    // ���� ȭ��
    Intro();

    add_system_message("���α׷��� �����մϴ�.");
    add_system_message("Dune Game programmed by Jooyoung Yun, 2024/11/05");

    // 3�� �Ŀ� ȭ���� �����ϰ� ���α׷� ����
    Sleep(3000);
    clearConsole();

    // ����ȭ��
    Outtro();
    cursorOn();

    return 0;
}