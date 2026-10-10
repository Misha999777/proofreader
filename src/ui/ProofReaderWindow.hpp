#pragma once

#include <optional>
#include <Windows.h>
#include <saucer/smartview.hpp>

class ProofReaderWindow {
public:
    ProofReaderWindow(saucer::application* app, bool devMode = false);
    ~ProofReaderWindow();

    void show();
    void hide();
    void focus();
    void sendText(const std::wstring& text);

private:
    static std::string utf16_to_utf8(const std::wstring& wstr);
    static LRESULT CALLBACK themeSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR, DWORD_PTR);

    void setLogicalSize(saucer::size size);

    std::shared_ptr<saucer::window> m_window;
    std::optional<saucer::smartview> m_webview;

    saucer::size m_logicalSize{420, 620};
};
