#include <stdio.h>
#include "dune.h"

KEY get_key(void)
{
	if (!_kbhit()) { // �Էµ� Ű�� �ִ��� Ȯ��
		return k_none;
	}

	int byte = _getch(); // �Էµ� Ű�� ���� �ޱ�
	switch (byte) {
	case 'f': case 'F': return k_F;
	case 'h': case 'H': return k_H;
	case 'm': case 'M': return k_M;
	case 'p': case 'P': return k_P;
	case 's': case 'S': return k_S;
	case 't': case 'T': return k_T;
	case 'x': case 'X': return k_X;
	case 'q': case 'Q': return k_quit; // ' �� ������ ����
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
