#include <stdio.h>
#include <stdlib.h>
#include "dune.h"

#define MAX_MESG_LINE 1024

// objName은 UNIT_TYPE 순서에 따라 기술되어야 한다.
char objName[MAX_OBJECT][20] = {
	"사막", "기지", "스파이스", "장판", "암석", "숙소", "창고", "병영", "은신처", "투기장", "공장",
	"하베스터",  "프레멘", "보병", "투사", "중전차", "샌드웜", "사막독수리", "사막폭풍"
};
char objInformation[MAX_OBJECT][MAX_MESG_LINE] = {
	"사막정보가 출력됩니다.\n현재 출력정보가 결정되지 않았습니다.",
	"하베스터를 생산하는 건물입니다.", //기지
	"유닛을 생산하기 위해서는\n스파이스를 많이 수집하여야 합니다.", // 스파이스
	"건물을 짓기 위해서는\n먼저 장판을 깔아야 합니다.", // 장판
	"암석에는 건물을\n지을 수 없습니다.", // 암석
	"유닛을 생산하기 위해서는\n숙소가 필요합니다.", // 숙소
	"스파이스를 최대로\n보관하기 위해서는\n창고를 지어야 합니다.", // 창고
	"보병을 생산할 수 있습니다.", // 병영
	"프레멘을 생산할 수 있습니다.", // 은신처
	"투사를 생산할 수 있습니다.", // 투기장
	"중전차를 생산할 수 있습니다.", // 공장
	"하베스터는\nspice를 채굴하거나 이동합니다.\n생산비용: 5, 인구수: 5\n이동주기: 2초\n공격력, 공격주기: 없음\n체력: 70, 시야:0", // harvester
	"프레멘은 빠르고 은밀합니다.\n생산비용: 5, 인구수: 2\n이동주기: 0.4초\n공격력: 15, 공격주기: 0.2초\n체력: 25, 시야:8", // 프레멘
	"보병은 저렴하고 쉽게 생산할 수 있습니다.\n주로 초기 공격이나 방어에 활용됩니다.\n생산비용: 1, 인구수: 1\n이동주기: 1초\n공격력: 5,공격주기: 0.8초\n체력: 15, 시야:1", // 보병
	"투사는 일반 보병보다 강력하며,\n중전차와 같은 강력한 유닛에\n대응합니다.\n생산비용: 1, 인구수: 1\n이동주기: 1.2초\n공격력: 6,공격주기: 0.6초\n체력: 10, 시야:1", // 투사
	"중전차는 전면 공격을 하는 유닛으로\n높은 방어력과 강력한 화력을 자랑합니다.\n생산비용: 12, 인구수: 5\n이동주기: 3초\n공격력: 40,공격주기: 4초\n체력: 60, 시야:4", // 중전차
	"사막지역에서 유닛과 차량을 삼켜버립니다.", //샌드웜
	"목적없이 하늘을 날아다니는 공중 유닛입니다.", // 사막독수리
	"자연재해로 건물이나 유닛을 파괴합니다." // 사막폭풍
};

char objCommand[MAX_OBJECT][MAX_MESG_LINE] = {
	"",
	"H: 하베스터 생산",
	"", // 스파이스
	"", // 장판
	"", // 암석
	"", // 숙소
	"", // 창고
	"S: 보병 생산", // 병영
	"F: 프레멘 생산:", // 은신처
	"F: 투사 생산", // 투기장
	"T: 중전차 생산", // 공장
	"H: Harvest, M: Move", // harvester 명령어
	"M: 이동, P: 순찰", // 프레멘
	"M: 이동, P: 순찰", // 보병
	"M: 이동, P: 순찰", // 투사
	"M: 이동, P: 순찰", // 중전차
};

char symbolDescription[] = "=건물과 유닛 구별 문자=\n\
b,B: 건물, P: 장판,\n1~9: 스파이스, R: 암석,\n\
D: 숙소, G: 창고,\n\
K: 병영, L: 은신처\n\
A: 투기장, C: 공장,\n\
H: 하베스터, M: 프레멘,\n\
S: 보병, F: 투사,\n\
T: 중전차,\n\
w,W: 샌드웜,\n\
E: 사막독수리,\nO: 사막폭풍";
char defaultCommand[] = "B : 건물 건축\n건축 가능한 건물리스트\n\
- 장판 (건물 기초 생성),\n\
- 숙소 (인구 수 증가),\n\
- 창고 (스파이스 저장량 증가),\n\
- 병영 (보병 생산),\n\
- 은신처 (프레멘 생산),\n\
- 투기장 (투사 생산),\n\
- 공장 (중전차 생산)";

