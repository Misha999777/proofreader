#include "ProofReaderWindow.hpp"

#include <dwmapi.h>
#include <saucer/embedded/all.hpp>
#include <saucer/modules/stable/webview2.hpp>

#include "utils/Settings.hpp"
#include "utils/Utils.hpp"

LRESULT CALLBACK ProofReaderWindow::themeSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR, DWORD_PTR) {
    if (msg == WM_SETTINGCHANGE && lParam != 0) {
        const wchar_t* setting = reinterpret_cast<const wchar_t*>(lParam);
        if (wcscmp(setting, L"ImmersiveColorSet") == 0) {
            Utils::applyDarkTitleBar(hwnd);
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

ProofReaderWindow::ProofReaderWindow(saucer::application* app) {
    auto window_result = saucer::window::create(app);
    m_window = std::move(window_result.value());

    std::set<std::string> browserFlags;
#ifdef NDEBUG
    browserFlags.emplace("--js-flags=--max-old-space-size=48");
#endif
    auto webview_result = saucer::smartview::create({
        .window = m_window,
        .hardware_acceleration = false,
        .browser_flags = std::move(browserFlags),
    });
    m_webview.emplace(std::move(webview_result.value()));
    m_webview->set_context_menu(false);

    m_window->set_title("ProofReader");
    setLogicalSize(m_logicalSize);
    m_window->set_resizable(false);

    HWND hwnd = m_window->native().hwnd;
    if (hwnd) {
        HINSTANCE hInstance = GetModuleHandle(NULL);
        HICON hIcon = LoadIconW(hInstance, MAKEINTRESOURCEW(101));
        if (hIcon) {
            SendMessageW(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
            SendMessageW(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
        }

        Utils::applyDarkTitleBar(hwnd);
        SetWindowSubclass(hwnd, themeSubclassProc, 1, 0);

        if (Utils::applyMicaBackdrop(hwnd)) {
            m_window->set_background({0, 0, 0, 0});
            m_webview->set_background({0, 0, 0, 0});
        }
    }

    m_webview->expose("resize", [this](int w, int h) {
        setLogicalSize({w, h});
    });

    m_webview->expose("hide", [this]() {
        m_window->hide();
    });

    m_webview->expose("getSettings", []() {
        return SettingsStore::load();
    });

    m_webview->expose("setSettings", [](Settings settings) {
        return SettingsStore::save(settings);
    });

    m_webview->expose("post", [this](std::string url, std::string apiKey, std::string body, saucer::executor<std::string> executor) {
        m_http.post(url, apiKey, body, [executor = std::move(executor)](auto result) {
            if (result) {
                executor.resolve(*result);
            } else {
                executor.reject(result.error());
            }
        });
    });

    m_webview->expose("cancel", [this]() {
        m_http.cancel();
    });

    m_window->on<saucer::window::event::close>([this]() {
        m_window->hide();
        return saucer::policy::block;
    });

#ifdef NDEBUG
    m_webview->embed(saucer::embedded::all());
    m_webview->serve("index.html");
#else
    m_webview->set_url("http://localhost:5173");
     m_webview->set_dev_tools(true);
#endif
}

ProofReaderWindow::~ProofReaderWindow() {
    HWND hwnd = m_window ? m_window->native().hwnd : nullptr;
    if (hwnd) {
        RemoveWindowSubclass(hwnd, themeSubclassProc, 1);
    }
}

void ProofReaderWindow::setLogicalSize(saucer::size size) {
    m_logicalSize = size;
    m_window->set_size(size);
}

void ProofReaderWindow::show() {
    HWND hwnd = m_window ? m_window->native().hwnd : nullptr;
    if (hwnd) {
        UINT currentDpi = GetDpiForWindow(hwnd);
        RECT rect;
        if (GetWindowRect(hwnd, &rect)) {
            SendMessageW(hwnd, WM_DPICHANGED, MAKEWPARAM(currentDpi, currentDpi), (LPARAM)&rect);
        }
    }

    m_window->set_size(m_logicalSize);
    m_window->show();
}

void ProofReaderWindow::hide() {
    m_window->hide();
}

void ProofReaderWindow::focus() {
    m_window->focus();
    m_webview->native().controller->MoveFocus(COREWEBVIEW2_MOVE_FOCUS_REASON_PROGRAMMATIC);
}

void ProofReaderWindow::sendText(const std::wstring& text) {
    m_webview->execute("if(window.setOriginalText) window.setOriginalText({});", utf16_to_utf8(text));
}
