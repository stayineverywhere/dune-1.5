#include <stdio.h>
#include "dune.h"

char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH] = { 0 };
RESOURCE resource = { 0 };
CURSOR cursor = { 0 };
int clock = 0, prev_clock = -1000;
int selected = RESET_OBJECT;
COMMAND_TYPE command = c_none;

void update_cursor_info(CURSOR* cursor)
{
    put_object(map);
    display_map(map);
    update_cursor(cursor);
}

int main()
{
    KEY oldKey = k_none;
    int quit = FALSE;
    int prev_selected; // 선택 객체가 변경되었음을 알려줌

    initBuffers();

    // initBuffers에서 스크린 버터를 가져오거 생성하기 때문에, cursorOff는 initBuffers 다음에 위치하여야 함
    cursorOff();

    // 시작 화면
    Intro();

    add_system_message("프로그램을 시작합니다.");
    add_system_message("Dune Game programmed by 윤주영(20232532), 2024/12/10");

    resource.population_max = 300; // 초기 최대 인구 수
    resource.spice_max = 100;
    add_system_fmessage("최대 스파이스 저장량은 %d으로 설정되었습니다.", resource.spice_max);
    add_system_fmessage("최대 인구 수는 %d으로 설정되었습니다.", resource.population_max);

    init_map(map);
    hakonen_map(map); // 하코넨 진영 명령어
    put_object(map);

    init_cursor(&cursor);

    display(resource, map, cursor, -1, clock);

    while (!quit) {
        KEY key = get_key();

        switch (key) {
        case k_quit: case k_Q:
            add_system_message("프로그램을 종료합니다...");
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
                    add_system_fmessage("%s이(가) 선택되었습니다. ",
                        get_object_name(objectPool[selected].obj->repr));
                else if (selected == -1)
                    display_desert_information();
            }
            else if (selected >= 0 && is_pos_command(command)) {
                // unit 명령어를 위한 추가 명령 (위치 지정)
                add_system_fmessage("%s 위치 지정 명령",
                    get_object_name(objectPool[selected].obj->repr));
                objectPool[selected].obj->cmd = command;
                objectPool[selected].obj->dest = cvt_map_position(cursor.pos);
                command = c_none;
                selected = RESET_OBJECT;
            }
            else if (selected == RESET_OBJECT) {
                // build command
                decrease_cursor_size(&cursor); // 커서 크기 축소
                add_system_fmessage("건설 명령어: %d", command);
                invoke_build_command(command, cursor.pos);
                command = c_none;
                selected = RESET_OBJECT;
                // cursor가 있는 자리에서 건물 업데이트가 발생하였기 때문에, 강제로 커서 정보 갱신
                update_cursor_info(&cursor);
            }
            break;
        case k_escape: case k_X:
            if (selected >= 0) { // 진행중이던 작업이 존재하는 경우
                objectPool[selected].obj->cmd = c_none;
                objectPool[selected].obj->consumed_time = 0;
                add_system_fmessage("%s이(가) 진행중이던 작업을 취소하였습니다.",
                    get_object_name(objectPool[selected].obj->repr));
            }
            else if (command != c_none) { // 건축 명령
                decrease_cursor_size(&cursor); // 커서 크기 축소
            }
            else {
                int s = check_object_select(cursor.pos);
                cancel_build_command(s);
                // cursor가 있는 자리에서 건물 건설 취소가 발생하였기 때문에, 강제로 커서 정보 갱신
                update_cursor_info(&cursor);
            }
            selected = RESET_OBJECT; // -1은 선택되지 않음, RESET_OBJECT는 초기화
            command = c_none;
            clear_messages();
            break;
        case k_B: // 생성 가능한 건물 리스트 출력 및 건물 건축 명령어 수집
            if (selected == RESET_OBJECT && command == c_none) {
                command = c_build_cmd;
                show_building_command();
                break;
            }
        default:
            if (key != k_none)
                prev_clock = -1000;
            // unit이 선택되어 있다면, 키 값을 입력 받아 명령어 전달
            if (selected >= 0 && key != k_none) {
                command = fetch_unit_command(selected, key);
                invoke_unit_command(command, selected, cursor.pos);
            }
            // unit이 선택되지 않은 상황 (사막도 선택되지 않음, 사막은 -1로 선택)에서 
            // build_cmd 상황이면 build command를 입력 받음
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
        // build command가 즉시 시행되지 않고 지연된 실행을 지원하는 경우를 고려.
        execute_build_command();

        display(resource, map, cursor, selected, clock);
        flushBuffer();
    }

    // 3초 후에 화면을 삭제하고 프로그램 종료
    Sleep(3000);
    clearConsole();

    // 종료화면
    Outtro();
    cursorOn();
    free_objectPool();

    return 0;
}