#include <stdio.h>
#include "dune.h"

/*
 * Double Buffering은 윈도우즈에서 지원하는 Console API를 사용하여 구현함.
 * 화면에 보이는 ConsoleBuffer는 hStdOut이며, 화면에 보이지 않는 ConsoleBuffer를 hHiddenBuffer로 생성하여
 * 커서를 제외한 화면 작업은 모두 hHiddenBuffer에서 진행하고, 명령 처리후에는 hHiddenBuffer의 내용을 hStdOut으로 복사함
 * 화면의 깜박임을 줄이기 위하여 두 화면 버퍼의 내용을 비교하여, 변경된 경우에만 복사함
 * 윈도우즈의 Console API를 사용하였기 때문에, 화면에 출력하는 별도의 버퍼를 이용하지 않고, 자연스러운 화면 출력을 얻을 수 있음
 * 참고문헌:
 * 1. https://stackoverflow.com/questions/34842526/update-console-without-flickering-c
 * 2. https://learn.microsoft.com/ko-kr/windows/console/reading-and-writing-blocks-of-characters-and-attributes
 */

HANDLE hStdOut, hHiddenBuffer;
COORD screenSize;
// 화면 비교를 위한 문자 정보 배열, 화면의 크기를 알수 없기 때문에 동적 메모리 할당 이용
CHAR_INFO* frameData, * frameBack;
SMALL_RECT rectResourceMesg, rectMap, rectSysMesg, rectStatusMsg, rectCommand;

// 커서와 관련된 함수를 제외하고, _XXXX()는 화면 스크린 핸들을 전달받고, XXXX()는 hHiddenBuffer 핸들을 사용함
// 따라서 프로그램에서 핸들 버퍼를 지정하는 경우 (intro, outtro 등)에서는 _XXXX(hStdOut)을 이용
// 깜박이는 커서를 Off/On 함
void cursorOff()
{
    CONSOLE_CURSOR_INFO info;
    info.bVisible = FALSE;
    info.dwSize = 100;
    SetConsoleCursorInfo(hStdOut, &info);
}

// 기존 글자색의 반전(배경색->전경색, 전경색->배경색)
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
// _gotoXY는 Console의 HANDLE을 이용하지만, 기본적으로 hHiddenBuffer에서 위치를 이동
void _gotoXY(HANDLE handle, int x, int y)
{
    COORD pos = { x, y };
    SetConsoleCursorPosition(handle, pos);

}
void gotoXY(int x, int y)
{
    _gotoXY(hHiddenBuffer, x, y);
}

// _gotoXY와 비슷하게 Console의 HANDLE을 이용하에 정해지 위치에 문자를 출력
// 문자는 기본 char가 아닌 wide char 형식을 띔

CHAR_INFO _getCharXY(HANDLE handle, int x, int y)
{
    CHAR_INFO data[1]; // 복사하고자 하는 화면의 내용에 해당하는 버퍼
    COORD coordOrg = { 0, 0 };
    COORD coordSize = { 1, 1 };
    SMALL_RECT srcReadRect = { x, y, x, y }; // 선택 오류를 축소하기 위하여 한줄 전체 가져옴
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

// 출력 문자는 기본 char가 아닌 wide char 형식을 띄기 때문에, char*를 wchat_t*로 변경하여 함수 호출
// 한글도 출력할 수 있도록 하기 위하여 locale.h를 사용하여 한국어를 지정함
// 참고:
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

// 게임 화면을 그리기 위하여 각 창의 좌표를 계산
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

// 화면 창을 그리고, 각각의 창 내용을 출력

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

// 줄 단위로 문자를 출력 (동일한 attr 지정 가능)
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

// hHiddenBuffer의 내용을 지움
void clearBuffer()
{
    _fillLine(hHiddenBuffer, 0, 0, screenSize.X * screenSize.Y, ' ', 0);
    _gotoXY(hHiddenBuffer, 0, 0);
}

// 컨솔 화면 (hStdOut)을 지움
void clearConsole()
{
    _fillLine(hStdOut, 0, 0, screenSize.X * screenSize.Y, ' ', 0);
    _gotoXY(hStdOut, 0, 0);
}
// double buffering을 위하여 Console HANDLE을 생성
void initBuffers()
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;

    // 화면 컨솔
    hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    // double 버퍼링을 위한 컨솔 버퍼
    hHiddenBuffer = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);

    GetConsoleScreenBufferInfo(hStdOut, &csbi);
    screenSize.X = csbi.dwSize.X;
    screenSize.Y = csbi.dwSize.Y;

    init_parameters();

    // 두 Console 간의 데이터 복사를 위한 CHAR_INFO
    frameData = (CHAR_INFO*)malloc(screenSize.X * screenSize.Y * sizeof(CHAR_INFO));
    if (frameData == NULL) {
        fprintf(stderr, "화면을 위한 메모리 할당 실패: 화면의 크기를 줄여주세요..\n");
        exit(1);
    }
    frameBack = (CHAR_INFO*)malloc(screenSize.X * screenSize.Y * sizeof(CHAR_INFO));
    if (frameBack == NULL) {
        fprintf(stderr, "화면을 위한 메모리 할당 실패: 화면의 크기를 줄여주세요..\n");
        exit(1);
    }


    SetConsoleScreenBufferSize(hStdOut, screenSize);
    SetConsoleScreenBufferSize(hHiddenBuffer, screenSize);

    // 초기에는 frameData에 출력하여 복사하려고 했으나, 직접 hHiddenBuffer에 출력가능하여 이용하지 않음
    // frameData는 최초에 화면의 데이터를 읽어와서 초기화함
    //COORD coordScreen = { 0, 0 };
    //SMALL_RECT srcWindowRect = { 0, 0, screenSize.X - 1, screenSize.Y - 1 };

    // ReadConsoleOutput(hStdOut, frameData, screenSize, coordScreen, &srcWindowRect);
}

// 현재 화면과 숨겨진 화면의 내용을 비교하여, 한 글자라도 다르다면 변경되었다고 판단하여 내용을 복사함
// 윈도우 자체에서도 console 내용을 비교하여 화면 출력을 조절한다고 생각되기 때문에 필요 없을 수도 있지만
// 더블 버퍼링의 정확한 개념을 위하여 모든 내용을 비교하고 화면 출력을 결정함.
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
    // 각 윈도우의 경계를 출력
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

// 숨겨진 내용 (버퍼)을 화면 버퍼로 전송하고 출력함
void flushBuffer()
{
    COORD coordScreen = { 0, 0 };
    COORD coordBufferSize = { screenSize.X, screenSize.Y };
    SMALL_RECT srcReadRect = { 0, 0, screenSize.X - 1, screenSize.Y - 1 };
    SMALL_RECT srcWriteRect = { 0, 0, screenSize.X - 1, screenSize.Y - 1 };

    FlushFileBuffers(hHiddenBuffer);

    // 현재 스크린과 Double Buffered된 Console의 내용이 바뀌지 않았으면, 화면을 업데이트하지 않음
// 10ms 단위로 _kbhit()를 검사하고, 화면을 다시 그리도록 하기 때문에 변화없는 화면의 경우 다시 그리지 않음
    ReadConsoleOutput(hStdOut, frameBack, coordBufferSize, coordScreen, &srcReadRect);
    ReadConsoleOutput(hHiddenBuffer, frameData, coordBufferSize, coordScreen, &srcReadRect);
    if (frameDataChanged(frameBack, frameData)) {
        WriteConsoleOutput(hStdOut, frameData, coordBufferSize, coordScreen, &srcWriteRect);
    }
}