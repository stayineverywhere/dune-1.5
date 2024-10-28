#pragma once

#include <Windows.h>
#include <conio.h>

typedef enum {
	k_none = 0, k_escape, k_space, k_A, k_B, k_C, k_D, k_E, k_F, k_G,
	k_H, k_I, k_J, k_L, k_M, k_N,
	k_O, k_P, k_Q, k_R, k_S, k_T, k_U, k_V, k_W, k_X, k_Y, k_Z,
	k_quit,
	k_up, k_right, k_down, k_left,
	k_undef
} KEY;

typedef enum {
	d_stay = 0,
	d_up, d_right, d_down, d_left
} DIRECTION;