#include "console.h"
#include <iostream>

#if defined(_WIN32)
HANDLE Console::hStdout;
CONSOLE_SCREEN_BUFFER_INFO Console::csbiInfo;
CONSOLE_CURSOR_INFO	Console::cursorInfo;
#endif

bool Console::initConsole() {
#if defined(_WIN32)
	hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hStdout == INVALID_HANDLE_VALUE) {
		std::cout << "Error: GetStdHandle\n";
		return false;
	}
	if (!GetConsoleScreenBufferInfo(hStdout, &csbiInfo))
	{
		std::cout << "Error: GetConsoleScreenBufferInfo\n";
		return false;
	}

	GetConsoleCursorInfo(hStdout, &cursorInfo);
	cursorInfo.bVisible = false;
	SetConsoleCursorInfo(hStdout, &cursorInfo);
#endif
	return true;
}

void Console::gotoXY(int x, int y) {
#if defined(_WIN32)
	csbiInfo.dwCursorPosition.X = x;
	csbiInfo.dwCursorPosition.Y = y;
	SetConsoleCursorPosition(hStdout, csbiInfo.dwCursorPosition);
#elif defined(__linux__)
	printf("%c[%d;%df", 0x1B, y + 1, x);
#endif
}
