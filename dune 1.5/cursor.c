#include "dune.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// 초기 커서 좌표를 지정
// 커서 좌표는 화면 좌표를 이용하고, 이동 범위를 rectMap 안으로 한정
void init_cursor(CURSOR* cursor) {
	srand((unsigned)time(NULL));
	cursor->pos.row = rand() % MAP_HEIGHT + rectMap.Top;
	cursor->pos.column = rand() % MAP_WIDTH + rectMap.Left;
	cursor->charInfo[0] = getCharXY(cursor->pos.column, cursor->pos.row);
	cursor->size = 1;

	add_system_fmessage("초기 커서 좌표: %d행 %d열",
		cursor->pos.row - rectMap.Top,
		cursor->pos.column - rectMap.Left);
}

// 화면에 커서를 출력한다.
// 커서가 확대되는 경우에는 동일한 방식으로 확대하여 사용
// console의 커서를 사용하는 방법도 고려하였으나, 2x2 화면 커서를 출력하기 위하서 자체적으로 커서를 출력함
void display_cursor(CURSOR cursor) {
	CHAR_INFO ch = getCharXY(cursor.pos.column, cursor.pos.row);
	// 커서의 색을 기존 글자색의 반전(배경색->전경색, 전경색->배경색)하여 사용함
	WORD fg = ch.Attributes & 0x0F, bg = ch.Attributes & 0xF0;
	WORD wAttr = setTextAttribute(fg << 4 | bg >> 4);

	// 커서를 그림. size에 따라 1x1 또는 2x2를 출력
	putCharXY(cursor.pos.column, cursor.pos.row, cursor.charInfo[0].Char.UnicodeChar);
	if (cursor.size == 2) {
		putCharXY(cursor.pos.column + 1, cursor.pos.row, cursor.charInfo[1].Char.UnicodeChar);
		putCharXY(cursor.pos.column, cursor.pos.row + 1, cursor.charInfo[2].Char.UnicodeChar);
		putCharXY(cursor.pos.column + 1, cursor.pos.row + 1, cursor.charInfo[3].Char.UnicodeChar);
	}
	setTextAttribute(wAttr);
}

// 커서 밑에 있는 화면 정보를 복원한다. 커서 이동할 때 사용
void restoreCursorChar(int col, int row, CHAR_INFO ch)
{
	WORD attr = setTextAttribute(ch.Attributes);
	putCharXY(col, row, ch.Char.UnicodeChar);
	setTextAttribute(attr);
}

// 커서 크기를 2x2로 키운다.
void increase_cursor_size(CURSOR* cursor)
{
	if (cursor->size == 2) return;
	if (cursor->pos.row == rectMap.Bottom || cursor->pos.column == rectMap.Right) {
		// 현재 커서가 map의 마지막 라인이나 오른쪽에 있는 경우, 크기를 증가시킬 경우 한칸 위또는 왼쪽으로 
		// 이동해야 하기 때문에 원래 커서 위치의 문자를 복원하고 위 또는 왼쪽으로 한칸씩 이동
		restoreCursorChar(cursor->pos.column, cursor->pos.row, cursor->charInfo[0]);
		if (cursor->pos.row == rectMap.Bottom)
			cursor->pos.row--;
		if (cursor->pos.column == rectMap.Right)
			cursor->pos.column--;
		cursor->charInfo[0] = getCharXY(cursor->pos.column, cursor->pos.row);
	}

	// 2x2에 해당하는 문자 정보를 기록
	cursor->charInfo[1] = getCharXY(cursor->pos.column + 1, cursor->pos.row);
	cursor->charInfo[2] = getCharXY(cursor->pos.column, cursor->pos.row + 1);
	cursor->charInfo[3] = getCharXY(cursor->pos.column + 1, cursor->pos.row + 1);

	cursor->size = 2;
}

// 커서 크기를 1x1로 줄인다.
void decrease_cursor_size(CURSOR* cursor)
{
	if (cursor->size == 1) return;
	// 2x2를 1x1으로 변경할 때, 축소되는 부분을 화면에 복원
	restoreCursorChar(cursor->pos.column + 1, cursor->pos.row, cursor->charInfo[1]);
	restoreCursorChar(cursor->pos.column, cursor->pos.row + 1, cursor->charInfo[2]);
	restoreCursorChar(cursor->pos.column + 1, cursor->pos.row + 1, cursor->charInfo[3]);

	cursor->size = 1;
}

// 커서를 이동하며, map 안으로 제한함. delay가 40ms 이하인 경우에는 3칸식 이동가능
void move_cursor(CURSOR* cursor, KEY key, int diff) {
	POSITION oldPos = cursor->pos;
	// 40ms 이하인 경우, 3칸씩 이동하고, 그렇지 않은 경우 1칸씩 이동
	int jump = diff <= 40 ? 3 : 1;

	switch (key) {
	case k_up:
		if (cursor->pos.row - jump >= rectMap.Top) {
			cursor->pos.row -= jump;
		}
		break;
	case k_down:
		// 커서 크기에 따라 이동하는 위치 제한
		if (cursor->pos.row + jump <= rectMap.Bottom - (cursor->size - 1)) {
			cursor->pos.row += jump;
		}
		break;
	case k_left:
		if (cursor->pos.column - jump >= rectMap.Left) {
			cursor->pos.column -= jump;
		}
		break;
	case k_right:
		// 커서 크기에 따라 이동하는 위치 제한
		if (cursor->pos.column + jump <= rectMap.Right - (cursor->size - 1)) {
			cursor->pos.column += jump;
		}
		break;
	}

	// 커서의 위치가 변동되면, 기존의 화면 정보는 복원하고, 새롭게 이동된 커서의 위치를 저장한다.
	if (oldPos.column != cursor->pos.column || oldPos.row != cursor->pos.row) {
		// 커서의 크기를 고려하여 1x1 또는 2x2의 문자를 복원한다.
		restoreCursorChar(oldPos.column, oldPos.row, cursor->charInfo[0]);
		if (cursor->size == 2) {
			restoreCursorChar(oldPos.column + 1, oldPos.row, cursor->charInfo[1]);
			restoreCursorChar(oldPos.column, oldPos.row + 1, cursor->charInfo[2]);
			restoreCursorChar(oldPos.column + 1, oldPos.row + 1, cursor->charInfo[3]);
		}

		// 커서의 크기를 고려하여 화면 정보를 저장한다.
		cursor->charInfo[0] = getCharXY(cursor->pos.column, cursor->pos.row);
		if (cursor->size == 2) {
			cursor->charInfo[1] = getCharXY(cursor->pos.column + 1, cursor->pos.row);
			cursor->charInfo[2] = getCharXY(cursor->pos.column, cursor->pos.row + 1);
			cursor->charInfo[3] = getCharXY(cursor->pos.column + 1, cursor->pos.row + 1);
		}
	}
}
