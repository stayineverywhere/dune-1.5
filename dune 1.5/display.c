#include <stdio.h>
#include "dune.h"

/*
 * Double Buffering�� ��������� �����ϴ� Console API�� ����Ͽ� ������.
 * ȭ�鿡 ���̴� ConsoleBuffer�� hStdOut�̸�, ȭ�鿡 ������ �ʴ� ConsoleBuffer�� hHiddenBuffer�� �����Ͽ�
 * Ŀ���� ������ ȭ�� �۾��� ��� hHiddenBuffer���� �����ϰ�, ��� ó���Ŀ��� hHiddenBuffer�� ������ hStdOut���� ������
 * ȭ���� �������� ���̱� ���Ͽ� �� ȭ�� ������ ������ ���Ͽ�, ����� ��쿡�� ������
 * ���������� Console API�� ����Ͽ��� ������, ȭ�鿡 ����ϴ� ������ ���۸� �̿����� �ʰ�, �ڿ������� ȭ�� ����� ���� �� ����
 * ������:
 * 1. https://stackoverflow.com/questions/34842526/update-console-without-flickering-c
 * 2. https://learn.microsoft.com/ko-kr/windows/console/reading-and-writing-blocks-of-characters-and-attributes
 */

HANDLE hStdOut, hHiddenBuffer;
COORD screenSize;
// ȭ�� �񱳸� ���� ���� ���� �迭, ȭ���� ũ�⸦ �˼� ���� ������ ���� �޸� �Ҵ� �̿�
CHAR_INFO* frameData, * frameBack;
SMALL_RECT rectResourceMesg, rectMap, rectSysMesg, rectStatusMsg, rectCommand;

// Ŀ���� ���õ� �Լ��� �����ϰ�, _XXXX()�� ȭ�� ��ũ�� �ڵ��� ���޹ް�, XXXX()�� hHiddenBuffer �ڵ��� �����
// ���� ���α׷����� �ڵ� ���۸� �����ϴ� ��� (intro, outtro ��)������ _XXXX(hStdOut)�� �̿�
// �����̴� Ŀ���� Off/On ��
void cursorOff()
{
    CONSOLE_CURSOR_INFO info;
    info.bVisible = FALSE;
    info.dwSize = 100;
    SetConsoleCursorInfo(hStdOut, &info);
}

// ���� ���ڻ��� ����(����->�����, �����->����)
WORD reverseAttribute(WORD attr)
{
    WORD fg = attr & 0x0F, bg = attr & 0xF0;
    WORD wAttr = setTextAttribute(fg << 4 | bg >> 4);
    return wAttr;
}
void cursorOn()
{
    CONSOLE_CURSOR_INFO info;
    info.bVisible = TRUE;
    info.dwSize = 100;
    SetConsoleCursorInfo(hStdOut, &info);
}

// move cursor to (x, y)
// _gotoXY�� Console�� HANDLE�� �̿�������, �⺻������ hHiddenBuffer���� ��ġ�� �̵�
void _gotoXY(HANDLE handle, int x, int y)
{
    COORD pos = { x, y };
    SetConsoleCursorPosition(handle, pos);

}
void gotoXY(int x, int y)
{
    _gotoXY(hHiddenBuffer, x, y);
}

// _gotoXY�� ����ϰ� Console�� HANDLE�� �̿��Ͽ� ������ ��ġ�� ���ڸ� ���
// ���ڴ� �⺻ char�� �ƴ� wide char ������ ��

CHAR_INFO _getCharXY(HANDLE handle, int x, int y)
{
    CHAR_INFO data[1]; // �����ϰ��� �ϴ� ȭ���� ���뿡 �ش��ϴ� ����
    COORD coordOrg = { 0, 0 };
    COORD coordSize = { 1, 1 };
    SMALL_RECT srcReadRect = { x, y, x, y }; // ���� ������ ����ϱ� ���Ͽ� ���� ��ü ������
    ReadConsoleOutput(handle, data, coordSize, coordOrg, &srcReadRect);
    return data[0];
}

CHAR_INFO getCharXY(int x, int y)
{
    return _getCharXY(hHiddenBuffer, x, y);
}

void _putCharXY(HANDLE handle, int x, int y, wchar_t ch)
{
    _gotoXY(handle, x, y);
    WriteConsole(handle, &ch, 1, NULL, NULL);
}

void putCharXY(int x, int y, wchar_t ch)
{
    _putCharXY(hHiddenBuffer, x, y, ch);
}

// ��� ���ڴ� �⺻ char�� �ƴ� wide char ������ ��� ������, char*�� wchat_t*�� �����Ͽ� �Լ� ȣ��
// �ѱ۵� ����� �� �ֵ��� �ϱ� ���Ͽ� locale.h�� ����Ͽ� �ѱ�� ������
// ����:
// https://m.blog.naver.com/tipsware/223141103183

