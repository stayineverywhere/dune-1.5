#include <stdio.h>
#include "dune.h"

KEY get_key(void)
{
	if (!_kbhit()) { // �Էµ� Ű�� �ִ��� Ȯ��
		return k_none;
	}

	int byte = _getch(); // �Էµ� Ű�� ���� �ޱ�
	if (byte >= 'a' && byte <= 'z') {
		return k_A + byte - 'a';
	}
	else if (byte >= 'A' && byte <= 'Z') {
		return k_A + byte - 'A';
	}
	switch (byte) {
	case ' ': return k_space;
	case 27: return k_escape;
	case 224:
		byte = _getch(); // MSB 224 �� �Է� �Ǹ� 1 ����Ʈ �� ����
		switch (byte) {
		case 72: return k_up;
		case 75: return k_left;
		case 77: return k_right;
		case 80: return k_down;
		default: return k_undef;
		}
	default: return k_undef;
	}
}
