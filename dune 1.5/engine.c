#include <stdio.h>
#include "dune.h"

char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH] = { 0 };
RESOURCE resource = { 0 };
CURSOR cursor = { 0 };
int clock = 0, prev_clock = -1000;
int selected = RESET_OBJECT;
COMMAND_TYPE command = c_none;


int main()
{
    KEY oldKey = k_none;
    int quit = FALSE;
    int prev_selected; // ���� ��ü�� ����Ǿ����� �˷���

    initBuffers();

    // initBuffers���� ��ũ�� ���͸� �������� �����ϱ� ������, cursorOff�� initBuffers ������ ��ġ�Ͽ��� ��
    cursorOff();

    // ���� ȭ��
    Intro();

    add_system_message("���α׷��� �����մϴ�.");
    add_system_message("Dune Game programmed by ���ֿ�(20232532), 2024/11/12");

    resource.population_max = 300; // �ʱ� �ִ� �α� ��
    resource.spice_max = 100;
    add_system_fmessage("�ִ� �����̽� ���差�� %d���� �����Ǿ����ϴ�.", resource.spice_max);
    add_system_fmessage("�ִ� �α� ���� %d���� �����Ǿ����ϴ�.", resource.population_max);

    init_map(map);
    init_cursor(&cursor);

    display(resource, map, cursor, -1, clock);

    while (!quit) {
        KEY key = get_key();

        switch (key) {
        case k_quit: case k_Q:
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
            if (command == c_none) {
                prev_selected = selected;
                selected = check_object_select(cursor.pos);
                if (selected != -1 && prev_selected != selected)
                    add_system_fmessage("%s��(��) ���õǾ����ϴ�. ",
                        get_object_name(objectPool[selected].obj->repr));
                else if (selected == -1)
                    display_desert_information();
            }
            else {
                // build command
                decrease_cursor_size(&cursor); // Ŀ�� ũ�� ���
                add_system_fmessage("�Ǽ� ��ɾ�: %d", command);
                invoke_build_command(command, cursor.pos);
                command = c_none;
                selected = RESET_OBJECT;
            }
            break;
        case k_escape: case k_X:
            if (selected >= 0) { // �������̴� �۾��� �����ϴ� ���
                objectPool[selected].obj->cmd = c_none;
                objectPool[selected].obj->consumed_time = 0;
                add_system_fmessage("%s��(��) �������̴� �۾��� ����Ͽ����ϴ�.",
                    get_object_name(objectPool[selected].obj->repr));
            }
            else if (command != c_none) { // ���� ���
                decrease_cursor_size(&cursor); // Ŀ�� ũ�� ���
            }
            selected = RESET_OBJECT; // -1�� ���õ��� ����, RESET_OBJECT�� �ʱ�ȭ
            command = c_none;
            clear_messages();
            break;
        case k_B: // ���� ������ �ǹ� ����Ʈ ��� �� �ǹ� ���� ��ɾ� ����
            if (selected == RESET_OBJECT) {
                command = c_build_cmd;
                show_building_command();
            }
            break;
        default:
            if (key != k_none)
                prev_clock = -1000;
            // unit�� ���õǾ� �ִٸ�, Ű ���� �Է� �޾� ��ɾ� ����
            if (selected >= 0 && key != k_none) {
                command = fetch_unit_command(selected, key);
                invoke_unit_command(command, selected);
            }
            // unit�� ���õ��� ���� ��Ȳ (�縷�� ���õ��� ����, �縷�� -1�� ����)���� 
            // build_cmd ��Ȳ�̸� build command�� �Է� ����
            else if (selected == RESET_OBJECT && command == c_build_cmd && key != k_none) {
                command = fetch_build_command(key);
            }
            break;
        }
        Sleep(TICK);
        clock += TICK;

        object_move();
        put_object(map);
        execute_unit_command();
        // build command�� ��� ������� �ʰ� ������ ������ �����ϴ� ��츦 ���.
        // ���� �̱��� : TODO
        // execute_build_command();

        display(resource, map, cursor, selected, clock);
        flushBuffer();
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