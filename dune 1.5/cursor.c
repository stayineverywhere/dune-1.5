#include "dune.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// �ʱ� Ŀ�� ��ǥ�� ����
// Ŀ�� ��ǥ�� ȭ�� ��ǥ�� �̿��ϰ�, �̵� ������ rectMap ������ ����
void init_cursor(CURSOR* cursor) {
	srand((unsigned)time(NULL));
	cursor->pos.row = rand() % MAP_HEIGHT + rectMap.Top;
	cursor->pos.column = rand() % MAP_WIDTH + rectMap.Left;
	cursor->charInfo[0] = getCharXY(cursor->pos.column, cursor->pos.row);
	cursor->size = 1;

	add_system_fmessage("�ʱ� Ŀ�� ��ǥ: %d�� %d��",
		cursor->pos.row - rectMap.Top,
		cursor->pos.column - rectMap.Left);
}

// ȭ�鿡 Ŀ���� ����Ѵ�.
// Ŀ���� Ȯ��Ǵ� ��쿡�� ������ ������� Ȯ���Ͽ� ���
// console�� Ŀ���� ����ϴ� ����� ����Ͽ�����, 2x2 ȭ�� Ŀ���� ����ϱ� ���ϼ� ��ü������ Ŀ���� �����
void display_cursor(CURSOR cursor) {
	CHAR_INFO ch = getCharXY(cursor.pos.column, cursor.pos.row);
	// Ŀ���� ���� ���� ���ڻ��� ����(����->�����, �����->����)�Ͽ� �����
	WORD fg = ch.Attributes & 0x0F, bg = ch.Attributes & 0xF0;
	WORD wAttr = setTextAttribute(fg << 4 | bg >> 4);

	// Ŀ���� �׸�. size�� ���� 1x1 �Ǵ� 2x2�� ���
	putCharXY(cursor.pos.column, cursor.pos.row, cursor.charInfo[0].Char.UnicodeChar);
	if (cursor.size == 2) {
		putCharXY(cursor.pos.column + 1, cursor.pos.row, cursor.charInfo[1].Char.UnicodeChar);
		putCharXY(cursor.pos.column, cursor.pos.row + 1, cursor.charInfo[2].Char.UnicodeChar);
		putCharXY(cursor.pos.column + 1, cursor.pos.row + 1, cursor.charInfo[3].Char.UnicodeChar);
	}
	setTextAttribute(wAttr);
}

// Ŀ�� �ؿ� �ִ� ȭ�� ������ �����Ѵ�. Ŀ�� �̵��� �� ���
void restoreCursorChar(int col, int row, CHAR_INFO ch)
{
	WORD attr = setTextAttribute(ch.Attributes);
	putCharXY(col, row, ch.Char.UnicodeChar);
	setTextAttribute(attr);
}

// Ŀ�� ũ�⸦ 2x2�� Ű���.
void increase_cursor_size(CURSOR* cursor)
{
	if (cursor->size == 2) return;
	if (cursor->pos.row == rectMap.Bottom || cursor->pos.column == rectMap.Right) {
		// ���� Ŀ���� map�� ������ �����̳� �����ʿ� �ִ� ���, ũ�⸦ ������ų ��� ��ĭ ���Ǵ� �������� 
		// �̵��ؾ� �ϱ� ������ ���� Ŀ�� ��ġ�� ���ڸ� �����ϰ� �� �Ǵ� �������� ��ĭ�� �̵�
		restoreCursorChar(cursor->pos.column, cursor->pos.row, cursor->charInfo[0]);
		if (cursor->pos.row == rectMap.Bottom)
			cursor->pos.row--;
		if (cursor->pos.column == rectMap.Right)
			cursor->pos.column--;
		cursor->charInfo[0] = getCharXY(cursor->pos.column, cursor->pos.row);
	}

	// 2x2�� �ش��ϴ� ���� ������ ���
	cursor->charInfo[1] = getCharXY(cursor->pos.column + 1, cursor->pos.row);
	cursor->charInfo[2] = getCharXY(cursor->pos.column, cursor->pos.row + 1);
	cursor->charInfo[3] = getCharXY(cursor->pos.column + 1, cursor->pos.row + 1);

	cursor->size = 2;
}

// Ŀ�� ũ�⸦ 1x1�� ���δ�.
void decrease_cursor_size(CURSOR* cursor)
{
	if (cursor->size == 1) return;
	// 2x2�� 1x1���� ������ ��, ��ҵǴ� �κ��� ȭ�鿡 ����
	restoreCursorChar(cursor->pos.column + 1, cursor->pos.row, cursor->charInfo[1]);
	restoreCursorChar(cursor->pos.column, cursor->pos.row + 1, cursor->charInfo[2]);
	restoreCursorChar(cursor->pos.column + 1, cursor->pos.row + 1, cursor->charInfo[3]);

	cursor->size = 1;
}

// Ŀ���� �̵��ϸ�, map ������ ������. delay�� 40ms ������ ��쿡�� 3ĭ�� �̵�����
void move_cursor(CURSOR* cursor, KEY key, int diff) {
	POSITION oldPos = cursor->pos;
	// 40ms ������ ���, 3ĭ�� �̵��ϰ�, �׷��� ���� ��� 1ĭ�� �̵�
	int jump = diff <= 40 ? 3 : 1;

	switch (key) {
	case k_up:
		if (cursor->pos.row - jump >= rectMap.Top) {
			cursor->pos.row -= jump;
		}
		break;
	case k_down:
		// Ŀ�� ũ�⿡ ���� �̵��ϴ� ��ġ ����
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
		// Ŀ�� ũ�⿡ ���� �̵��ϴ� ��ġ ����
		if (cursor->pos.column + jump <= rectMap.Right - (cursor->size - 1)) {
			cursor->pos.column += jump;
		}
		break;
	}

	// Ŀ���� ��ġ�� �����Ǹ�, ������ ȭ�� ������ �����ϰ�, ���Ӱ� �̵��� Ŀ���� ��ġ�� �����Ѵ�.
	if (oldPos.column != cursor->pos.column || oldPos.row != cursor->pos.row) {
		// Ŀ���� ũ�⸦ ����Ͽ� 1x1 �Ǵ� 2x2�� ���ڸ� �����Ѵ�.
		restoreCursorChar(oldPos.column, oldPos.row, cursor->charInfo[0]);
		if (cursor->size == 2) {
			restoreCursorChar(oldPos.column + 1, oldPos.row, cursor->charInfo[1]);
			restoreCursorChar(oldPos.column, oldPos.row + 1, cursor->charInfo[2]);
			restoreCursorChar(oldPos.column + 1, oldPos.row + 1, cursor->charInfo[3]);
		}

		// Ŀ���� ũ�⸦ ����Ͽ� ȭ�� ������ �����Ѵ�.
		cursor->charInfo[0] = getCharXY(cursor->pos.column, cursor->pos.row);
		if (cursor->size == 2) {
			cursor->charInfo[1] = getCharXY(cursor->pos.column + 1, cursor->pos.row);
			cursor->charInfo[2] = getCharXY(cursor->pos.column, cursor->pos.row + 1);
			cursor->charInfo[3] = getCharXY(cursor->pos.column + 1, cursor->pos.row + 1);
		}
	}
}
