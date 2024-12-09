#include <stdio.h>
#include "dune.h"

KEY get_key(void)
{
	if (!_kbhit()) { // 입력된 키가 있는지 확인
		return k_none;
	}

	int byte = _getch(); // 입력된 키를 전달 받기
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
		byte = _getch(); // MSB 224 가 입력 되면 1 바이트 더 받음
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