#include <locale.h>
void _putStringXY(HANDLE handle, int x, int y, char* str)
{
    _gotoXY(handle, x, y);
    wchar_t* wstr = (wchar_t*)malloc((strlen(str) + 1) * sizeof(wchar_t));
    size_t converted;
    setlocale(LC_ALL, "ko-KR");
    mbstowcs_s(&converted, wstr, strlen(str) + 1, str, strlen(str) + 1);
    WriteConsole(handle, wstr, (DWORD)converted, NULL, NULL);
    free(wstr);
}


void putStringXY(int x, int y, char* str)
{
    _putStringXY(hHiddenBuffer, x, y, str);
}

// ���� ȭ���� �׸��� ���Ͽ� �� â�� ��ǥ�� ���
void init_parameters()
{
    int sx = 0, sy = 1;
    int ex = screenSize.X - 1, ey = screenSize.Y - 1;
    int mx = MAP_WIDTH + 1, my = MAP_HEIGHT + sy + 1;

    SMALL_RECT res = { 0, 0, ex, 0 };
    rectResourceMesg = res;
    SMALL_RECT map = { sx + 1, sy + 1, mx - 1, my - 1 };
    rectMap = map;
    SMALL_RECT sys = { sx + 1, my + 1, mx - 1, ey - 1 };
    rectSysMesg = sys;
    SMALL_RECT status = { mx + 1, sy + 1, ex - 1, my - 1 };
    rectStatusMsg = status;
    SMALL_RECT cmd = { mx + 1, my + 1, ex - 1, ey - 1 };
    rectCommand = cmd;
}

// ȭ�� â�� �׸���, ������ â ������ ���

// get current text color
WORD _getTextAttribute()
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;

    GetConsoleScreenBufferInfo(hStdOut, &csbi);
    return csbi.wAttributes;
}

WORD getTextAttribute()
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;

    GetConsoleScreenBufferInfo(hHiddenBuffer, &csbi);
    return csbi.wAttributes;
}

// set current text color
WORD _setTextAttribute(WORD wAttribute)
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;

    GetConsoleScreenBufferInfo(hStdOut, &csbi);
    SetConsoleTextAttribute(hStdOut, wAttribute);

    return csbi.wAttributes;
}
WORD setTextAttribute(WORD wAttribute)
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;

    GetConsoleScreenBufferInfo(hHiddenBuffer, &csbi);
    SetConsoleTextAttribute(hHiddenBuffer, wAttribute);

    return csbi.wAttributes;
}

// �� ������ ���ڸ� ��� (������ attr ���� ����)
void _fillLine(HANDLE handle, int x, int y, int length, wchar_t ch, WORD attr)
{
    COORD coordScreen = { x, y };
    DWORD cCharsWritten;

    FillConsoleOutputCharacter(handle, ch, length, coordScreen, &cCharsWritten);
    FillConsoleOutputAttribute(handle, attr, length, coordScreen, &cCharsWritten);
}

void fillLine(int x, int y, int length, wchar_t ch, WORD attr)
{
    _fillLine(hHiddenBuffer, x, y, length, ch, attr);
}

// hHiddenBuffer�� ������ ����
void clearBuffer()
{
    _fillLine(hHiddenBuffer, 0, 0, screenSize.X * screenSize.Y, ' ', 0);
    _gotoXY(hHiddenBuffer, 0, 0);
}

// ���� ȭ�� (hStdOut)�� ����
void clearConsole()
{
    _fillLine(hStdOut, 0, 0, screenSize.X * screenSize.Y, ' ', 0);
    _gotoXY(hStdOut, 0, 0);
}
// double buffering�� ���Ͽ� Console HANDLE�� ����
void initBuffers()
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;

    // ȭ�� ����
    hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    // double ���۸��� ���� ���� ����
    hHiddenBuffer = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);

    GetConsoleScreenBufferInfo(hStdOut, &csbi);
    screenSize.X = csbi.dwSize.X;
    screenSize.Y = csbi.dwSize.Y;

    init_parameters();

    // �� Console ���� ������ ���縦 ���� CHAR_INFO
    frameData = (CHAR_INFO*)malloc(screenSize.X * screenSize.Y * sizeof(CHAR_INFO));
    if (frameData == NULL) {
        fprintf(stderr, "ȭ���� ���� �޸� �Ҵ� ����: ȭ���� ũ�⸦ �ٿ��ּ���..\n");
        exit(1);
    }
    frameBack = (CHAR_INFO*)malloc(screenSize.X * screenSize.Y * sizeof(CHAR_INFO));
    if (frameBack == NULL) {
        fprintf(stderr, "ȭ���� ���� �޸� �Ҵ� ����: ȭ���� ũ�⸦ �ٿ��ּ���..\n");
        exit(1);
    }


    SetConsoleScreenBufferSize(hStdOut, screenSize);
    SetConsoleScreenBufferSize(hHiddenBuffer, screenSize);

    // �ʱ⿡�� frameData�� ����Ͽ� �����Ϸ��� ������, ���� hHiddenBuffer�� ��°����Ͽ� �̿����� ����
    // frameData�� ���ʿ� ȭ���� �����͸� �о�ͼ� �ʱ�ȭ��
    //COORD coordScreen = { 0, 0 };
    //SMALL_RECT srcWindowRect = { 0, 0, screenSize.X - 1, screenSize.Y - 1 };

    // ReadConsoleOutput(hStdOut, frameData, screenSize, coordScreen, &srcWindowRect);
}

