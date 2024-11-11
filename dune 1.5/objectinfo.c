#include <stdio.h>
#include <stdlib.h>
#include "dune.h"

#define MAX_MESG_LINE 1024

// objName�� UNIT_TYPE ������ ���� ����Ǿ�� �Ѵ�.
char objName[MAX_OBJECT][20] = {
	"�縷", "����", "�����̽�", "����", "�ϼ�", "����", "â��", "����", "����ó", "������", "����",
	"�Ϻ�����",  "������", "����", "����", "������", "�����", "�縷������", "�縷��ǳ"
};
char objInformation[MAX_OBJECT][MAX_MESG_LINE] = {
	"�縷������ ��µ˴ϴ�.\n���� ��������� �������� �ʾҽ��ϴ�.",
	"�Ϻ����͸� �����ϴ� �ǹ��Դϴ�.", //����
	"������ �����ϱ� ���ؼ���\n�����̽��� ���� �����Ͽ��� �մϴ�.", // �����̽�
	"�ǹ��� ���� ���ؼ���\n���� ������ ��ƾ� �մϴ�.", // ����
	"�ϼ����� �ǹ���\n���� �� �����ϴ�.", // �ϼ�
	"������ �����ϱ� ���ؼ���\n���Ұ� �ʿ��մϴ�.", // ����
	"�����̽��� �ִ��\n�����ϱ� ���ؼ���\nâ�� ����� �մϴ�.", // â��
	"������ ������ �� �ֽ��ϴ�.", // ����
	"�������� ������ �� �ֽ��ϴ�.", // ����ó
	"���縦 ������ �� �ֽ��ϴ�.", // ������
	"�������� ������ �� �ֽ��ϴ�.", // ����
	"�Ϻ����ʹ�\nspice�� ä���ϰų� �̵��մϴ�.\n������: 5, �α���: 5\n�̵��ֱ�: 2��\n���ݷ�, �����ֱ�: ����\nü��: 70, �þ�:0", // harvester
	"", // ������
	"", // ����
	"", // ����
	"", // ������
	"", //�����
	"", // �縷������
	"" // �縷��ǳ
};

char objCommand[MAX_OBJECT][MAX_MESG_LINE] = {
	"",
	"H: �Ϻ����� ����",
	"", // �����̽�
	"", // ����
	"", // �ϼ�
	"", // ����
	"", // â��
	"S: ���� ����", // ����
	"F: ������ ����:", // ����ó
	"F: ���� ����", // ������
	"T: ������ ����", // ����
	"H: Harvest, M: Move", // harvester ��ɾ�
	"M: �̵�, P: ����", // ������
	"M: �̵�, P: ����", // ����
	"M: �̵�, P: ����", // ����
	"M: �̵�, P: ����", // ������
};

char symbolDescription[] = "=�ǹ��� ���� ���� ����=\n\
b,B: �ǹ�, P: ����,\n1~9: �����̽�, R: �ϼ�,\n\
D: ����, G: â��,\n\
K: ����, L: ����ó\n\
A: ������, C: ����,\n\
H: �Ϻ�����, M: ������,\n\
S: ����, F: ����,\n\
T: ������,\n\
w,W: �����,\n\
E: �縷������,\nO: �縷��ǳ";
char defaultCommand[] = "���õ� ������ ����\n���డ���� ����� �����ϴ�.";

// ��ü ����(����)�κ��� ������ ��ü ������ ����ϵ��� ���� �ε���(enum)�� ��ȯ�մϴ�.
// spice�� ���, spice ���� ������ ������ ���� ����ϵ��� ���� �����մϴ�.
UNIT_TYPE convert_repr2object(char repr)
{
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
			msg += len;
			if (eol != NULL) msg++;
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
	if (selObj >= 0) {
		OBJECT* obj = objectPool[selObj].obj;
		int obj_class = convert_repr2object(obj->repr);
		char* status = objInformation[obj_class];
		add_overlap_messages(rectStatusMsg, status);
		if (obj->consumed_time > 0) {
			char buf[128];
			sprintf_s(buf, sizeof buf, "���� �������� ������ �����մϴ�.\n���� �Ϸ� �ð� %d ms",
				obj->consumed_time * TICK);
			add_overlap_messages(rectStatusMsg, buf);
		}
	}
	else if (selObj == RESET_OBJECT)
		add_overlap_messages(rectStatusMsg, symbolDescription);
}

void display_command(int selObj)
{
	if (selObj >= 0) {
		int object = convert_repr2object(objectPool[selObj].obj->repr);
		char* cmd = objCommand[object];
		add_overlap_messages(rectCommand, cmd);
	}
	else if (selObj == RESET_OBJECT)
		add_overlap_messages(rectCommand, defaultCommand);
}

void clear_messages()
{
	add_overlap_messages(rectStatusMsg, "");
	add_overlap_messages(rectCommand, "");
}

// ������ �ȵ� ��� �縷 ������ ����մϴ�.
void display_desert_information()
{
	char* status = objInformation[NONE];

	add_overlap_messages(rectStatusMsg, status);
	add_overlap_messages(rectCommand, "");
}