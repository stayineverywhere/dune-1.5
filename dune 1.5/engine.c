/* 진행 사항 (완료사항) 
* 키 입력 받기
* q가 입력 받고 3초후에 종료
* 키 타입 정의
* 방향키 입력 받기
* 문자 색상 변경 
* 스켈레톤 코드
* 1) 준비
* 초기 상태
* 2) 커서 & 상태창 
* 3) 중립 유닛 
*    - 샌드웜 , Bonus 1 , Bonus 2
*    - 샌드웜 이동 과정 -> README.txt에 설명해두었습니다. 
*    - Bonus1 과 Bonus 2에 대해서는 README.txt에 설명해두었습니다. 
* 4) 유닛 1기 생산
*    - Bonus -> README.txt에 자세히 설명해두었습니다. 
* 5) 시스템 메시지 
*/
/*
* 교수님께 
* command창의 버퍼가 너무 크면은 속도가 느려지고 intro outro의 화면이 보이지 않는 경우가 있습니다. 
* 설정을 가로 120 , 세로 40 ~ 50 으로 맞추시면 intro outro와 실행창이 보다 뚜렷하게 나타납니다. 
* 감사합니다. 
*/
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
    int prev_selected; // 선택 객체가 변경되었음을 알려줌

    initBuffers();

    // initBuffers에서 스크린 버터를 가져오거 생성하기 때문에, cursorOff는 initBuffers 다음에 위치하여야 함
    cursorOff();

    // 시작 화면
    Intro();

    add_system_message("프로그램을 시작합니다.");
    add_system_message("Dune Game programmed by 윤주영(20232532), 2024/11/12");

    resource.population_max = 300; // 초기 최대 인구 수
    resource.spice_max = 100;
    add_system_fmessage("최대 스파이스 저장량은 %d으로 설정되었습니다.", resource.spice_max);
    add_system_fmessage("최대 인구 수는 %d으로 설정되었습니다.", resource.population_max);

    init_map(map);
    init_cursor(&cursor);

    display(resource, map, cursor, -1, clock);

    while (!quit) {
        KEY key = get_key();

        switch (key) {
        case k_quit:
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
            prev_selected = selected;
            selected = check_object_select(cursor.pos);
            if (selected != -1 && prev_selected != selected)
                add_system_fmessage("%s이(가) 선택되었습니다. ",
                    get_object_name(objectPool[selected].obj->repr));
            else if (selected == -1)
                display_desert_information();
            break;
        case k_escape: case k_X:
            if (selected >= 0) { // 진행중이던 작업이 존재하는 경우
                objectPool[selected].obj->cmd = c_none;
                objectPool[selected].obj->consumed_time = 0;
                add_system_fmessage("%s이(가) 진행중이던 작업을 취소하였습니다.",
                    get_object_name(objectPool[selected].obj->repr));
            }
            selected = RESET_OBJECT; // -1은 선택되지 않음, RESET_OBJECT는 초기화
            clear_messages();
            break;

        case k_M: // 임시로 커서 크기를 확대
            increase_cursor_size(&cursor);
            break;
        case k_S: // 임시로 커서 크기를 축소
            decrease_cursor_size(&cursor);
            break;
        default:
            if (key != k_none)
                prev_clock = -1000;
            if (selected >= 0 && key != k_none) {
                command = fetch_command(selected, key);
                invoke_command(command, selected);
            }
            break;
        }
        Sleep(TICK);
        clock += TICK;

        object_move();
        put_object(map);
        execute_command();

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