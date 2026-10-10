#include "utils/Utils.hpp"

#include <dwmapi.h>

typedef int (WINAPI *fnSetPreferredAppMode)(int mode);
typedef void (WINAPI *fnFlushMenuThemes)();

#define SET_PREFERRED_APP_MODE_ORDINAL 135
#define FLUSH_MENU_THEMES_ORDINAL 136
#define APP_MODE_DEFAULT 0
#define APP_MODE_ALLOW_DARK 2

bool Utils::isWindowsDarkMode() {
    DWORD value = 1;
    DWORD size = sizeof(value);
    RegGetValueW(HKEY_CURRENT_USER,
        L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
        L"AppsUseLightTheme", RRF_RT_REG_DWORD, nullptr, &value, &size);
    return value == 0;
}

void Utils::applyMenuTheme() {
    HMODULE hUxtheme = LoadLibraryExW(L"uxtheme.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (hUxtheme) {
        auto pSetPreferredAppMode
            = (fnSetPreferredAppMode)GetProcAddress(hUxtheme, MAKEINTRESOURCEA(SET_PREFERRED_APP_MODE_ORDINAL));
        auto pFlushMenuThemes
            = (fnFlushMenuThemes)GetProcAddress(hUxtheme, MAKEINTRESOURCEA(FLUSH_MENU_THEMES_ORDINAL));

        if (pSetPreferredAppMode) {
            pSetPreferredAppMode(isWindowsDarkMode() ? APP_MODE_ALLOW_DARK : APP_MODE_DEFAULT);
        }
        if (pFlushMenuThemes) {
            pFlushMenuThemes();
        }
    }
}

void Utils::applyDarkTitleBar(HWND hwnd) {
    BOOL useDark = Utils::isWindowsDarkMode() ? TRUE : FALSE;
    DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &useDark, sizeof(useDark));
}
