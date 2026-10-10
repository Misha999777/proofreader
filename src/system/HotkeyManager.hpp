#pragma once

#include <string>
#include <Windows.h>
#include <UIAutomation.h>

class HotkeyManager {
public:
    HotkeyManager(HWND hwnd, int hotkeyId);
    ~HotkeyManager();

    std::wstring getSelectedTextViaUIA();

private:
    std::wstring tryGetSelectedText(IUIAutomation* automation);

    HWND m_hwnd;
    int m_hotkeyId;
};
