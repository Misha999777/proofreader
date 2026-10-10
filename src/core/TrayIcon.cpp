#include "TrayIcon.hpp"

#include <cwctype>
#include <algorithm>

#include "utils/Utils.hpp"
#include "resources/resource.hpp"

#define WM_TRAYICON (WM_USER + 1)
#define WM_SINGLE_INSTANCE (WM_USER + 2)
#define HOTKEY_ID 1

TrayIcon::TrayIcon(Callbacks callbacks) : m_callbacks(std::move(callbacks)) {
    Utils::applyMenuTheme();

    HINSTANCE hInstance = GetModuleHandleW(NULL);
    WNDCLASSW wc = {};
    wc.lpfnWndProc = TrayIcon::windowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"ProofReaderTrayWindow";
    RegisterClassW(&wc);

    m_hwnd = CreateWindowW(L"ProofReaderTrayWindow", L"ProofReaderTrayWindow", 0, 0, 0, 0, 0,
        NULL, NULL, hInstance, this);

    SetWindowLongPtr(m_hwnd, GWLP_USERDATA, (LONG_PTR)this);

    m_nid = {};
    m_nid.cbSize = sizeof(NOTIFYICONDATAW);
    m_nid.hWnd = m_hwnd;
    m_nid.uID = 1;
    m_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    m_nid.uCallbackMessage = WM_TRAYICON;
    m_nid.hIcon = LoadIconW(hInstance, MAKEINTRESOURCEW(IDI_APP_ICON));
    wcscpy_s(m_nid.szTip, L"ProofReader");
    Shell_NotifyIconW(NIM_ADD, &m_nid);

    RegisterHotKey(m_hwnd, HOTKEY_ID, MOD_CONTROL | MOD_ALT | MOD_NOREPEAT, 'P');
}

TrayIcon::~TrayIcon() {
    UnregisterHotKey(m_hwnd, HOTKEY_ID);
    Shell_NotifyIconW(NIM_DELETE, &m_nid);
    DestroyWindow(m_hwnd);
}

LRESULT CALLBACK TrayIcon::windowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    TrayIcon* pThis = (TrayIcon*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

    switch (msg) {
        case WM_TRAYICON: {
            if (LOWORD(lParam) == WM_LBUTTONUP) {
                if (pThis && pThis->m_callbacks.onToggleWindow) {
                    pThis->m_callbacks.onToggleWindow();
                }
            } else if (LOWORD(lParam) == WM_RBUTTONUP) {
                POINT pt;
                GetCursorPos(&pt);
                HMENU hMenu = CreatePopupMenu();
                InsertMenuW(hMenu, 0, MF_BYPOSITION | MF_STRING, 1, L"Show");
                InsertMenuW(hMenu, 1, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
                InsertMenuW(hMenu, 2, MF_BYPOSITION | MF_STRING, 2, L"Exit");
                SetForegroundWindow(hwnd);
                int cmd = TrackPopupMenu(hMenu, TPM_RETURNCMD | TPM_NONOTIFY, pt.x, pt.y, 0, hwnd, NULL);
                DestroyMenu(hMenu);
                
                if (cmd == 1) {
                    if (pThis && pThis->m_callbacks.onToggleWindow) {
                        pThis->m_callbacks.onToggleWindow();
                    }
                } else if (cmd == 2) {
                    if (pThis && pThis->m_callbacks.onQuit) {
                        pThis->m_callbacks.onQuit();
                    }
                }
            }
            break;
        }
        case WM_HOTKEY: {
            if (pThis && pThis->m_callbacks.onTextSelected && wParam == HOTKEY_ID) {
                std::wstring text = pThis->getSelectedTextViaUIA();
                pThis->m_callbacks.onTextSelected(text);
            }
            break;
        }
        case WM_SINGLE_INSTANCE: {
            if (pThis && pThis->m_callbacks.onToggleWindow) {
                pThis->m_callbacks.onToggleWindow();
            }
            break;
        }
        case WM_SETTINGCHANGE: {
            if (lParam != 0) {
                const wchar_t* setting = reinterpret_cast<const wchar_t*>(lParam);
                if (wcscmp(setting, L"ImmersiveColorSet") == 0) {
                    Utils::applyMenuTheme();
                }
            }
            break;
        }
        case WM_CLOSE: {
            if (pThis && pThis->m_callbacks.onQuit) {
                pThis->m_callbacks.onQuit(); 
            } else {
                DestroyWindow(hwnd);
            }
            return 0;
        }
        case WM_DESTROY: {
            PostQuitMessage(0); 
            return 0;
        }
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

std::wstring TrayIcon::getSelectedTextViaUIA() {
    IUIAutomation* automation = nullptr;
    HRESULT hr = CoCreateInstance(__uuidof(CUIAutomation), NULL, CLSCTX_INPROC_SERVER,
        __uuidof(IUIAutomation), (void**)&automation);
    if (FAILED(hr) || !automation) {
        return L"";
    }

    constexpr int maxAttempts = 3;
    constexpr DWORD retryDelayMs = 100;
    std::wstring result = tryGetSelectedText(automation);
    for (int attempt = 0; attempt < maxAttempts && result.empty(); attempt++) {
        Sleep(retryDelayMs);
        result = tryGetSelectedText(automation);
    }

    automation->Release();
    return result;
}

std::wstring TrayIcon::tryGetSelectedText(IUIAutomation* automation) {
    IUIAutomationElement* focused = nullptr;
    HRESULT hr = automation->GetFocusedElement(&focused);
    if (FAILED(hr) || !focused) {
        return L"";
    }

    std::wstring result = L"";
    
    IUIAutomationTextPattern* textPattern = nullptr;
    hr = focused->GetCurrentPatternAs(UIA_TextPatternId, __uuidof(IUIAutomationTextPattern), (void**)&textPattern);
    if (SUCCEEDED(hr) && textPattern) {
        IUIAutomationTextRangeArray* selection = nullptr;
        hr = textPattern->GetSelection(&selection);
        if (SUCCEEDED(hr) && selection) {
            int length = 0;
            selection->get_Length(&length);
            if (length > 0) {
                IUIAutomationTextRange* range = nullptr;
                hr = selection->GetElement(0, &range);
                if (SUCCEEDED(hr) && range) {
                    BSTR text = nullptr;
                    hr = range->GetText(-1, &text);
                    if (SUCCEEDED(hr) && text) {
                        result = text;
                        SysFreeString(text);
                    }
                    range->Release();
                }
            }
            selection->Release();
        }
        textPattern->Release();
    }
    focused->Release();

    if (std::ranges::all_of(result, [](wchar_t c) { return std::iswspace(c); })) {
        return L"";
    }
    return result;
}
