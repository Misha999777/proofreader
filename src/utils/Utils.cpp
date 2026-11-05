#include "utils/Utils.hpp"

#include <dwmapi.h>

bool Utils::isWindowsDarkMode() {
    DWORD value = 1;
    DWORD size = sizeof(value);
    RegGetValueW(HKEY_CURRENT_USER,
        L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
        L"AppsUseLightTheme", RRF_RT_REG_DWORD, nullptr, &value, &size);
    return value == 0;
}

void Utils::applyDarkTitleBar(HWND hwnd) {
    BOOL useDark = Utils::isWindowsDarkMode() ? TRUE : FALSE;
    DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &useDark, sizeof(useDark));
}

bool Utils::applyMicaBackdrop(HWND hwnd) {
    DWM_SYSTEMBACKDROP_TYPE backdrop = DWMSBT_MAINWINDOW;
    if (FAILED(DwmSetWindowAttribute(hwnd, DWMWA_SYSTEMBACKDROP_TYPE, &backdrop, sizeof(backdrop)))) {
        return false;
    }
    MARGINS margins = {-1, -1, -1, -1};
    DwmExtendFrameIntoClientArea(hwnd, &margins);
    return true;
}
