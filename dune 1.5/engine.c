#include <stdio.h>
#include "dune.h"

char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH] = { 0 };
RESOURCE resource = { 0 };
CURSOR cursor = { 0 };
int clock = 0, prev_clock = -1000;
int selected = -1;

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

    init_map(map);
    init_cursor(&cursor);

    display(resource, map, cursor, -1);

    while (!quit) {
        KEY key = get_key();

        switch (key) {
        case k_quit:
            add_system_message("���α׷��� �����մϴ�...");
            quit = TRUE;
            break;
        case k_up: case k_right: case k_down: case k_left:
            move_cursor(&cursor, key, clock - prev_clock);
            if (key == oldKey)
                prev_clock = clock;
            else prev_clock = -1000;
            oldKey = key;
            break;
        case k_space:
            prev_selected = selected;
            selected = check_object_select(cursor.pos);
            if (selected != -1 && prev_selected != selected)
                add_system_fmessage("%c �� ���õǾ����ϴ�. ", objectPool[selected].obj->repr);
            else if (selected == -1)
                display_desert_information();
            break;
        case k_escape:
            selected = -1;
            clear_messages();
            break;

        case k_M: // �ӽ÷� Ŀ�� ũ�⸦ Ȯ��
            increase_cursor_size(&cursor);
            break;
        case k_S: // �ӽ÷� Ŀ�� ũ�⸦ ���
            decrease_cursor_size(&cursor);
            break;
        default:
            if (key != k_none)
                prev_clock = -1000;
            break;
        }
        Sleep(TICK);
        clock += TICK;

        display(resource, map, cursor, selected);
        flushBuffer();

        object_move();
        put_object(map);
    }

    // 3�� �Ŀ� ȭ���� �����ϰ� ���α׷� ����
    Sleep(3000);
    clearConsole();

    // ����ȭ��
    Outtro();
    cursorOn();
    free_objectPool();

    return 0;
}