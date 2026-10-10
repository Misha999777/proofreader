#pragma once

#include <string>
#include <functional>
#include <Windows.h>
#include <UIAutomation.h>

class TrayIcon {
public:
    struct Callbacks {
        std::function<void()> onToggleWindow;
        std::function<void(const std::wstring&)> onTextSelected;
        std::function<void()> onQuit;
    };

    TrayIcon(Callbacks callbacks);
    ~TrayIcon();

private:
    static LRESULT CALLBACK windowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    std::wstring getSelectedTextViaUIA();
    std::wstring tryGetSelectedText(IUIAutomation* automation);

    Callbacks m_callbacks;
    HWND m_hwnd;
    NOTIFYICONDATAW m_nid;
};