// 객체 정보(문자)로부터 공동된 객체 정보를 출력하도록 관련 인덱스(enum)를 반환합니다.
// spice의 경우, spice 관련 정보와 현재의 값을 출력하도록 추후 수정합니다.
UNIT_TYPE convert_repr2object(char repr)
{
	if (repr < 0)
		repr = -repr;
	switch (repr) {
	case 'h': case 'H':
		return HARVESTER;
	case 'b': case 'B':
		return BASE;
	case 'P':
		return PLATE;
	case 'R':
		return ROCK;
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
		return SPICE;
	case 'S':
		return SOLDIER;
	case 'M':
		return FREMEN;
	case 'F':
		return FIGHTER;
	case 'G':
		return GARAGE;
	case 'T':
		return TANK;
	case 'D':
		return DORMITORY;
	case 'K':
		return BARRACKS;
	case 'L':
		return SHELTER;
	case 'A':
		return ARENA;
	case 'E':
		return SANDEAGLE;
	case 'O':
		return SANDSTORM;
	case 'w': case 'W':
		return SANDWORM;
	}
	return NONE;
}

char* get_object_name(char repr)
{
	return objName[(int)convert_repr2object(repr)];
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
			msg += len;
			putStringXY(rect.Left, y, cmdLine);
			// 현재 출력 라인의 남은 공간을 ' '로 채움
			fillLine(rect.Left + len, y, rect.Right - rect.Left - len + 1, ' ', 0);
		}
		else // 한줄 전체를 ' '로 채움
			fillLine(rect.Left, y, rect.Right - rect.Left + 1, ' ', 0);
		if (eol != NULL) msg++;
	}

}

void display_status(int selObj)
{
	if (selObj >= 0) {
		OBJECT* obj = objectPool[selObj].obj;
		int obj_class = convert_repr2object(obj->repr);
		char* desc = objInformation[obj_class];
		char status_buf[2048];
		char tmp[128];
		//add_overlap_messages(rectStatusMsg, status);
		strcpy_s(status_buf, sizeof status_buf, desc);
		if (obj->unit == HARVESTER) {
			sprintf_s(tmp, sizeof tmp, "\n보유 스파이스: %d", obj->spice);
			strcat_s(status_buf, sizeof status_buf, tmp);
		}
		sprintf_s(tmp, sizeof tmp, "\n현재 체력: %d\n", obj->strength);
		strcat_s(status_buf, sizeof status_buf, tmp);

		if (obj->consumed_time > 0) {
			sprintf_s(tmp, sizeof tmp, "****\n현재 생산중인 유닛이 존재합니다.\n남은 완료 시간 %d ms",
				obj->consumed_time * TICK);
			//add_overlap_messages(rectStatusMsg, buf);
			strcat_s(status_buf, sizeof status_buf, tmp);
		}
		add_overlap_messages(rectStatusMsg, status_buf);
	}
	else if (selObj == RESET_OBJECT)
		add_overlap_messages(rectStatusMsg, symbolDescription);
}

void display_command(int selObj)
{
	extern COMMAND_TYPE command;
	if (selObj >= 0) {
		int object = convert_repr2object(objectPool[selObj].obj->repr);
		char* cmd = objCommand[object];
		if (objectPool[selObj].obj->type != AI)
			add_overlap_messages(rectCommand, cmd);
		else
			add_overlap_messages(rectCommand, "");
	}
	else if (selObj == RESET_OBJECT && command == c_none)
		add_overlap_messages(rectCommand, defaultCommand);
}

void clear_messages()
{
	add_overlap_messages(rectStatusMsg, "");
	add_overlap_messages(rectCommand, "");
}

// 선택이 안된 경우 사막 정보를 출력합니다.
void display_desert_information()
{
	char* status = objInformation[NONE];

	add_overlap_messages(rectStatusMsg, status);
	add_overlap_messages(rectCommand, "");
}