#include "core/ProofReaderApp.hpp"

ProofReaderApp::ProofReaderApp() : m_hMutex(NULL) {}

ProofReaderApp::~ProofReaderApp() {
    if (m_hMutex) {
        CloseHandle(m_hMutex);
    }
}

bool ProofReaderApp::enforceSingleInstance() {
    m_hMutex = CreateMutexW(NULL, TRUE, L"ProofReaderSingleInstanceMutex");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        HWND existing = FindWindowW(L"ProofReaderTrayWindow", L"ProofReaderTrayWindow");
        if (existing) {
            PostMessageW(existing, WM_USER + 2, 0, 0);
        }
        return false;
    }
    return true;
}

int ProofReaderApp::run(const std::wstring& cmdLine) {
    if (!enforceSingleInstance()) {
        return 0;
    }

    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

    bool hiddenLaunch = cmdLine.find(L"--autostart") != std::wstring::npos;
    bool devMode = (cmdLine.find(L"--dev") != std::wstring::npos);
    auto app_result = saucer::application::create({.id = "ProofReader", .quit_on_last_window_closed = false});
    if (!app_result) {
        return 1;
    }
    auto appInstance = std::move(app_result.value());

    auto start_coro = [this, hiddenLaunch, devMode](saucer::application *app) -> coco::stray {
        m_app = app;

        m_trayIcon = std::make_unique<TrayIcon>(this);
        m_hotkeyManager = std::make_unique<HotkeyManager>(m_trayIcon->getHwnd(), 1);
        m_proofReaderWindow = std::make_unique<ProofReaderWindow>(app, devMode);

        if (!hiddenLaunch) {
            m_proofReaderWindow->show();
        }
        co_await app->finish();
        m_proofReaderWindow.reset();
        m_hotkeyManager.reset();
        m_trayIcon.reset();
        m_app = nullptr;
    };

    return appInstance.run(start_coro);
}

void ProofReaderApp::toggleWindow() {
    if (!m_app) return;
    m_app->post([this]() {
        if (m_proofReaderWindow) {
            m_proofReaderWindow->show();
            m_proofReaderWindow->focus();
        }
    });
}

void ProofReaderApp::showWindowWithText(const std::wstring& text) {
    if (!m_app) return;
    m_app->post([this, text]() {
        if (m_proofReaderWindow) {
            if (!text.empty()) {
                m_proofReaderWindow->sendText(text);
            }
            m_proofReaderWindow->show();
            m_proofReaderWindow->focus();
        }
    });
}

void ProofReaderApp::handleHotkey(int hotkeyId) {
    if (hotkeyId == 1 && m_hotkeyManager) {
        std::wstring text = m_hotkeyManager->getSelectedTextViaUIA();
        if (!text.empty()) {
            showWindowWithText(text);
        } else {
            MessageBeep(MB_ICONWARNING);
        }
    }
}

void ProofReaderApp::quit() {
    if (!m_app) return;
    m_app->post([this]() {
        m_app->quit();
    });
}
