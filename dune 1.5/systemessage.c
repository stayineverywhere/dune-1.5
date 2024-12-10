#include <stdio.h>
#include <stdlib.h>
#include "dune.h"

// ���� ũ��� ȭ�鿡 ǥ�õǴ� ������ ũ�� �����ϰ�, ȭ�鿡 ���̴� ��ŭ�� ����Ѵ�.
// �޽����� ������ ���� ũ�⸦ �Ѿ�� ��쿡�� ������ �޽��� (������ �պκ� �޽���)�� �����Ѵ�.
// ���� ������ ������ ������.

#define MAX_BUFFER_SIZE 100
#define MAX_MESG_LINE 100
char mesgBuffer[MAX_BUFFER_SIZE][MAX_MESG_LINE] = { 0 };
int wndSize = 0; // ȭ�鿡 ���̴� �޽��� â�� ũ��
int bufferIndex = -1;

// printf�� ���� ���� �μ��� �̿��Ͽ� �޽����� ����մϴ�.
void add_system_fmessage(char* mesg, ...)
{
	char buf[MAX_MESG_LINE];
	va_list args;
	va_start(args, mesg);
	vsprintf_s(buf, sizeof(buf), mesg, args);
	va_end(args);
	add_system_message(buf);
}

// ������ �޽����� ���
void add_system_message(char* mesg)
{
	if (wndSize == 0)
		wndSize = rectSysMesg.Bottom - rectSysMesg.Top + 1;

	if (wndSize > MAX_BUFFER_SIZE) {
		printf("Message buffer is too large.\n");
		wndSize = MAX_BUFFER_SIZE;
	}
	if (++bufferIndex >= MAX_BUFFER_SIZE) {
		for (int i = 1; i < MAX_BUFFER_SIZE; i++) {
			strcpy_s(mesgBuffer[i - 1], sizeof(mesgBuffer[i]), mesgBuffer[i]);
		}
		bufferIndex--;
	}

	char* ptr = strchr(mesg, '\n');
	if (ptr != NULL) {
		int len = (int)(ptr - mesg);
		strncpy_s(mesgBuffer[bufferIndex], sizeof(mesgBuffer[0]), mesg, len);
		mesgBuffer[bufferIndex][len] = '\0';

		add_system_message(ptr + 1);
	}
	else
		strcpy_s(mesgBuffer[bufferIndex], sizeof(mesgBuffer[0]), mesg);
}

// bufferIndex�� windowũ�⸦ ����Ͽ� ȭ�鿡 ���̴� �κи� ����մϴ�.
void display_system_message()
{
	if (wndSize == 0 || bufferIndex < 0)
		return;

	int from = bufferIndex < wndSize ? 0 : bufferIndex - wndSize + 1;
	int to = bufferIndex;
	for (int i = from; i <= to; i++) {
		putStringXY(rectSysMesg.Left, rectSysMesg.Top + i - from, mesgBuffer[i]);
		for (int x = rectSysMesg.Left + (int)strlen(mesgBuffer[i]); x < rectSysMesg.Right; x++) {
			putCharXY(x, rectSysMesg.Top + i - from, ' ');
		}
	}
}