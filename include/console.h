#pragma once

#if defined(_WIN32)
#include <Windows.h>
#elif defined(__linux__) 
#include<stdio.h>
#include<stdlib.h>
#endif

class Console {
#if defined(_WIN32)
	static HANDLE hStdout;
	static CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
	static CONSOLE_CURSOR_INFO cursorInfo;
#endif

public:
	static bool initConsole();
	static void gotoXY(int x, int y);
};

