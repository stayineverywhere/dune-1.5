#include <stdio.h>
#include <stdlib.h>
#include "dune.h"

#define MAX_OBJECT 20
#define MAX_MESG_LINE 1024

enum object_id { obj_desert, obj_harvest, obj_base, obj_plate, obj_rock, obj_spice };
char objInformation[MAX_OBJECT][MAX_MESG_LINE] = {
	"�縷������ ��µ˴ϴ�.\n����� ����� ����ϴ��� �𸣰ھ��.",
	"�Ϻ����ʹ�\nspice�� ä���ϰų� �̵��մϴ�.\n������: 5, �α���: 5\n�̵��ֱ�: 2��\n���ݷ�, �����ֱ�: ����\nü��: 70, �þ�:0", // harvest
};
char objCommand[MAX_OBJECT][MAX_MESG_LINE] = {
	"",
	"H: Harvest, M: Move", // harvest ��ɾ�
};

// ��ü ����(����)�κ��� ������ ��ü ������ ����ϵ��� ���� �ε���(enum)�� ��ȯ�մϴ�.
// spice�� ���, spice ���� ������ ������ ���� ����ϵ��� ���� �����մϴ�.
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

// object â �Ǵ� ���� â�� object ���õ� ������ ����Ѵ�.
// �ý��� â�� ������ �߰��Ǹ�, ��ũ���� ������, object â�� �׻� ���� ���� ������
// object â�� ��ġ�� �������� object ���� ������ ����Ѵ�.

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
		// clear screen line, �޽����� ������ ��ü ������ ����
		if (len > 0) {
			strncpy_s(cmdLine, sizeof(cmdLine), msg, len);
			cmdLine[len] = '\0';
			msg += len + 1;
			putStringXY(rect.Left, y, cmdLine);
			// ���� ��� ������ ���� ������ ' '�� ä��
			fillLine(rect.Left + len, y, rect.Right - rect.Left - len + 1, ' ', 0);
		}
		else // ���� ��ü�� ' '�� ä��
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

// ������ �ȵ� ��� �縷 ������ ����մϴ�.
void display_desert_information()
{
	char* status = objInformation[obj_desert];

	add_overlap_messages(rectStatusMsg, status);
	add_overlap_messages(rectCommand, "");
}