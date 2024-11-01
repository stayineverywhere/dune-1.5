#include <stdio.h>
#include <stdlib.h>
#include "dune.h"

#define MAX_OBJECT 20
#define MAX_MESG_LINE 1024

enum object_id { obj_desert, obj_harvest, obj_base, obj_plate, obj_rock, obj_spice };
char objInformation[MAX_OBJECT][MAX_MESG_LINE] = {
	"사막정보가 출력됩니다.\n현재는 어떤것을 출력하는지 모르겠어요.",
	"하베스터는\nspice를 채굴하거나 이동합니다.\n생산비용: 5, 인구수: 5\n이동주기: 2초\n공격력, 공격주기: 없음\n체력: 70, 시야:0", // harvest
};
char objCommand[MAX_OBJECT][MAX_MESG_LINE] = {
	"",
	"H: Harvest, M: Move", // harvest 명령어
};

// 객체 정보(문자)로부터 공동된 객체 정보를 출력하도록 관련 인덱스(enum)를 반환합니다.
// spice의 경우, spice 관련 정보와 현재의 값을 출력하도록 추후 수정합니다.
int convert_repr2object(char repr)
{
	switch (repr) {
	case 'h': case 'H':
		return obj_harvest;
	case 'b': case 'B':
		return obj_base;
	case 'P':
		return obj_plate;
	case 'R':
		return obj_rock;
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
		return obj_spice;
	}
	return obj_desert;
}

// object 창 또는 상태 창에 object 관련된 정보를 출력한다.
// 시스템 창은 내용이 추가되며, 스크롤이 되지만, object 창은 항상 새로 쓰기 때문에
// object 창의 위치를 기준으로 object 관련 정보를 출력한다.

void add_overlap_messages(SMALL_RECT rect, char* msg)
{
	char cmdLine[128];

	for (int y = rect.Top; y <= rect.Bottom; y++) {
		char* eol = strchr(msg, '\n');
		int len = 0;
		if (eol == NULL)
			len = (int)strlen(msg);
		else if (eol - msg > 0)
			len = (int)(eol - msg);
		// clear screen line, 메시지가 없으면 전체 라인을 지움
		if (len > 0) {
			strncpy_s(cmdLine, sizeof(cmdLine), msg, len);
			cmdLine[len] = '\0';
			msg += len + 1;
			putStringXY(rect.Left, y, cmdLine);
			// 현재 출력 라인의 남은 공간을 ' '로 채움
			fillLine(rect.Left + len, y, rect.Right - rect.Left - len + 1, ' ', 0);
		}
		else // 한줄 전체를 ' '로 채움
			fillLine(rect.Left, y, rect.Right - rect.Left + 1, ' ', 0);
	}

}

void display_status(int selObj)
{
	int object = convert_repr2object(objectPool[selObj].obj->repr);
	char* status = objInformation[object];

	add_overlap_messages(rectStatusMsg, status);
}

void display_command(int selObj)
{
	int object = convert_repr2object(objectPool[selObj].obj->repr);
	char* cmd = objCommand[object];

	add_overlap_messages(rectCommand, cmd);
}

void clear_messages()
{
	add_overlap_messages(rectStatusMsg, "");
	add_overlap_messages(rectCommand, "");
}

// 선택이 안된 경우 사막 정보를 출력합니다.
void display_desert_information()
{
	char* status = objInformation[obj_desert];

	add_overlap_messages(rectStatusMsg, status);
	add_overlap_messages(rectCommand, "");
}