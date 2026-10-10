#pragma once

#include <Windows.h>

class Utils {
public:
    static void applyDarkTitleBar(HWND hwnd);
    static void applyMenuTheme();
private:
    static bool isWindowsDarkMode();
};