// ���� ȭ��� ������ ȭ���� ������ ���Ͽ�, �� ���ڶ� �ٸ��ٸ� ����Ǿ��ٰ� �Ǵ��Ͽ� ������ ������
// ������ ��ü������ console ������ ���Ͽ� ȭ�� ����� �����Ѵٰ� �����Ǳ� ������ �ʿ� ���� ���� ������
// ���� ���۸��� ��Ȯ�� ������ ���Ͽ� ��� ������ ���ϰ� ȭ�� ����� ������.
int frameDataChanged(CHAR_INFO* frameBack, CHAR_INFO* frameData)
{
    int changed = FALSE;
    int x, y;

    for (y = 0; y < screenSize.Y; y++) {
        for (x = 0; x < screenSize.X; x++) {
            int framePos = y * screenSize.X + x;
            if (frameBack[framePos].Char.UnicodeChar != frameData[framePos].Char.UnicodeChar ||
                frameBack[framePos].Attributes != frameData[framePos].Attributes) {
                return TRUE;
            }
        }
    }

    return FALSE;
}

void display_clock(int clock)
{
    int h, m, s;
    h = clock / (3600000);
    m = (clock - h * 3600000) / 60000;
    s = (clock - h * 3600000 - m * 60000) / 1000;
    char buf[50];
    sprintf_s(buf, sizeof buf, "%02d:%02d:%02d", h, m, s);
    putStringXY(rectMap.Right - 8, 0, buf);
}

void display(RESOURCE resource, char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH], CURSOR cursor, int selectedObj, int clock)
{
    // �� �������� ��踦 ���
    for (int x = rectMap.Left - 1; x <= rectCommand.Right + 1; x++)
        putCharXY(x, rectMap.Top - 1, '*');
    for (int x = rectMap.Left - 1; x <= rectCommand.Right + 1; x++)
        putCharXY(x, rectSysMesg.Top - 1, '*');
    for (int x = rectMap.Left - 1; x <= rectCommand.Right + 1; x++)
        putCharXY(x, rectCommand.Bottom + 1, '*');

    for (int y = rectMap.Top - 1; y <= rectCommand.Bottom; y++)
        putCharXY(rectMap.Left - 1, y, '*');
    for (int y = rectMap.Top - 1; y <= rectCommand.Bottom; y++)
        putCharXY(rectSysMesg.Right + 1, y, '*');
    for (int y = rectMap.Top - 1; y <= rectCommand.Bottom; y++)
        putCharXY(rectCommand.Right + 1, y, '*');

    display_resource(resource);
    display_map(map);
    display_system_message();
    // if selected:
    display_status(selectedObj);
    display_command(selectedObj);
    display_cursor(cursor);

    // display_last_time
    display_clock(clock);
}

// ������ ���� (����)�� ȭ�� ���۷� �����ϰ� �����
void flushBuffer()
{
    COORD coordScreen = { 0, 0 };
    COORD coordBufferSize = { screenSize.X, screenSize.Y };
    SMALL_RECT srcReadRect = { 0, 0, screenSize.X - 1, screenSize.Y - 1 };
    SMALL_RECT srcWriteRect = { 0, 0, screenSize.X - 1, screenSize.Y - 1 };

    FlushFileBuffers(hHiddenBuffer);

    // ���� ��ũ���� Double Buffered�� Console�� ������ �ٲ��� �ʾ�����, ȭ���� ������Ʈ���� ����
// 10ms ������ _kbhit()�� �˻��ϰ�, ȭ���� �ٽ� �׸����� �ϱ� ������ ��ȭ���� ȭ���� ��� �ٽ� �׸��� ����
    ReadConsoleOutput(hStdOut, frameBack, coordBufferSize, coordScreen, &srcReadRect);
    ReadConsoleOutput(hHiddenBuffer, frameData, coordBufferSize, coordScreen, &srcReadRect);
    if (frameDataChanged(frameBack, frameData)) {
        WriteConsoleOutput(hStdOut, frameData, coordBufferSize, coordScreen, &srcWriteRect);
    }
}