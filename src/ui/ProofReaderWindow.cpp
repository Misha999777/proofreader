#include "ui/ProofReaderWindow.hpp"

#include <dwmapi.h>
#include <saucer/embedded/all.hpp>

#include "utils/Utils.hpp"

#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif

static void applyDarkTitleBar(HWND hwnd) {
    BOOL useDark = Utils::isWindowsDarkMode() ? TRUE : FALSE;
    DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &useDark, sizeof(useDark));
}

static LRESULT CALLBACK themeSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR, DWORD_PTR) {
    if (msg == WM_SETTINGCHANGE && lParam != 0) {
        const wchar_t* setting = reinterpret_cast<const wchar_t*>(lParam);
        if (wcscmp(setting, L"ImmersiveColorSet") == 0) {
            applyDarkTitleBar(hwnd);
        }
    }
    return DefSubclassProc(hwnd, msg, wParam, lParam);
}

std::string ProofReaderWindow::utf16_to_utf8(const std::wstring& wstr) {
    if (wstr.empty()) return {};
    int size = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), (int)wstr.size(), nullptr, 0, nullptr, nullptr);
    std::string result(size, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.data(), (int)wstr.size(), result.data(), size, nullptr, nullptr);
    return result;
}

ProofReaderWindow::ProofReaderWindow(saucer::application* app, bool devMode) {
    auto window_result = saucer::window::create(app);
    m_window = std::move(window_result.value());

    auto webview_result = saucer::smartview::create({
        .window = m_window,
        .hardware_acceleration = false,
        .browser_flags = {"--js-flags=--max-old-space-size=48"},
    });
    m_webview.emplace(std::move(webview_result.value()));

    m_window->set_title("ProofReader");
    m_window->set_size({420, 620});
    m_window->set_resizable(false);

    // Find our window and apply the embedded icon via Win32
    HWND hwnd = FindWindowW(NULL, L"ProofReader");
    if (hwnd) {
        HINSTANCE hInstance = GetModuleHandle(NULL);
        HICON hIcon = LoadIconW(hInstance, MAKEINTRESOURCEW(101));
        if (hIcon) {
            SendMessageW(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
            SendMessageW(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
        }

        applyDarkTitleBar(hwnd);
        SetWindowSubclass(hwnd, themeSubclassProc, 1, 0);
    }

    m_webview->set_context_menu(false);
    m_webview->set_dev_tools(devMode);

    m_webview->expose("resize", [this](int w, int h) {
        m_window->set_size({w, h});
    });

    m_window->on<saucer::window::event::close>([this]() {
        m_window->hide();
        return saucer::policy::block;
    });

    if (devMode) {
        m_webview->set_url("http://localhost:5173");
    } else {
        m_webview->embed(saucer::embedded::all());
        m_webview->serve("index.html");
    }
}

ProofReaderWindow::~ProofReaderWindow() {
    HWND hwnd = FindWindowW(NULL, L"ProofReader");
    if (hwnd) {
        RemoveWindowSubclass(hwnd, themeSubclassProc, 1);
    }
}

void ProofReaderWindow::show() {
    m_window->show();
}

void ProofReaderWindow::hide() {
    m_window->hide();
}

void ProofReaderWindow::focus() {
    m_window->focus();
}

void ProofReaderWindow::sendText(const std::wstring& text) {
    m_webview->execute("if(window.setOriginalText) window.setOriginalText({});", utf16_to_utf8(text));
}
