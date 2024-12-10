#include <stdio.h>
#include <stdlib.h>
#include "dune.h"

// 버퍼 크기는 화면에 표시되는 값보다 크게 설정하고, 화면에 보이는 만큼만 출력한다.
// 메시지의 내용이 버퍼 크기를 넘어가는 경우에는 오래된 메시지 (버퍼의 앞부분 메시지)를 삭제한다.
// 원형 버퍼의 개념을 적용함.

#define MAX_BUFFER_SIZE 100
#define MAX_MESG_LINE 100
char mesgBuffer[MAX_BUFFER_SIZE][MAX_MESG_LINE] = { 0 };
int wndSize = 0; // 화면에 보이는 메시지 창의 크기
int bufferIndex = -1;

// printf와 같이 가변 인수를 이용하여 메시지를 출력합니다.
void add_system_fmessage(char* mesg, ...)
{
	char buf[MAX_MESG_LINE];
	va_list args;
	va_start(args, mesg);
	vsprintf_s(buf, sizeof(buf), mesg, args);
	va_end(args);
	add_system_message(buf);
}

// 정해진 메시지만 출력
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

// bufferIndex와 window크기를 고려하여 화면에 보이는 부분만 출력합니다.
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